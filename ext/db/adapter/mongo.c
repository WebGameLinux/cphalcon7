
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

#include "db/adapter/mongo.h"
#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/exception.h"
#include "db/result/mongo.h"
#include "db/column.h"
#include "debug.h"

#include <bson.h>
#include <bcon.h>
#include <mongoc.h>

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/array.h"
#include "kernel/exception.h"
#include "kernel/fcall.h"
#include "kernel/object.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/string.h"
#include "kernel/operators.h"
#include "kernel/debug.h"

/**
 * Phalcon\Db\Adapter\Mongo
 *
 * Phalcon\Db\Adapter\Mongo is the Phalcon\Db that internally uses libmongoc to connect to a MongoDB
 *
 *<code>
 *	$connection = new Phalcon\Db\Adapter\Mongo(array(
 *		'uri' => "mongodb://localhost:27017",
 *      'db' => 'phalcon_test',
 *		'collection' => 'rabots'
 *	));
 *</code>
 */
zend_class_entry *phalcon_db_adapter_mongo_ce;

PHP_METHOD(Phalcon_Db_Adapter_Mongo, __construct);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, connect);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, query);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, execute);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, close);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeIdentifier);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeString);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, lastInsertId);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getInternalHandler);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getErrorInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, descriptor)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_mongo_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Mongo, __construct, arginfo_phalcon_db_adapter_mongo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter_Mongo, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, query, arginfo_phalcon_db_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, execute, arginfo_phalcon_db_adapterinterface_execute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeString, arginfo_phalcon_db_adapterinterface_escapestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, lastInsertId, arginfo_phalcon_db_adapterinterface_lastinsertid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, getInternalHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, getErrorInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_object_handlers phalcon_db_adapter_mongo_object_handlers;
zend_object* phalcon_db_adapter_mongo_object_create_handler(zend_class_entry *ce)
{
	phalcon_db_adapter_mongo_object *intern = ecalloc(1, sizeof(phalcon_db_adapter_mongo_object) + zend_object_properties_size(ce));
	intern->std.ce = ce;

	zend_object_std_init(&intern->std, ce);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &phalcon_db_adapter_mongo_object_handlers;

	intern->client = NULL;
	intern->collection = NULL;
	return &intern->std;
}

void phalcon_db_adapter_mongo_object_free_handler(zend_object *object)
{
	phalcon_db_adapter_mongo_object *intern;
	intern = phalcon_db_adapter_mongo_object_from_obj(object);
	if (intern->collection) {
		mongoc_collection_destroy(intern->collection);
		intern->collection = NULL;
	}
	if (intern->client) {
		mongoc_client_destroy(intern->client);
		intern->client = NULL;
	}
	zend_object_std_dtor(object);
}

