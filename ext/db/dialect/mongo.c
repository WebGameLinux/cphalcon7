
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
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/

#include "db/dialect/mongo.h"
#include "db/dialectinterface.h"
#include "db/column.h"
#include "db/columninterface.h"
#include "db/indexinterface.h"
#include "db/exception.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/concat.h"
#include "kernel/array.h"
#include "kernel/string.h"

/**
 * Phalcon\Db\Dialect\Mongo
 *
 * Generates database specific SQL for the MongoDB RDBMS
 */
zend_class_entry *phalcon_db_dialect_mongo_ce;

PHP_METHOD(Phalcon_Db_Dialect, select);
PHP_METHOD(Phalcon_Db_Dialect, insert);
PHP_METHOD(Phalcon_Db_Dialect, update);
PHP_METHOD(Phalcon_Db_Dialect, delete);

static const zend_function_entry phalcon_db_dialect_mongo_method_entry[] = {
	PHP_ME(Phalcon_Db_Dialect_Mongo, select, arginfo_phalcon_db_dialectinterface_select, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, insert, arginfo_phalcon_db_dialectinterface_insert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, update, arginfo_phalcon_db_dialectinterface_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, delete, arginfo_phalcon_db_dialectinterface_delete, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Dialect\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Dialect_Mongo){

	PHALCON_REGISTER_CLASS(Phalcon\\Db\\Dialect, Mongo, db_dialect_mongo, phalcon_db_dialect_mongo_method_entry, 0);

	zend_class_implements(phalcon_db_dialect_mongo_ce, 1, phalcon_db_dialectinterface_ce);

	return SUCCESS;
}

/**
 * Builds a SELECT statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,select){

	zval *definition, tables = {}, columns = {}, distinct = {}, where = {}, group = {}, having = {}, order = {}, limit = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid SELECT definition");
		return;
	}
	if (!phalcon_array_isset_fetch_str(&tables, definition, SL("tables"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'tables' is required in the definition array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&columns, definition, SL("columns"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'columns' is required in the definition array");
		return;
	}

	if (Z_TYPE_P(&columns) == IS_ARRAY) {
	}

	if (phalcon_array_isset_fetch_str(&distinct, definition, SL("distinct"))) {
	}

	/* Check for a WHERE clause */
	if (phalcon_array_isset_fetch_str(&where, definition, SL("where"))) {
	}

	/* Check for a GROUP clause */
	if (phalcon_array_isset_fetch_str(&group, definition, SL("group"))) {
	}

	/* Check for a HAVING clause */
	if (phalcon_array_isset_fetch_str(&having, definition, SL("having"))) {
	}

	/* Check for a ORDER clause */
	if (phalcon_array_isset_fetch_str(&order, definition, SL("order"))) {
	}

	/**
	 * Check for a LIMIT condition
	 */
	if (phalcon_array_isset_fetch_str(&limit, definition, SL("limit"))) {
	}
}

/**
 * Builds a INSERT statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,insert)
{
	zval *definition, table = {}, fields = {}, values = {}, number_fields = {}, exception_message = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid INSERT definition");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&table, definition, SL("table"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'table' is required in the definition array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&fields, definition, SL("fields"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'fields' is required in the definition array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&values, definition, SL("values"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'values' is required in the definition array");
		return;
	}

	if (unlikely(Z_TYPE(values) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The second parameter for insert isn't an Array");
		return;
	}

	/**
	 * A valid array with more than one element is required
	 */
	if (!phalcon_fast_count_ev(&values)) {
		PHALCON_CONCAT_SVS(&exception_message, "Unable to insert into ", &table, " without data");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}
}

/**
 * Builds a UPDATE statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,update){

	zval *definition, tables = {}, fields = {}, values = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid Update definition");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&tables, definition, SL("tables"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'tables' is required in the definition array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&fields, definition, SL("fields"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'fields' is required in the definition array");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&values, definition, SL("values"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'fields' is required in the definition array");
		return;
	}
}

/**
 * Builds a DELETE statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,delete){

	zval *definition, tables = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid Update definition");
		return;
	}
	if (!phalcon_array_isset_fetch_str(&tables, definition, SL("tables"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'tables' is required in the definition array");
		return;
	}
}
