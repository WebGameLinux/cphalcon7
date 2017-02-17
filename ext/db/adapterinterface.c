
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

#include "db/adapterinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_db_adapterinterface_ce;

static const zend_function_entry phalcon_db_adapterinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, insert, arginfo_phalcon_db_adapterinterface_insert)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, update, arginfo_phalcon_db_adapterinterface_update)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, delete, arginfo_phalcon_db_adapterinterface_delete)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getDescriptor, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getType, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getDialectType, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getDialect, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, connect, arginfo_phalcon_db_adapterinterface_connect)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, query, arginfo_phalcon_db_adapterinterface_query)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, execute, arginfo_phalcon_db_adapterinterface_execute)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, affectedRows, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, close, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, lastInsertId, arginfo_phalcon_db_adapterinterface_lastinsertid)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getInternalHandler, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, useExplicitIdValue, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, getDefaultIdValue, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, supportSequences, NULL)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, escapeString, arginfo_phalcon_db_adapterinterface_escapestring)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, escapeBytea, arginfo_phalcon_db_adapterinterface_escapebytea)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, unescapeBytea, arginfo_phalcon_db_adapterinterface_unescapebytea)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, escapeArray, arginfo_phalcon_db_adapterinterface_escapearray)
	PHP_ABSTRACT_ME(Phalcon_Db_AdapterInterface, unescapeArray, arginfo_phalcon_db_adapterinterface_unescapearray)
	PHP_FE_END
};


/**
 * Phalcon\Db\AdapterInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_AdapterInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Db, AdapterInterface, db_adapterinterface, phalcon_db_adapterinterface_method_entry);

	return SUCCESS;
}

/**
 * Inserts data into a table using custom RBDM SQL syntax
 *
 * @param 	string $table
 * @param 	array $values
 * @param 	array $fields
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, insert);

/**
 * Updates data on a table using custom RBDM SQL syntax
 *
 * @param 	string $table
 * @param 	array $fields
 * @param 	array $values
 * @param 	string $whereCondition
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, update);

/**
 * Deletes data from a table using custom RBDM SQL syntax
 *
 * @param  string $table
 * @param  string $whereCondition
 * @param  array $placeholders
 * @param  array $dataTypes
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, delete);

/**
 * Appends a LIMIT clause to $sqlQuery argument
 *
 * @param  	string $sqlQuery
 * @param 	int $number
 * @return 	string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, limit);

/**
 * Return descriptor used to connect to the active database
 *
 * @return array
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getDescriptor);

/**
 * Gets the active connection unique identifier
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getConnectionId);

/**
 * Returns type of database system the adapter is used for
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getType);

/**
 * Returns the name of the dialect used
 *
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getDialectType);

/**
 * Returns internal dialect instance
 *
 * @return Phalcon\Db\DialectInterface
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getDialect);

/**
 * This method is automatically called in Phalcon\Db\Adapter\Pdo constructor.
 * Call it when you need to restore a database connection
 *
 * @param array $descriptor
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, connect);

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server return rows
 *
 * @param string $sqlStatement
 * @param array $placeholders
 * @param array $dataTypes
 * @return Phalcon\Db\ResultInterface
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, query);

/**
 * Sends SQL statements to the database server returning the success state.
 * Use this method only when the SQL statement sent to the server don't return any row
 *
 * @param string $sqlStatement
 * @param array $placeholders
 * @param array $dataTypes
 * @param boolean $flag
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, execute);

/**
 * Returns the number of affected rows by the last INSERT/UPDATE/DELETE reported by the database system
 *
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, affectedRows);

/**
 * Closes active connection returning success. Phalcon automatically closes and destroys active connections within Phalcon\Db\Pool
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, close);

/**
 * Returns insert id for the auto_increment column inserted in the last SQL statement
 *
 * @param string $sequenceName
 * @return int
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, lastInsertId);

/**
 * Return internal PDO handler
 *
 * @return \PDO
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getInternalHandler);

/**
 * Check whether the database system requires an explicit value for identity columns
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, useExplicitIdValue);

/**
 * Return the default identity value to insert in an identity column
 *
 * @return Phalcon\Db\RawValue
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, getDefaultIdValue);

/**
 * Check whether the database system requires a sequence to produce auto-numeric values
 *
 * @return boolean
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, supportSequences);

/**
 * Escapes a column/table/schema name
 *
 * @param string $identifier
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, escapeIdentifier);

/**
 * Escapes a value to avoid SQL injections
 *
 * @param string $str
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_AdapterInterface, escapeString);

/**
 * Convert php bytea to database bytea
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_AdapterInterface, escapeBytea);

/**
 * Convert database bytea to php bytea
 *
 * @param string $value
 * @return string
 */
PHP_METHOD(Phalcon_Db_AdapterInterface, unescapeBytea);

/**
 * Convert php array to database array
 *
 * @param array $value
 * @param int $type
 * @return string
 */
PHP_METHOD(Phalcon_Db_AdapterInterface, escapeArray);

/**
 * Convert database array to php array
 *
 * @param string $value
 * @param int $type
 * @return array
 */
PHP_METHOD(Phalcon_Db_AdapterInterface, unescapeArray);