/**
 * Phalcon\Db\Adapter\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Mongo){

	PHALCON_REGISTER_CLASS_CREATE_OBJECT_EX(Phalcon\\Db\\Adapter, Mongo, db_adapter_mongo, phalcon_db_adapter_ce, phalcon_db_adapter_mongo_method_entry, 0);

	zend_declare_property_null(phalcon_db_adapter_mongo_ce, SL("_affectedRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_mongo_ce, SL("_transactionLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_mongo_ce, SL("_schema"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Constructor for Phalcon\Db\Adapter\Mongo
 *
 * @param array $descriptor
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, __construct){

	zval *descriptor;

	phalcon_fetch_params(0, 1, 0, &descriptor);

	if (Z_TYPE_P(descriptor) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The descriptor must be an array");
		return;
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "connect", descriptor);
	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_mongo_ce, getThis(), "__construct", descriptor);
}

/**
 * This method is automatically called in Phalcon\Db\Adapter\Mongo constructor.
 * Call it when you need to restore a database connection
 *
 *<code>
 * //Make a connection
 * $connection = new Phalcon\Db\Adapter\Mongo(array(
 *		'uri' => "mongodb://localhost:27017",
 *      'db' => 'phalcon_test',
 *		'collection' => 'rabots'
 * ));
 *
 * //Reconnect
 * $connection->connect();
 * </code>
 *
 * @param array $descriptor
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, connect)
{
	zval *_descriptor = NULL, descriptor = {}, uri = {}, db = {};
	phalcon_cache_backend_mongo_object *mongo_object;

	phalcon_fetch_params(0, 0, 1, &_descriptor);

	if (!_descriptor || Z_TYPE_P(_descriptor) == IS_NULL) {
		phalcon_return_property(&descriptor, getThis(), SL("_descriptor"));
	} else {
		PHALCON_CPY_WRT(&descriptor, _descriptor);
	}

	if (!phalcon_array_isset_fetch_str(&uri, &descriptor, SL("uri"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'uri' is required in the descriptor array");
		return;
	}

	if (phalcon_array_isset_fetch_str(&db, &descriptor, SL("db"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'db' is required in the descriptor array");
		return;
	}

	mongo_object = phalcon_cache_backend_mongo_object_from_obj(Z_OBJ_P(getThis()));
	if (mongo_object->collection) {
		mongoc_collection_destroy(mongo_object->collection);
		mongo_object->collection = NULL;
	}
	if (mongo_object->client) {
		mongoc_client_destroy(mongo_object->client);
		mongo_object->client = NULL;
	}
	mongo_object->client = mongoc_client_new(Z_STRVAL(uri));
	if (!mongo_object->client) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The URI parsed unsuccessfully");
		return;
	}
}

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server is returning rows
 *
 *<code>
 *	//Querying data
 *	$resultset = $connection->query("SELECT * FROM robots WHERE type='mechanical'");
 *	$resultset = $connection->query("SELECT * FROM robots WHERE type=?", array("mechanical"));
 *</code>
 *
 * @param string $sqlStatement
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, query){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL;

	phalcon_fetch_params(0, 1, 2, &statement, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", statement, bind_params, bind_types, &PHALCON_GLOBAL(z_true));
}

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server doesn't return any row
 *
 *<code>
 *	//Inserting data
 *	$success = $connection->execute("INSERT INTO robots VALUES (1, 'Astro Boy')");
 *	$success = $connection->execute("INSERT INTO robots VALUES (?, ?)", array(1, 'Astro Boy'));
 *</code>
 *
 * @param  string $sqlStatement
 * @param  array $bindParams
 * @param  array $bindTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, execute){

	zval *statement, *bind_params = NULL, *bind_types = NULL, *flag = NULL, type = {}, debug_message = {}, processed_sql = {}, *value = NULL, processed_params = {}, processed_types = {};
	zval events_manager = {}, event_name = {}, status = {}, affected_rows = {};
	zval statement = {}, new_statement = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 3, &statement, &bind_params, &bind_types, &flag);

	if (Z_TYPE_P(statement) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_cache_exception_ce, "The statement must be an arry unsuccessfully");
		return;
	}

	if (!phalcon_array_isset_fetch_str(&type, statement, SL("type"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The parameter 'type' is required");
		return;
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (!flag) {
		flag = &PHALCON_GLOBAL(z_false);
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "SQL STATEMENT: ", statement);
		PHALCON_DEBUG_LOG(&debug_message);
		if (bind_params && PHALCON_IS_NOT_EMPTY(bind_params)) {
			PHALCON_STR(&debug_message, "Bind Params: ");
			PHALCON_DEBUG_LOG(&debug_message);
			PHALCON_DEBUG_LOG(bind_params);
		}
		if (bind_types && PHALCON_IS_NOT_EMPTY(bind_types)) {
			PHALCON_STR(&debug_message, "Bind Types: ");
			PHALCON_DEBUG_LOG(&debug_message);
			PHALCON_DEBUG_LOG(bind_types);
		}
	}

	/**
	 * Execute the beforeQuery event if a EventsManager is available
	 */
	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		phalcon_update_property_zval(getThis(), SL("_sqlStatement"), &processed_sql);
		phalcon_update_property_zval(getThis(), SL("_sqlVariables"), &processed_params);
		phalcon_update_property_zval(getThis(), SL("_sqlBindTypes"), &processed_types);

		PHALCON_STR(&event_name, "db:beforeExecute");

		PHALCON_CALL_METHOD(&status, &events_manager, "fire", &event_name, getThis(), &processed_params);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	if (zend_is_true(flag)) {
		/**
		 * Execute the afterQuery event if a EventsManager is available
		 */
		if (likely(Z_TYPE(new_statement) == IS_OBJECT)) {
			if (Z_TYPE(events_manager) == IS_OBJECT) {
				PHALCON_STR(&event_name, "db:afterExecute");
				PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &processed_params);
			}

			object_init_ex(return_value, phalcon_db_result_pdo_ce);
			PHALCON_CALL_METHOD(NULL, return_value, "__construct", getThis(), &new_statement, &processed_sql, &processed_params, &processed_types);
			return;
		}

		RETURN_CTOR(&new_statement);
	}

	if (likely(Z_TYPE(new_statement) == IS_OBJECT)) {
		PHALCON_CALL_METHOD(&affected_rows, &new_statement, "rowcount");
	} else {
		ZVAL_LONG(&affected_rows, 0);
	}

	phalcon_update_property_zval(getThis(), SL("_affectedRows"), &affected_rows);
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		PHALCON_STR(&event_name, "db:afterExecute");
		PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &processed_params);
	}

	RETURN_TRUE;
}

