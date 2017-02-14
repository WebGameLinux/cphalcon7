
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

#include <ext/mongo/php_mongo_driver.h>

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
 * Phalcon\Db\Adapter\Mongo is the Phalcon\Db that internally uses MONGO to connect to a database
 *
 *<code>
 *	$connection = new Phalcon\Db\Adapter\Mongo(array(
 *		'uri' => "mongodb://localhost:27017",
 *      'db' => 'caches',
 *		'collection' => 'images'
 *	));
 *</code>
 */
zend_class_entry *phalcon_db_adapter_mongo_ce;

PHP_METHOD(Phalcon_Db_Adapter_Mongo, __construct);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, connect);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeColumns);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeIndexes);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeReferences);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeBytea);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, unescapeArray);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, prepare);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, executePrepared);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, query);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, execute);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, affectedRows);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, close);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeIdentifier);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, escapeString);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, convertBoundParams);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, lastInsertId);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, begin);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, rollback);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, commit);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getTransactionLevel);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, isUnderTransaction);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getInternalHandler);
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getErrorInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, descriptor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlStatement)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo_executeprepared, 0, 0, 1)
	ZEND_ARG_INFO(0, statement)
	ZEND_ARG_INFO(0, placeholders)
	ZEND_ARG_INFO(0, dataTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo_begin, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo_rollback, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_mongo_commit, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_mongo_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Mongo, __construct, arginfo_phalcon_db_adapter_mongo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter_Mongo, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, describeColumns, arginfo_phalcon_db_adapterinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, describeIndexes, arginfo_phalcon_db_adapterinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, describeReferences, arginfo_phalcon_db_adapterinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, prepare, arginfo_phalcon_db_adapter_mongo_prepare, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, executePrepared, arginfo_phalcon_db_adapter_mongo_executeprepared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, query, arginfo_phalcon_db_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, execute, arginfo_phalcon_db_adapterinterface_execute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, affectedRows, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, escapeString, arginfo_phalcon_db_adapterinterface_escapestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, convertBoundParams, arginfo_phalcon_db_adapterinterface_convertboundparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, lastInsertId, arginfo_phalcon_db_adapterinterface_lastinsertid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, begin, arginfo_phalcon_db_adapter_mongo_begin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, rollback, arginfo_phalcon_db_adapter_mongo_rollback, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, commit, arginfo_phalcon_db_adapter_mongo_commit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, getTransactionLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, isUnderTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, getInternalHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Mongo, getErrorInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Mongo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter, Mongo, db_adapter_mongo, phalcon_db_adapter_ce, phalcon_db_adapter_mongo_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_db_adapter_mongo_ce, SL("_mongo"), ZEND_ACC_PROTECTED);
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
 * $connection = new Phalcon\Db\Adapter\Mongo\Mysql(array(
 *  'host' => '192.168.0.11',
 *  'username' => 'sigma',
 *  'password' => 'secret',
 *  'dbname' => 'blog',
 * ));
 *
 * //Reconnect
 * $connection->connect();
 * </code>
 *
 * @param 	array $descriptor
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, connect)
{
	zval *_descriptor = NULL, descriptor = {}, username = {}, password = {}, options = {}, dsn_parts = {}, *value, dsn_attributes = {}, mongo_type = {}, dsn = {}, persistent = {}, mongo = {};
	zend_class_entry *ce;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 0, 1, &_descriptor);

	if (!_descriptor || Z_TYPE_P(_descriptor) == IS_NULL) {
		phalcon_return_property(&descriptor, getThis(), SL("_descriptor"));
	} else {
		PHALCON_CPY_WRT(&descriptor, _descriptor);
	}

	if (Z_TYPE(descriptor) != IS_ARRAY) {
		RETURN_FALSE;
	}

	/**
	 * Check for a username or use null as default
	 */
	if (phalcon_array_isset_fetch_str(&username, &descriptor, SL("username"))) {
		phalcon_array_unset_str(&descriptor, SL("username"), PH_SEPARATE);
	}

	/**
	 * Check for a password or use null as default
	 */
	if (phalcon_array_isset_fetch_str(&password, &descriptor, SL("password"))) {
		phalcon_array_unset_str(&descriptor, SL("password"), PH_SEPARATE);
	}

	/**
	 * Check if the developer has defined custom options or create one from scratch
	 */
	if (phalcon_array_isset_fetch_str(&options, &descriptor, SL("options"))) {
		phalcon_array_unset_str(&descriptor, SL("options"), PH_SEPARATE);
	} else {
		array_init(&options);
	}

	/**
	 * Remove the dialectClass from the descriptor if any
	 */
	if (phalcon_array_isset_str(&descriptor, SL("dialectClass"))) {
		phalcon_array_unset_str(&descriptor, SL("dialectClass"), PH_SEPARATE);
	}

	/**
	 * Check if the user has defined a custom dsn
	 */
	if (!phalcon_array_isset_fetch_str(&dsn_attributes, &descriptor, SL("dsn"))) {
		array_init(&dsn_parts);

		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(descriptor), idx, str_key, value) {
			zval key = {}, dsn_attribute = {};
			if (str_key) {
				ZVAL_STR(&key, str_key);
			} else {
				ZVAL_LONG(&key, idx);
			}
			PHALCON_CONCAT_VSV(&dsn_attribute, &key, "=", value);
			phalcon_array_append(&dsn_parts, &dsn_attribute, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&dsn_attributes, SL(";"), &dsn_parts);
	}

	phalcon_read_property(&mongo_type, getThis(), SL("_type"), PH_NOISY);

	PHALCON_CONCAT_VSV(&dsn, &mongo_type, ":", &dsn_attributes);

	/**
	 * Default options
	 */
	phalcon_array_update_long_long(&options, MONGO_ATTR_ERRMODE, MONGO_ERRMODE_EXCEPTION, PH_COPY);

	/**
	 * Check if the connection must be persistent
	 */
	if (phalcon_array_isset_fetch_str(&persistent, &descriptor, SL("persistent"))) {
		phalcon_array_unset_str(&descriptor, SL("persistent"), PH_SEPARATE);
		if (zend_is_true(&persistent)) {
			phalcon_array_update_long_bool(&options, MONGO_ATTR_PERSISTENT, 1, PH_COPY);
		}
	}

	/**
	 * Create the connection using MONGO
	 */
	ce = phalcon_fetch_str_class(SL("MONGO"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_OBJECT_INIT(&mongo, ce);
	PHALCON_CALL_METHOD(NULL, &mongo, "__construct", &dsn, &username, &password, &options);

	phalcon_update_property_zval(getThis(), SL("_mongo"), &mongo);
}



/**
 * Returns an array of Phalcon\Db\Column objects describing a table
 *
 * <code>
 * print_r($connection->describeColumns("posts")); ?>
 * </code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Column[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeColumns){

	zval *table, *schema = NULL, columns = {}, dialect = {}, size_pattern = {}, sql = {}, fetch_num = {}, describe = {}, old_column = {}, *field;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	array_init(&columns);

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY);

	ZVAL_STRING(&size_pattern, "#\\(([0-9]++)(?:,\\s*([0-9]++))?\\)#");

	PHALCON_CALL_METHOD(&sql, &dialect, "describecolumns", table, schema);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, MONGO_FETCH_NUM);

	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), field) {
		zval definition = {}, column_type = {}, matches = {}, pos = {}, match_one = {}, match_two = {}, attribute = {}, column_name = {}, column = {};

		array_init_size(&definition, 1);
		add_assoc_long_ex(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_STR);

		phalcon_array_fetch_long(&column_type, field, 2, PH_NOISY);

		if (phalcon_memnstr_str(&column_type, SL("("))) {
			ZVAL_NULL(&matches);
			ZVAL_MAKE_REF(&matches);
			RETURN_ON_FAILURE(phalcon_preg_match(&pos, &size_pattern, &column_type, &matches));
			ZVAL_UNREF(&matches);
			if (zend_is_true(&pos)) {
				if (phalcon_array_isset_fetch_long(&match_one, &matches, 1)) {
					convert_to_long(&match_one);
					phalcon_array_update_str(&definition, SL("size"), &match_one, PH_COPY);
					phalcon_array_update_str(&definition, SL("bytes"), &match_one, PH_COPY);
				}
				if (phalcon_array_isset_fetch_long(&match_two, &matches, 2)) {
					convert_to_long(&match_two);
					phalcon_array_update_str(&definition, SL("scale"), &match_two, PH_COPY);
				}
			}
		}

		/**
		 * Check the column type to get the correct Phalcon type
		 */
		while (1) {
			/**
			 * Tinyint(1) is boolean
			 */
			if (phalcon_memnstr_str(&column_type, SL("tinyint(1)"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_BOOLEAN, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), 5, 0);
				ZVAL_STRING(&column_type, "boolean"); // Change column type to skip size check.
				break;
			}

			/**
			 * Smallint
			 */
			if (phalcon_memnstr_str(&column_type, SL("smallint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 2, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Mediumint
			 */
			if (phalcon_memnstr_str(&column_type, SL("mediumint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 3, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * BIGINT
			 */
			if (phalcon_memnstr_str(&column_type, SL("bigint"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Integers/Int are int
			 */
			phalcon_fast_stripos_str(&pos, &column_type, SL("int"));
			if (PHALCON_IS_NOT_FALSE(&pos)) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_INTEGER, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_INT, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);

				phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);

				/**
				 * Check if the column is auto increment
				 */
				if (zend_is_true(&attribute)) {
					phalcon_array_update_str_bool(&definition, SL("autoIncrement"), 1, 0);
				}
				break;
			}

			/**
			 * Float/Smallfloats/Decimals are float
			 */
			if (phalcon_memnstr_str(&column_type, SL("float"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_FLOAT, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 4, 0);
				break;
			}

			/**
			 * Double are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("double"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DOUBLE, 0);
				phalcon_array_update_str(&definition, SL("isNumeric"), &PHALCON_GLOBAL(z_true), PH_COPY);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				phalcon_array_update_str_long(&definition, SL("bytes"), 8, 0);
				break;
			}

			/**
			 * Decimals are floats
			 */
			if (phalcon_memnstr_str(&column_type, SL("decimal"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DECIMAL, 0);
				phalcon_array_update_str_bool(&definition, SL("isNumeric"), 1, 0);
				phalcon_array_update_str_long(&definition, SL("bindType"), PHALCON_DB_COLUMN_BIND_PARAM_DECIMAL, 0);
				if (phalcon_is_numeric(&match_one)) {
					phalcon_array_update_str_long(&definition, SL("bytes"), Z_LVAL(match_one) * 8, 0);
				} else {
					phalcon_array_update_str_long(&definition, SL("size"), 5, 0);
					phalcon_array_update_str_long(&definition, SL("bytes"), 5, 0);
				}
				if (phalcon_is_numeric(&match_two)) {
					phalcon_array_update_str(&definition, SL("scale"), &match_two, PH_COPY);
				} else {
					phalcon_array_update_str_long(&definition, SL("scale"), 2, 0);
				}
				break;
			}

			/**
			 * Varchar are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("varchar"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
				break;
			}

			/**
			 * Date/Datetime are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("date"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATE, 0);
				break;
			}

			/**
			 * Timestamp as date
			 */
			if (phalcon_memnstr_str(&column_type, SL("timestamp"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TIMESTAMP, 0);
				break;
			}

			/**
			 * Chars are chars
			 */
			if (phalcon_memnstr_str(&column_type, SL("char"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				break;
			}

			/**
			 * Special type for datetime
			 */
			if (phalcon_memnstr_str(&column_type, SL("datetime"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_DATETIME, 0);
				break;
			}

			/**
			 * Text are varchars
			 */
			if (phalcon_memnstr_str(&column_type, SL("text"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_TEXT, 0);
				break;
			}

			/**
			 * Enum are treated as char
			 */
			if (phalcon_memnstr_str(&column_type, SL("enum"))) {
				phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_CHAR, 0);
				break;
			}

			/**
			 * By default is string
			 */
			phalcon_array_update_str_long(&definition, SL("type"), PHALCON_DB_COLUMN_TYPE_VARCHAR, 0);
			break;
		}

		if (phalcon_memnstr_str(&column_type, SL("unsigned"))) {
			phalcon_array_update_str_bool(&definition, SL("unsigned"), 1, 0);
		}

		if (Z_TYPE(old_column) <= IS_NULL) {
			phalcon_array_update_str_bool(&definition, SL("first"), 1, 0);
		} else {
			phalcon_array_update_str(&definition, SL("after"), &old_column, PH_COPY);
		}

		/**
		 * Check if the field is primary key
		 */
		phalcon_array_fetch_long(&attribute, field, 5, PH_NOISY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("primary"), 1, 0);
		}

		/**
		 * Check if the column allows null values
		 */
		phalcon_array_fetch_long(&attribute, field, 3, PH_NOISY);
		if (zend_is_true(&attribute)) {
			phalcon_array_update_str_bool(&definition, SL("notNull"), 1, 0);
		}

		phalcon_array_fetch_long(&column_name, field, 1, PH_NOISY);

		/**
		 * If the column set the default values, get it
		 */
		phalcon_array_fetch_long(&attribute, field, 4, PH_NOISY);
		if (!PHALCON_IS_EMPTY(&attribute)) {
			phalcon_array_update_str(&definition, SL("default"), &attribute, PH_COPY);
		}

		/**
		 * Every column is stored as a Phalcon\Db\Column
		 */
		object_init_ex(&column, phalcon_db_column_ce);
		PHALCON_CALL_METHOD(NULL, &column, "__construct", &column_name, &definition);

		phalcon_array_append(&columns, &column, PH_COPY);
		PHALCON_CPY_WRT_CTOR(&old_column, &column_name);

	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(&columns);
}

/**
 * Lists table indexes
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Index[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeIndexes){

	zval *table, *_schema = NULL, schema = {}, dialect = {}, fetch_num = {}, sql = {}, describe = {}, indexes = {}, *index, index_objects = {}, *index_columns;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &_schema);

	if (!_schema || !zend_is_true(_schema)) {
		phalcon_read_property(&schema, getThis(), SL("_schema"), PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&schema, _schema);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, MONGO_FETCH_NUM);

	PHALCON_CALL_METHOD(&sql, &dialect, "describeindexes", table, &schema);
	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);

	/**
	 * Cryptic Guide: 0: position, 1: name
	 */
	array_init(&indexes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), index) {
		zval key_name = {}, sql_index_describe = {}, describe_index = {}, *index_column;

		phalcon_array_fetch_long(&key_name, index, 1, PH_NOISY);

		PHALCON_CALL_METHOD(&sql_index_describe, &dialect, "describeindex", &key_name);
		PHALCON_CALL_METHOD(&describe_index, getThis(), "fetchall", &sql_index_describe, &fetch_num);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe_index), index_column) {
			zval column_name = {};
			phalcon_array_fetch_long(&column_name, index_column, 2, PH_NOISY);
			phalcon_array_append_multi_2(&indexes, &key_name, &column_name, PH_COPY);
		} ZEND_HASH_FOREACH_END();
	} ZEND_HASH_FOREACH_END();

	array_init(&index_objects);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(indexes), idx, str_key, index_columns) {
		zval name = {}, index = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		object_init_ex(&index, phalcon_db_index_ce);
		PHALCON_CALL_METHOD(NULL, &index, "__construct", &name, index_columns);

		phalcon_array_update_zval(&index_objects, &name, &index, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(&index_objects);
}

/**
 * Lists table references
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Reference[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, describeReferences){

	zval *table, *schema = NULL, dialect = {}, sql, fetch_num, describe, reference_objects, *reference_describe;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&dialect, getThis(), SL("_dialect"), PH_NOISY);

	/**
	 * Get the SQL to describe the references
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "describereferences", table, schema);

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, MONGO_FETCH_NUM);

	/**
	 * Execute the SQL describing the references
	 */
	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);

	/**
	 * Cryptic Guide: 2: table, 3: from, 4: to
	 */
	array_init(&reference_objects);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(describe), idx, str_key, reference_describe) {
		zval number = {}, constraint_name = {}, referenced_table = {}, from = {}, to = {}, columns = {}, referenced_columns = {}, reference_array = {}, reference = {};
		if (str_key) {
			ZVAL_STR(&number, str_key);
		} else {
			ZVAL_LONG(&number, idx);
		}

		PHALCON_CONCAT_SV(&constraint_name, "foreign_key_", &number);
		phalcon_array_fetch_long(&referenced_table, reference_describe, 2, PH_NOISY);
		phalcon_array_fetch_long(&from, reference_describe, 3, PH_NOISY);
		phalcon_array_fetch_long(&to, reference_describe, 4, PH_NOISY);

		array_init_size(&columns, 1);
		phalcon_array_append(&columns, &from, PH_COPY);

		array_init_size(&referenced_columns, 1);
		phalcon_array_append(&referenced_columns, &to, PH_COPY);

		array_init_size(&reference_array, 4);
		add_assoc_null_ex(&reference_array, SL("referencedSchema"));
		phalcon_array_update_str(&reference_array, SL("referencedTable"), &referenced_table, PH_COPY);
		phalcon_array_update_str(&reference_array, SL("columns"), &columns, PH_COPY);
		phalcon_array_update_str(&reference_array, SL("referencedColumns"), &referenced_columns, PH_COPY);

		/**
		 * Every route is abstracted as a Phalcon\Db\Reference instance
		 */
		object_init_ex(&reference, phalcon_db_reference_ce);
		PHALCON_CALL_METHOD(NULL, &reference, "__construct", &constraint_name, &reference_array);
		phalcon_array_update_zval(&reference_objects, &constraint_name, &reference, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	RETURN_CTOR(&reference_objects);
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
 * Returns a MONGO prepared statement to be executed with 'executePrepared'
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $mongoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param string $sqlStatement
 * @return \MONGOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, prepare){

	zval *sql_statement, mongo = {};

	phalcon_fetch_params(0, 1, 0, &sql_statement);

	phalcon_update_property_zval(getThis(), SL("_sqlStatement"), sql_statement);

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&mongo, "prepare", sql_statement);
}

/**
 * Executes a prepared statement binding. This function uses integer indexes starting from zero
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $mongoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param \MONGOStatement $statement
 * @param array $placeholders
 * @param array $dataTypes
 * @return \MONGOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, executePrepared){

	zval *statement, *placeholders, *data_types, *z_one, *value = NULL, profiler = {}, sql_statement = {};
	zend_string *str_key;
	ulong idx;
	int is_array;

	phalcon_fetch_params(0, 1, 2, &statement, &placeholders, &data_types);

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(placeholders) == IS_ARRAY) {
		z_one = &PHALCON_GLOBAL(z_one);
		is_array = Z_TYPE_P(data_types) == IS_ARRAY ? 1 : 0;
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(placeholders), idx, str_key, value) {
			zval wildcard = {}, parameter = {}, type = {}, cast_value = {};
			if (str_key) {
				ZVAL_STR(&wildcard, str_key);
			} else {
				ZVAL_LONG(&wildcard, idx);
			}

			if (Z_TYPE(wildcard) == IS_LONG) {
				phalcon_add_function(&parameter, &wildcard, z_one);
			} else {
				if (Z_TYPE(wildcard) == IS_STRING) {
					PHALCON_CPY_WRT_CTOR(&parameter, &wildcard);
				} else {
					PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid bind parameter");
					return;
				}
			}

			if (is_array) {
				if (likely(phalcon_array_isset_fetch(&type, data_types, &wildcard, 0))) {
					/**
					 * The bind type is double so we try to get the double value
					 */
					if (phalcon_compare_strict_long(&type, 32)) {
						phalcon_cast(&cast_value, value, IS_DOUBLE);
						ZVAL_MAKE_REF(&cast_value);
						PHALCON_CALL_METHOD(NULL, statement, "bindvalue", &parameter, &cast_value);
						ZVAL_UNREF(&cast_value);
					} else {
						/**
						 * 1024 is ignore the bind type
						 */
						ZVAL_MAKE_REF(value);
						if (phalcon_compare_strict_long(&type, 1024)) {
							PHALCON_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value);
						} else {
							PHALCON_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value, &type);
						}
						ZVAL_UNREF(value);
					}

				} else {
					if (Z_TYPE_P(value) == IS_LONG) {
						ZVAL_LONG(&type, PHALCON_DB_COLUMN_BIND_PARAM_INT);
					} else {
						ZVAL_LONG(&type, PHALCON_DB_COLUMN_BIND_PARAM_STR);
					}
					ZVAL_MAKE_REF(value);
					PHALCON_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value, &type);
					ZVAL_UNREF(value);
				}
			} else {
				ZVAL_MAKE_REF(value);
				PHALCON_CALL_METHOD(NULL, statement, "bindvalue", &parameter, value);
				ZVAL_UNREF(value);
			}
		} ZEND_HASH_FOREACH_END();
	}

	phalcon_read_property(&profiler, getThis(), SL("_profiler"), PH_NOISY);

	if (Z_TYPE(profiler) == IS_OBJECT) {
		phalcon_read_property(&sql_statement, getThis(), SL("_sqlStatement"), PH_NOISY);
		PHALCON_CALL_METHOD(NULL, &profiler, "startprofile", &sql_statement, placeholders, data_types);

		PHALCON_CALL_METHOD(NULL, statement, "execute");

		PHALCON_CALL_METHOD(NULL, &profiler, "stopprofile");
	} else {
		PHALCON_CALL_METHOD(NULL, statement, "execute");
	}

	RETURN_CTOR(statement);
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
 * @param  string $sqlStatement
 * @param  array $bindParams
 * @param  array $bindTypes
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, query){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL, debug_message = {}, events_manager = {}, event_name = {}, status = {};
	zval statement = {}, new_statement = {};

	phalcon_fetch_params(0, 1, 2, &sql_statement, &bind_params, &bind_types);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "SQL STATEMENT: ", sql_statement);
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

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);

	/**
	 * Execute the beforeQuery event if a EventsManager is available
	 */
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		phalcon_update_property_zval(getThis(), SL("_sqlStatement"), sql_statement);
		phalcon_update_property_zval(getThis(), SL("_sqlVariables"), bind_params);
		phalcon_update_property_zval(getThis(), SL("_sqlBindTypes"), bind_types);

		PHALCON_STR(&event_name, "db:beforeQuery");
		PHALCON_CALL_METHOD(&status, &events_manager, "fire", &event_name, getThis(), bind_params);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	PHALCON_CALL_METHOD(&statement, getThis(), "prepare", sql_statement);
	PHALCON_CALL_METHOD(&new_statement, getThis(), "executeprepared", &statement, bind_params, bind_types);
	PHALCON_CPY_WRT_CTOR(&statement, &new_statement);

	/**
	 * Execute the afterQuery event if a EventsManager is available
	 */
	if (likely(Z_TYPE(statement) == IS_OBJECT)) {
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_STR(&event_name, "db:afterQuery");
			PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), bind_params);
		}

		object_init_ex(return_value, phalcon_db_result_mongo_ce);
		PHALCON_CALL_METHOD(NULL, return_value, "__construct", getThis(), &statement, sql_statement, bind_params, bind_types);

		return;
	}

	RETURN_CTOR(&statement);
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

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL, debug_message = {}, events_manager = {}, event_name = {}, status = {}, affected_rows = {};
	zval mongo = {}, profiler = {}, statement = {}, new_statement = {};

	phalcon_fetch_params(0, 1, 2, &sql_statement, &bind_params, &bind_types);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "SQL STATEMENT: ", sql_statement);
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

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	/**
	 * Execute the beforeQuery event if a EventsManager is available
	 */
	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);
	if (Z_TYPE(events_manager) == IS_OBJECT) {
		phalcon_update_property_zval(getThis(), SL("_sqlStatement"), sql_statement);
		phalcon_update_property_zval(getThis(), SL("_sqlVariables"), bind_params);
		phalcon_update_property_zval(getThis(), SL("_sqlBindTypes"), bind_types);

		PHALCON_STR(&event_name, "db:beforeExecute");
		PHALCON_CALL_METHOD(&status, &events_manager, "fire", &event_name, getThis(), bind_params);
		if (PHALCON_IS_FALSE(&status)) {
			RETURN_FALSE;
		}
	}

	if (Z_TYPE_P(bind_params) == IS_ARRAY) {
		PHALCON_CALL_METHOD(&statement, getThis(), "prepare", sql_statement);
		if (Z_TYPE(statement) == IS_OBJECT) {
			PHALCON_CALL_METHOD(&new_statement, getThis(), "executeprepared", &statement, bind_params, bind_types);
			PHALCON_CALL_METHOD(&affected_rows, &new_statement, "rowcount");
		} else {
			ZVAL_LONG(&affected_rows, 0);
		}
	} else {
		phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
		phalcon_read_property(&profiler, getThis(), SL("_profiler"), PH_NOISY);
		if (Z_TYPE(profiler) == IS_OBJECT) {
			PHALCON_CALL_METHOD(NULL, &profiler, "startprofile", sql_statement);
			PHALCON_CALL_METHOD(&affected_rows, &mongo, "exec", sql_statement);
			PHALCON_CALL_METHOD(NULL, &profiler, "stopprofile");
		} else {
			PHALCON_CALL_METHOD(&affected_rows, &mongo, "exec", sql_statement);
		}
	}

	/**
	 * Execute the afterQuery event if a EventsManager is available
	 */
	if (Z_TYPE(affected_rows) == IS_LONG) {
		phalcon_update_property_zval(getThis(), SL("_affectedRows"), &affected_rows);
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_STR(&event_name, "db:afterExecute");
			PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), bind_params);
		}
	}

	RETURN_TRUE;
}

