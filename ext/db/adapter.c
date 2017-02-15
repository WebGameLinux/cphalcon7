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

#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/dialectinterface.h"
#include "db/exception.h"
#include "db/rawvalue.h"
#include "di/injectable.h"

#include "kernel/main.h"
#include "kernel/memory.h"
#include "kernel/operators.h"
#include "kernel/object.h"
#include "kernel/array.h"
#include "kernel/concat.h"
#include "kernel/fcall.h"
#include "kernel/exception.h"
#include "kernel/file.h"
#include "kernel/hash.h"
#include "kernel/string.h"

/**
 * Phalcon\Db\Adapter
 *
 * Base class for Phalcon\Db adapters
 */
zend_class_entry *phalcon_db_adapter_ce;

PHP_METHOD(Phalcon_Db_Adapter, __construct);
PHP_METHOD(Phalcon_Db_Adapter, setProfiler);
PHP_METHOD(Phalcon_Db_Adapter, getProfiler);
PHP_METHOD(Phalcon_Db_Adapter, setDialect);
PHP_METHOD(Phalcon_Db_Adapter, getDialect);
PHP_METHOD(Phalcon_Db_Adapter, getType);
PHP_METHOD(Phalcon_Db_Adapter, getDialectType);
PHP_METHOD(Phalcon_Db_Adapter, insertAsDict);
PHP_METHOD(Phalcon_Db_Adapter, getDefaultIdValue);
PHP_METHOD(Phalcon_Db_Adapter, supportSequences);
PHP_METHOD(Phalcon_Db_Adapter, useExplicitIdValue);
PHP_METHOD(Phalcon_Db_Adapter, getDescriptor);
PHP_METHOD(Phalcon_Db_Adapter, getConnectionId);
PHP_METHOD(Phalcon_Db_Adapter, affectedRows);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_setprofiler, 0, 0, 1)
	ZEND_ARG_INFO(0, profiler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_setdialect, 0, 0, 1)
	ZEND_ARG_INFO(0, dialect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_insertasdict, 0, 0, 2)
	ZEND_ARG_INFO(0, table)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, dataTypes)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter, __construct, NULL, ZEND_ACC_PROTECTED|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter, setProfiler, arginfo_phalcon_db_adapter_setprofiler, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getProfiler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, setDialect, arginfo_phalcon_db_adapter_setdialect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDialect, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDialectType, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, insertAsDict, arginfo_phalcon_db_adapter_insertasdict, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDefaultIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, supportSequences, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, useExplicitIdValue, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getDescriptor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, getConnectionId, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter, affectedRows, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db, Adapter, db_adapter, phalcon_di_injectable_ce, phalcon_db_adapter_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);


	zend_declare_property_null(phalcon_db_adapter_ce, SL("_profiler"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_descriptor"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_dialectType"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_type"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_dialect"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_connectionId"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_transactionLevel"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_transactionsWithSavepoints"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_ce, SL("_connectionConsecutive"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_db_adapter_ce, SL("_affectedRows"), ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_adapter_ce, 1, phalcon_db_adapterinterface_ce);

	return SUCCESS;
}

/**
 * Phalcon\Db\Adapter constructor
 *
 * @param array $descriptor
 */
PHP_METHOD(Phalcon_Db_Adapter, __construct){

	zval *descriptor, *connection_consecutive, next_consecutive = {}, dialect_class = {}, dialect_object = {};
	zend_class_entry *ce0;

	phalcon_fetch_params(0, 1, 0, &descriptor);

	/**
	 * Every new connection created obtain a consecutive number from the static
	 * property self::$_connectionConsecutive
	 */
	connection_consecutive = phalcon_read_static_property_ce(phalcon_db_adapter_ce, SL("_connectionConsecutive"));

	phalcon_add_function(&next_consecutive, connection_consecutive, &PHALCON_GLOBAL(z_one));

	phalcon_update_static_property_ce(phalcon_db_adapter_ce, SL("_connectionConsecutive"), &next_consecutive);
	phalcon_update_property_zval(getThis(), SL("_connectionId"), connection_consecutive);

	/**
	 * Dialect class can override the default dialect
	 */
	if (phalcon_array_isset_fetch_str(&dialect_class, descriptor, SL("dialectClass"))) {
		/**
		 * Create the instance only if the dialect is a string
		 */
		if (likely(Z_TYPE(dialect_class) == IS_STRING)) {
			ce0 = phalcon_fetch_class(&dialect_class, ZEND_FETCH_CLASS_DEFAULT);
			PHALCON_OBJECT_INIT(&dialect_object, ce0);
			if (phalcon_has_constructor(&dialect_object)) {
				PHALCON_CALL_METHOD(NULL, &dialect_object, "__construct");
			}
			PHALCON_CALL_SELF(NULL, "setdialect", &dialect_object);
		} else if (Z_TYPE(dialect_class) == IS_OBJECT) {
			PHALCON_CALL_SELF(NULL, "setdialect", &dialect_class);
		}
	}

	phalcon_update_property_zval(getThis(), SL("_descriptor"), descriptor);
}

/**
 * Sets the profiler
 *
 * @param Phalcon\Db\Profiler $profiler
 */
PHP_METHOD(Phalcon_Db_Adapter, setProfiler){

	zval *profiler;

	phalcon_fetch_params(0, 1, 0, &profiler);

	phalcon_update_property_zval(getThis(), SL("_profiler"), profiler);

}

/**
 * Returns the profiler
 *
 * @return Phalcon\Db\Profiler
 */
PHP_METHOD(Phalcon_Db_Adapter, getProfiler){


	RETURN_MEMBER(getThis(), "_profiler");
}

/**
 * Sets the dialect used to produce the SQL
 *
 * @param Phalcon\Db\DialectInterface
 */
PHP_METHOD(Phalcon_Db_Adapter, setDialect){

	zval *dialect;

	phalcon_fetch_params(0, 1, 0, &dialect);

	PHALCON_VERIFY_INTERFACE_EX(dialect, phalcon_db_dialectinterface_ce, phalcon_db_exception_ce);
	phalcon_update_property_zval(getThis(), SL("_dialect"), dialect);
}

/**
 * Returns internal dialect instance
 *
 * @return Phalcon\Db\DialectInterface
 */
PHP_METHOD(Phalcon_Db_Adapter, getDialect){

	zval service_name = {}, has = {}, dialect_type = {}, dialect_class = {};
	zend_class_entry *ce0;

	phalcon_read_property(return_value, getThis(), SL("_dialect"), PH_NOISY);
	if (Z_TYPE_P(return_value) == IS_OBJECT) {
		return;
	}

	ZVAL_STRING(&service_name, "dialect");

	PHALCON_CALL_METHOD(&has, getThis(), "hasservice", &service_name);
	if (zend_is_true(&has)) {
		PHALCON_CALL_METHOD(return_value, getThis(), "getresolveservice", &service_name);
		if (Z_TYPE_P(return_value) != IS_OBJECT) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The injected service 'modelsMetadata' is not object (1)");
			return;
		}
		PHALCON_VERIFY_INTERFACE_EX(return_value, phalcon_db_dialectinterface_ce, phalcon_db_exception_ce);
		return;
	}

	phalcon_read_property(&dialect_type, getThis(), SL("_dialectType"), PH_NOISY);

	PHALCON_CONCAT_SV(&dialect_class, "phalcon\\db\\dialect\\", &dialect_type);

	ce0 = phalcon_fetch_class(&dialect_class, ZEND_FETCH_CLASS_DEFAULT);
	PHALCON_OBJECT_INIT(return_value, ce0);
	if (phalcon_has_constructor(return_value)) {
		PHALCON_CALL_METHOD(NULL, return_value, "__construct");
	}
	PHALCON_CALL_SELF(NULL, "setdialect", return_value);
}