/**
 * Convert php bytea to database bytea
 *
 * @param string $value
 * @return string
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	RETURN_CTOR(value);
}

/**
 * Convert database bytea to php bytea
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeBytea){

	zval *value;

	phalcon_fetch_params(0, 1, 0, &value);

	RETURN_CTOR(value);
}

/**
 * Convert php array to database array
 *
 * @param array $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeArray){

	zval *value, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &value, &type);

	RETURN_CTOR(value);
}

/**
 * Convert database array to php array
 *
 * @param string $value
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeArray){

	zval *value, *type = NULL;

	phalcon_fetch_params(0, 1, 1, &value, &type);

	RETURN_CTOR(value);
}

/**
 * Escapes a column/table/schema name
 *
 *<code>
 *	$escapedTable = $connection->escapeIdentifier('robots');
 *	$escapedTable = $connection->escapeIdentifier(array('store', 'robots'));
 *</code>
 *
 * @param string $identifier
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeIdentifier){

	zval *identifier, domain = {}, name = {};

	phalcon_fetch_params(0, 1, 0, &identifier);

	if (Z_TYPE_P(identifier) == IS_ARRAY) {
		phalcon_array_fetch_long(&domain, identifier, 0, PH_NOISY);
		phalcon_array_fetch_long(&name, identifier, 1, PH_NOISY);
		PHALCON_CONCAT_SVSVS(return_value, "\"", &domain, "\".\"", &name, "\"");
		return;
	}

	PHALCON_CONCAT_SVS(return_value, "\"", identifier, "\"");
}

/**
 * Escapes a value to avoid SQL injections according to the active charset in the connection
 *
 *<code>
 *	$escapedStr = $connection->escapeString('some dangerous value');
 *</code>
 *
 * @param string $str
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeString){

	zval *str, mongo = {};

	phalcon_fetch_params(0, 1, 0, &str);

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&mongo, "quote", str);
}

/**
 * Converts bound parameters such as :name: or ?1 into MONGO bind params ?
 *
 *<code>
 * print_r($connection->convertBoundParams('SELECT * FROM robots WHERE name = :name:', array('Bender')));
 *</code>
 *
 * @param string $sql
 * @param array $params
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, convertBoundParams){

	zval *sql, *params, query_params = {}, placeholders = {}, matches = {}, set_order = {}, bind_pattern = {}, status = {}, *place_match = NULL, question = {}, bound_sql = {};

	phalcon_fetch_params(0, 2, 0, &sql, &params);

	array_init(&query_params);
	array_init(&placeholders);
	ZVAL_LONG(&set_order, 2);

	PHALCON_STR(&bind_pattern, "/\\?([0-9]+)|:([a-zA-Z0-9_]+):/");

	ZVAL_MAKE_REF(&matches);
	PHALCON_CALL_FUNCTION(&status, "preg_match_all", &bind_pattern, sql, &matches, &set_order);
	ZVAL_UNREF(&matches);

	if (zend_is_true(&status)) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(matches), place_match) {
			zval numeric_place = {}, value = {}, str_place = {};

			phalcon_array_fetch_long(&numeric_place, place_match, 1, PH_NOISY);
			if (!phalcon_array_isset_fetch(&value, params, &numeric_place, 0)) {
				if (phalcon_array_isset_fetch_long(&str_place, place_match, 2)) {
					if (!phalcon_array_isset_fetch(&value, params, &str_place, 0)) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Matched parameter wasn't found in parameters list");
						return;
					}
				} else {
					PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Matched parameter wasn't found in parameters list");
					return;
				}
			}
			phalcon_array_append(&placeholders, &value, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		PHALCON_STR(&question, "?");

		PHALCON_CALL_FUNCTION(&bound_sql, "preg_replace", &bind_pattern, &question, sql);
	} else {
		PHALCON_CPY_WRT_CTOR(&bound_sql, sql);
	}

	/**
	 * Returns an array with the processed SQL and parameters
	 */
	array_init_size(return_value, 2);
	phalcon_array_update_str(return_value, SL("sql"), &bound_sql, PH_COPY);
	phalcon_array_update_str(return_value, SL("params"), &placeholders, PH_COPY);
}

/**
 * Returns the insert id for the auto_increment/serial column inserted in the lastest executed SQL statement
 *
 *<code>
 * //Inserting a new robot
 * $success = $connection->insert(
 *     "robots",
 *     array("Astro Boy", 1952),
 *     array("name", "year")
 * );
 *
 * //Getting the generated id
 * $id = $connection->lastInsertId();
 *</code>
 *
 * @param string $sequenceName
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, lastInsertId){

	zval *sequence_name = NULL, mongo;

	phalcon_fetch_params(0, 0, 1, &sequence_name);

	if (!sequence_name) {
		sequence_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (Z_TYPE(mongo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	PHALCON_RETURN_CALL_METHOD(&mongo, "lastinsertid", sequence_name);
}

/**
 * Return internal MONGO handler
 *
 * @return \MONGO
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getInternalHandler){

	phalcon_read_property(return_value, getThis(), SL("_mongo"), PH_NOISY);
}

/**
 * Return the error info, if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getErrorInfo){

	zval mongo = {};

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&mongo, "errorinfo");
}