/**
 * Returns the number of affected rows by the lastest INSERT/UPDATE/DELETE executed in the database system
 *
 *<code>
 *	$connection->execute("DELETE FROM robots");
 *	echo $connection->affectedRows(), ' were deleted';
 *</code>
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, affectedRows){


	RETURN_MEMBER(getThis(), "_affectedRows");
}

/**
 * Closes the active connection returning success. Phalcon automatically closes and destroys
 * active connections when the request ends
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, close){

	zval mongo = {};

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (likely(Z_TYPE(mongo) == IS_OBJECT)) {
		phalcon_update_property_zval(getThis(), SL("_mongo"), &PHALCON_GLOBAL(z_null));
		RETURN_TRUE;
	}

	RETURN_FALSE;
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
 * Starts a transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, begin){

	zval *nesting = NULL, mongo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (Z_TYPE(mongo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Increase the transaction nesting level
	 */
	phalcon_property_incr(getThis(), SL("_transactionLevel"));

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY);
	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB BEGIN TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
	}

	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		/**
		 * Notify the events manager about the started transaction
		 */
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_STR(&event_name, "db:beginTransaction");
			PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis());
		}

		PHALCON_RETURN_CALL_METHOD(&mongo, "begintransaction");
		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the created savepoint
				 */
				if (Z_TYPE(events_manager) == IS_OBJECT) {
					PHALCON_STR(&event_name, "db:createSavepoint");
					PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &savepoint_name);
				}

				PHALCON_RETURN_CALL_METHOD(getThis(), "createsavepoint", &savepoint_name);
				return;
			}
		}
	}

	RETURN_FALSE;
}

