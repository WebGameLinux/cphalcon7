
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2014 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  +------------------------------------------------------------------------+
*/

#include "kernel/memory.h"

#include <Zend/zend_alloc.h>

#include "kernel/fcall.h"
#include "kernel/backtrace.h"
#include "kernel/framework/orm.h"

/*
 * Memory Frames/Virtual Symbol Scopes
 *------------------------------------
 *
 * Phalcon uses memory frames to track the variables used within a method
 * in order to free them or reduce their reference counting accordingly before
 * exit the method in execution.
 *
 * This adds a minimum overhead to execution but save us the work of
 * free memory in each method.
 *
 * The whole memory frame is an open double-linked list which start is an
 * allocated empty frame that points to the real first frame. The start
 * memory frame is globally accesed using PHALCON_GLOBAL(start_frame)
 *
 * Not all methods must grow/restore the phalcon_memory_entry.
 */

void phalcon_initialize_memory(zend_phalcon_globals *phalcon_globals_ptr)
{
	phalcon_memory_entry *start;
	size_t i;

	start = (phalcon_memory_entry *) pecalloc(PHALCON_NUM_PREALLOCATED_FRAMES, sizeof(phalcon_memory_entry), 1);

	for (i = 0; i < PHALCON_NUM_PREALLOCATED_FRAMES; ++i) {
		start[i].addresses       = pecalloc(24, sizeof(zval*), 1);
		start[i].capacity        = 24;

#ifndef PHALCON_RELEASE
		start[i].permanent = 1;
#endif
	}

	start[0].next = &start[1];
	start[PHALCON_NUM_PREALLOCATED_FRAMES - 1].prev = &start[PHALCON_NUM_PREALLOCATED_FRAMES - 2];

	for (i = 1; i < PHALCON_NUM_PREALLOCATED_FRAMES - 1; ++i) {
		start[i].next = &start[i + 1];
		start[i].prev = &start[i - 1];
	}

	phalcon_globals_ptr->start_memory = start;
	phalcon_globals_ptr->end_memory   = start + PHALCON_NUM_PREALLOCATED_FRAMES;

	ZVAL_NULL(&phalcon_globals_ptr->z_null);
	ZVAL_FALSE(&phalcon_globals_ptr->z_false);
	ZVAL_TRUE(&phalcon_globals_ptr->z_true);
	ZVAL_LONG(&phalcon_globals_ptr->z_zero, 0);
	ZVAL_LONG(&phalcon_globals_ptr->z_one, 1);
	ZVAL_LONG(&phalcon_globals_ptr->z_two, 2);

	phalcon_globals_ptr->initialized = 1;
}

void phalcon_deinitialize_memory()
{
	size_t i;
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	if (phalcon_globals_ptr->initialized != 1) {
		phalcon_globals_ptr->initialized = 0;
		return;
	}

	if (phalcon_globals_ptr->start_memory != NULL) {
		phalcon_clean_restore_stack();
	}

	phalcon_orm_destroy_cache();

#ifndef PHALCON_RELEASE
	assert(phalcon_globals_ptr->start_memory != NULL);
#endif

	for (i = 0; i < PHALCON_NUM_PREALLOCATED_FRAMES; ++i) {
		pefree(phalcon_globals_ptr->start_memory[i].addresses, 1);
	}

	pefree(phalcon_globals_ptr->start_memory, 1);
	phalcon_globals_ptr->start_memory = NULL;

	phalcon_globals_ptr->initialized = 0;
}

static phalcon_memory_entry* phalcon_memory_grow_stack_common(zend_phalcon_globals *g)
{
	assert(g->start_memory != NULL);
	if (!g->active_memory) {
		g->active_memory = g->start_memory;
#ifndef PHALCON_RELEASE
		assert(g->active_memory->permanent == 1);
#endif
	}
	else if (!g->active_memory->next) {
		phalcon_memory_entry *entry;

		assert(g->active_memory >= g->end_memory - 1 || g->active_memory < g->start_memory);
		entry = (phalcon_memory_entry *) ecalloc(1, sizeof(phalcon_memory_entry));

#ifndef PHALCON_RELEASE
		entry->permanent  = 0;
		entry->func       = NULL;
#endif
		entry->prev       = g->active_memory;
		entry->prev->next = entry;
		g->active_memory  = entry;
	}
	else {
#ifndef PHALCON_RELEASE
		assert(g->active_memory->permanent == 1);
#endif
		assert(g->active_memory < g->end_memory && g->active_memory >= g->start_memory);
		g->active_memory = g->active_memory->next;
	}

	assert(g->active_memory->pointer == 0);

	return g->active_memory;
}