/**
 * Returns type of database system the adapter is used for
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getType){


	RETURN_MEMBER(getThis(), "_type");
}

/**
 * Returns the name of the dialect used
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getDialectType){


	RETURN_MEMBER(getThis(), "_dialectType");
}

/**
 * Inserts data into a table using custom RBDM SQL syntax
 * Another, more convenient syntax
 *
 * <code>
 * //Inserting a new robot
 * $success = $connection->insertAsDict(
 *	 "robots",
 *	 array(
 *		  "name" => "Astro Boy",
 *		  "year" => 1952
 *	  )
 * );
 *
 * //Next SQL sentence is sent to the database system
 * INSERT INTO `robots` (`name`, `year`) VALUES ("Astro boy", 1952);
 * </code>
 *
 * @param string table
 * @param array data
 * @param array dataTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, insertAsDict){

	zval *table, *data, *data_types = NULL, fields = {}, values = {}, *value;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 1, &table, &data, &data_types);

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(data) != IS_ARRAY || PHALCON_IS_EMPTY(data)) {
		RETURN_FALSE;
	}

	array_init(&fields);
	array_init(&values);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, str_key, value) {
		zval field = {};
		if (str_key) {
			ZVAL_STR(&field, str_key);
		} else {
			ZVAL_LONG(&field, idx);
		}
		phalcon_array_append(&fields, &field, PH_COPY);
		phalcon_array_append(&values, value, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	PHALCON_RETURN_CALL_METHOD(getThis(), "insert", table, &values, &fields, data_types);
}

/**
 * Returns the default identity value to be inserted in an identity column
 *
 * @return Phalcon\Db\RawValue
 */
PHP_METHOD(Phalcon_Db_Adapter, getDefaultIdValue){

	zval default_value = {};

	ZVAL_STRING(&default_value, "null");
	object_init_ex(return_value, phalcon_db_rawvalue_ce);
	PHALCON_CALL_METHOD(NULL, return_value, "__construct", &default_value);

	return;
}

/**
 * Check whether the database system requires a sequence to produce auto-numeric values
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, supportSequences){


	RETURN_FALSE;
}

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter, useExplicitIdValue){


	RETURN_FALSE;
}

/**
 * Return descriptor used to connect to the active database
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter, getDescriptor){


	RETURN_MEMBER(getThis(), "_descriptor");
}

/**
 * Gets the active connection unique identifier
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter, getConnectionId){


	RETURN_MEMBER(getThis(), "_connectionId");
}


/**
 * Returns the number of affected rows by the lastest INSERT/UPDATE/DELETE executed in the database system
 *
 * @return int
 */
PHP_METHOD(Phalcon_Db_Adapter, affectedRows){


	RETURN_MEMBER(getThis(), "_affectedRows");
}