/**
 * Rollbacks the active transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, rollback){

	zval *nesting = NULL, mongo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (Z_TYPE(mongo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY);
	if (!zend_is_true(&transaction_level)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "There is no active transaction");
		return;
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB ROLLBACK TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
	}

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);

	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		/**
		 * Notify the events manager about the rollbacked transaction
		 */
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_STR(&event_name, "db:rollbackTransaction");
			PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis());
		}

		/**
		 * Reduce the transaction nesting level
		 */
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
		PHALCON_RETURN_CALL_METHOD(&mongo, "rollback");
		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the rollbacked savepoint
				 */
				if (Z_TYPE(events_manager) == IS_OBJECT) {
					PHALCON_STR(&event_name, "db:rollbackSavepoint");
					PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &savepoint_name);
				}

				/**
				 * Reduce the transaction nesting level
				 */
				phalcon_property_decr(getThis(), SL("_transactionLevel"));
				PHALCON_RETURN_CALL_METHOD(getThis(), "rollbacksavepoint", &savepoint_name);
				return;
			}
		}
	}

	/**
	 * Reduce the transaction nesting level
	 */
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
	}

	RETURN_FALSE;
}

/**
 * Commits the active transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, commit){

	zval *nesting = NULL, mongo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (Z_TYPE(mongo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	/**
	 * Check the transaction nesting level
	 */
	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY);
	if (!zend_is_true(&transaction_level)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "There is no active transaction");
		return;
	}

	if (unlikely(PHALCON_GLOBAL(debug).enable_debug)) {
		PHALCON_CONCAT_SV(&debug_message, "DB COMMIT TRANSACTION LEVEL: ", &transaction_level);
		PHALCON_DEBUG_LOG(&debug_message);
	}

	phalcon_read_property(&events_manager, getThis(), SL("_eventsManager"), PH_NOISY);
	if (PHALCON_IS_LONG(&transaction_level, 1)) {
		/**
		 * Notify the events manager about the commited transaction
		 */
		if (Z_TYPE(events_manager) == IS_OBJECT) {
			PHALCON_STR(&event_name, "db:commitTransaction");
			PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis());
		}

		/**
		 * Reduce the transaction nesting level
		 */
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
		PHALCON_RETURN_CALL_METHOD(&mongo, "commit");
		return;
	}

	if (zend_is_true(&transaction_level)) {
		if (zend_is_true(nesting)) {

			/**
			 * Check if the current database system supports nested transactions
			 */
			PHALCON_CALL_METHOD(&ntw_savepoint, getThis(), "isnestedtransactionswithsavepoints");
			if (zend_is_true(&ntw_savepoint)) {
				PHALCON_CALL_METHOD(&savepoint_name, getThis(), "getnestedtransactionsavepointname");

				/**
				 * Notify the events manager about the commited savepoint
				 */
				if (Z_TYPE(events_manager) == IS_OBJECT) {
					PHALCON_STR(&event_name, "db:releaseSavepoint");
					PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), &savepoint_name);
				}

				/**
				 * Reduce the transaction nesting level
				 */
				phalcon_property_decr(getThis(), SL("_transactionLevel"));
				PHALCON_RETURN_CALL_METHOD(getThis(), "releasesavepoint", &savepoint_name);
				return;
			}
		}
	}

	/**
	 * Reduce the transaction nesting level
	 */
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		phalcon_property_decr(getThis(), SL("_transactionLevel"));
	}

	RETURN_FALSE;
}

/**
 * Returns the current transaction nesting level
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, getTransactionLevel){


	RETURN_MEMBER(getThis(), "_transactionLevel");
}

/**
 * Checks whether the connection is under a transaction
 *
 *<code>
 *	$connection->begin();
 *	var_dump($connection->isUnderTransaction()); //true
 *</code>
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Mongo, isUnderTransaction){

	zval mongo = {};

	phalcon_read_property(&mongo, getThis(), SL("_mongo"), PH_NOISY);
	if (likely(Z_TYPE(mongo) == IS_OBJECT)) {
		PHALCON_RETURN_CALL_METHOD(&mongo, "intransaction");
		return;
	}

	RETURN_FALSE;
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