static void phalcon_memory_restore_stack_common(zend_phalcon_globals *g)
{
	phalcon_memory_entry *prev, *active_memory;
	zval **ptr;
	size_t i;

	active_memory = g->active_memory;
	assert(active_memory != NULL);

	if (EXPECTED(!CG(unclean_shutdown))) {

		/* Traverse all zvals allocated, reduce the reference counting or free them */
		for (i = 0; i < active_memory->pointer; ++i) {
			ptr = active_memory->addresses[i];
			if (EXPECTED(ptr != NULL && *ptr != NULL)) {
				if (Z_REFCOUNTED_P(*ptr)) {
					if (Z_REFCOUNT_P(*ptr) <= 1) {
						if (!Z_ISREF_P(*ptr) || Z_TYPE_P(*ptr) == IS_OBJECT) {
							Z_DELREF_P(*ptr);
						} else {
							zval_ptr_dtor(*ptr);
							efree(*ptr);
							*ptr = NULL;
						}
					} else {
						Z_DELREF_P(*ptr);
					}
				} else {
					zval_ptr_dtor(*ptr);
					efree(*ptr);
					*ptr = NULL;
				}
			}
		}
	}

	prev = active_memory->prev;

	if (active_memory >= g->end_memory || active_memory < g->start_memory) {
		assert(prev != NULL);

		if (active_memory->addresses != NULL) {
			efree(active_memory->addresses);
		}

		efree(g->active_memory);
		g->active_memory = prev;
		prev->next = NULL;
	} else {
		active_memory->pointer      = 0;
		g->active_memory = prev;
	}
}

/**
 * Finishes the current memory stack by releasing allocated memory
 */
int ZEND_FASTCALL phalcon_memory_restore_stack(const char *func)
{
	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	if (UNEXPECTED(phalcon_globals_ptr->active_memory == NULL)) {
		fprintf(stderr, "WARNING: calling phalcon_memory_restore_stack() without an active memory frame!\n");
		phalcon_print_backtrace();
		return FAILURE;
	}

	if (UNEXPECTED(phalcon_globals_ptr->active_memory->func != func)) {
		fprintf(stderr, "Trying to free someone else's memory frame!\n");
		fprintf(stderr, "The frame was created by %s\n", phalcon_globals_ptr->active_memory->func);
		fprintf(stderr, "Calling function: %s\n", func);
		phalcon_print_backtrace();
	}

	phalcon_memory_restore_stack_common(phalcon_globals_ptr);
	return SUCCESS;
}

/**
 * Adds a memory frame in the current executed method
 */
void ZEND_FASTCALL phalcon_memory_grow_stack(const char *func)
{
	phalcon_memory_entry *entry;
	zend_phalcon_globals *g = PHALCON_VGLOBAL;	

	if (g->start_memory == NULL) {
		phalcon_initialize_memory(g);
	}

	entry = phalcon_memory_grow_stack_common(g);
	entry->func = func;
}

PHALCON_ATTR_NONNULL static void phalcon_reallocate_memory(const zend_phalcon_globals *g)
{
	phalcon_memory_entry *frame = g->active_memory;
	int persistent = (frame >= g->start_memory && frame < g->end_memory);
	void *buf = perealloc(frame->addresses, sizeof(zval **) * (frame->capacity + 16), persistent);
	if (EXPECTED(buf != NULL)) {
		frame->capacity += 16;
		frame->addresses = buf;
	}
	else {
		zend_error(E_CORE_ERROR, "Memory allocation failed");
	}

#ifndef PHALCON_RELEASE
	assert(frame->permanent == persistent);
#endif
}

PHALCON_ATTR_NONNULL1(2) static inline void phalcon_do_memory_observe(zval **var, const zend_phalcon_globals *g)
{
	phalcon_memory_entry *frame = g->active_memory;
	
#ifndef PHALCON_RELEASE
	if (UNEXPECTED(frame == NULL)) {
		fprintf(stderr, "PHALCON_MM_GROW() must be called before using any of MM functions or macros!");
		phalcon_print_backtrace();
		abort();
	}
#endif

	if (UNEXPECTED(frame->pointer == frame->capacity)) {
		phalcon_reallocate_memory(g);
	}
	
#ifndef PHALCON_RELEASE
	{
		size_t i;
		for (i = 0; i < frame->pointer; ++i) {
			if (frame->addresses[i] == var) {
				fprintf(stderr, "Variable %p is already observed", var);
				phalcon_print_backtrace();
				abort();
			}
		}
	}
#endif

	frame->addresses[frame->pointer] = var;
	++frame->pointer;
}

#ifndef PHALCON_RELEASE

static void phalcon_verify_frame(const phalcon_memory_entry *frame, const char *func, zval **var)
{
	size_t i;

	if (UNEXPECTED(frame == NULL)) {
		fprintf(stderr, "PHALCON_MM_GROW() must be called before using any of MM functions or macros!");
		phalcon_print_backtrace();
		abort();
	}

	if (func && strcmp(frame->func, func)) {
		fprintf(stderr, "Memory frames do not match: function: %s, frame creator: %s\n", func, frame->func);
		phalcon_print_backtrace();
		abort();
	}

	for (i=0; i<frame->pointer; ++i) {
		if (frame->addresses[i] == var) {
			fprintf(stderr, "Variable %p is already observed", var);
			phalcon_print_backtrace();
			abort();
		}
	}
}

#endif

/**
 * Observes a memory pointer to release its memory at the end of the request
 */
void ZEND_FASTCALL phalcon_memory_observe(zval **var, const char *func)
{
	zend_phalcon_globals *g     = PHALCON_VGLOBAL;
	phalcon_memory_entry *frame = g->active_memory;

#ifndef PHALCON_RELEASE
	phalcon_verify_frame(frame, func, var);
#endif

	phalcon_do_memory_observe(var, g);
	*var = NULL; /* In case an exception or error happens BEFORE the observed variable gets initialized */
}

/**
 * Observes a variable and allocates memory for it
 */
void ZEND_FASTCALL phalcon_memory_alloc(zval **var, const char *func)
{
	zend_phalcon_globals *g     = PHALCON_VGLOBAL;
	phalcon_memory_entry *frame = g->active_memory;

#ifndef PHALCON_RELEASE
	phalcon_verify_frame(frame, func, var);
#endif

	phalcon_do_memory_observe(var, g);
	PHALCON_ALLOC_INIT_ZVAL(*var);
}


/**
 * Cleans the phalcon memory stack recursivery
 */
int ZEND_FASTCALL phalcon_clean_restore_stack() {

	zend_phalcon_globals *phalcon_globals_ptr = PHALCON_VGLOBAL;

	while (phalcon_globals_ptr->active_memory != NULL) {
		phalcon_memory_restore_stack_common(phalcon_globals_ptr);
	}

	return SUCCESS;
}

/**
 * Exports symbols to the active symbol table
 */
int phalcon_set_symbol(zend_array *symbol_table, zval *key_name, zval *value)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	if (Z_TYPE_P(key_name) == IS_STRING) {
		Z_TRY_ADDREF_P(value);
		zend_hash_update(symbol_table, Z_STR_P(key_name), value);
	}

	return SUCCESS;
}

/**
 * Exports a string symbol to the active symbol table
 */
int phalcon_set_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length, zval *value)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	Z_TRY_ADDREF_P(value);
	zend_hash_str_update(symbol_table, key_name, key_length, value);

	return SUCCESS;
}

int phalcon_del_symbol(zend_array *symbol_table, zval *key_name)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	if (Z_TYPE_P(key_name) == IS_STRING) {
		zend_hash_del(symbol_table, Z_STR_P(key_name));
	}

	return SUCCESS;
}

int phalcon_del_symbol_str(zend_array *symbol_table, char *key_name, unsigned int key_length)
{
	if (!symbol_table) {
		php_error_docref(NULL, E_WARNING, "Cannot find a valid symbol_table");
		return FAILURE;
	}

	zend_hash_str_del(symbol_table, key_name, key_length);

	return SUCCESS;
}
