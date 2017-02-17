
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

#include "db/adapter/pdo.h"
#include "db/adapter.h"
#include "db/adapterinterface.h"
#include "db/exception.h"
#include "db/result/pdo.h"
#include "db/column.h"
#include "db/index.h"
#include "db/reference.h"
#include "db/rawvalue.h"
#include "debug.h"

#include <ext/pdo/php_pdo_driver.h>

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
 * Phalcon\Db\Adapter\Pdo
 *
 * Phalcon\Db\Adapter\Pdo is the Phalcon\Db that internally uses PDO to connect to a database
 *
 *<code>
 *	$connection = new Phalcon\Db\Adapter\Pdo\Mysql(array(
 *		'host' => '192.168.0.11',
 *		'username' => 'sigma',
 *		'password' => 'secret',
 *		'dbname' => 'blog',
 *		'port' => '3306'
 *	));
 *</code>
 */
zend_class_entry *phalcon_db_adapter_pdo_ce;

PHP_METHOD(Phalcon_Db_Adapter_Pdo, __construct);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, connect);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, prepare);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, executePrepared);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, query);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, execute);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, fetchOne);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, fetchAll);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, insert);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, update);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, delete);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getColumnList);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, limit);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, tableExists);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, viewExists);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, forUpdate);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, sharedLock);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createTable);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropTable);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createView);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropView);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addColumn);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, modifyColumn);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropColumn);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addIndex);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropIndex);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addForeignKey);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropForeignKey);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, listTables);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, listViews);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, describeIndexes);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, describeReferences);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, tableOptions);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createSavepoint);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, releaseSavepoint);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollbackSavepoint);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, setNestedTransactionsWithSavepoints);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isNestedTransactionsWithSavepoints);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getNestedTransactionSavepointName);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLStatement);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getRealSQLStatement);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLVariables);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLBindTypes);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, close);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeIdentifier);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeString);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, convertBoundParams);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, lastInsertId);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, begin);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollback);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, commit);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getTransactionLevel);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isUnderTransaction);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getInternalHandler);
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getErrorInfo);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo___construct, 0, 0, 1)
	ZEND_ARG_INFO(0, descriptor)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlStatement)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_executeprepared, 0, 0, 1)
	ZEND_ARG_INFO(0, statement)
	ZEND_ARG_INFO(0, placeholders)
	ZEND_ARG_INFO(0, dataTypes)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_fetchone, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sqlQuery, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, fetchMode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, placeholders, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, dataTypes, IS_ARRAY, 1)
	ZEND_ARG_INFO(0, fetchArgument)
	ZEND_ARG_TYPE_INFO(0, ctorArgs, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_fetchall, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, sqlQuery, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, fetchMode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, placeholders, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, dataTypes, IS_ARRAY, 1)
	ZEND_ARG_INFO(0, fetchArgument)
	ZEND_ARG_TYPE_INFO(0, ctorArgs, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_getcolumnlist, 0, 0, 1)
	ZEND_ARG_INFO(0, columnList)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_limit, 0, 0, 2)
	ZEND_ARG_INFO(0, sqlQuery)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_tableexists, 0, 0, 1)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_viewexists, 0, 0, 1)
	ZEND_ARG_INFO(0, viewName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_forupdate, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlQuery)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_sharedlock, 0, 0, 1)
	ZEND_ARG_INFO(0, sqlQuery)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_createtable, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_droptable, 0, 0, 1)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
	ZEND_ARG_INFO(0, ifExists)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_createview, 0, 0, 2)
	ZEND_ARG_INFO(0, viewName)
	ZEND_ARG_INFO(0, definition)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_dropview, 0, 0, 1)
	ZEND_ARG_INFO(0, viewName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
	ZEND_ARG_INFO(0, ifExists)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_addcolumn, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_modifycolumn, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_dropcolumn, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, columnName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_addindex, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_dropindex, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, indexName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_addprimarykey, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_dropprimarykey, 0, 0, 2)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_addforeignkey, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, reference)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_dropforeignkey, 0, 0, 3)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 0)
	ZEND_ARG_INFO(0, referenceName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_getcolumndefinition, 0, 0, 1)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_listtables, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_listviews, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, schemaName, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_convertboundparams, 0, 0, 2)
	ZEND_ARG_INFO(0, sqlStatement)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_tableoptions, 0, 0, 1)
	ZEND_ARG_INFO(0, tableName)
	ZEND_ARG_INFO(0, schemaName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_createsavepoint, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_releasesavepoint, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_rollbacksavepoint, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_setnestedtransactionswithsavepoints, 0, 0, 1)
	ZEND_ARG_INFO(0, nestedTransactionsWithSavepoints)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_begin, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_rollback, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_adapter_pdo_commit, 0, 0, 0)
	ZEND_ARG_INFO(0, nesting)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_adapter_pdo_method_entry[] = {
	PHP_ME(Phalcon_Db_Adapter_Pdo, __construct, arginfo_phalcon_db_adapter_pdo___construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Db_Adapter_Pdo, connect, arginfo_phalcon_db_adapterinterface_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, prepare, arginfo_phalcon_db_adapter_pdo_prepare, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, executePrepared, arginfo_phalcon_db_adapter_pdo_executeprepared, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, query, arginfo_phalcon_db_adapterinterface_query, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, execute, arginfo_phalcon_db_adapterinterface_execute, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, fetchOne, arginfo_phalcon_db_adapter_pdo_fetchone, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, fetchAll, arginfo_phalcon_db_adapter_pdo_fetchall, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, insert, arginfo_phalcon_db_adapterinterface_insert, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, update, arginfo_phalcon_db_adapterinterface_update, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, delete, arginfo_phalcon_db_adapterinterface_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getColumnList, arginfo_phalcon_db_adapter_pdo_getcolumnlist, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, limit, arginfo_phalcon_db_adapter_pdo_limit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, tableExists, arginfo_phalcon_db_adapter_pdo_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, viewExists, arginfo_phalcon_db_adapter_pdo_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, forUpdate, arginfo_phalcon_db_adapter_pdo_forupdate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, sharedLock, arginfo_phalcon_db_adapter_pdo_sharedlock, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, createTable, arginfo_phalcon_db_adapter_pdo_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropTable, arginfo_phalcon_db_adapter_pdo_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, createView, arginfo_phalcon_db_adapter_pdo_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropView, arginfo_phalcon_db_adapter_pdo_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, addColumn, arginfo_phalcon_db_adapter_pdo_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, modifyColumn, arginfo_phalcon_db_adapter_pdo_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropColumn, arginfo_phalcon_db_adapter_pdo_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, addIndex, arginfo_phalcon_db_adapter_pdo_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropIndex, arginfo_phalcon_db_adapter_pdo_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, addPrimaryKey, arginfo_phalcon_db_adapter_pdo_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropPrimaryKey, arginfo_phalcon_db_adapter_pdo_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, addForeignKey, arginfo_phalcon_db_adapter_pdo_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, dropForeignKey, arginfo_phalcon_db_adapter_pdo_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getColumnDefinition, arginfo_phalcon_db_adapter_pdo_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, listTables, arginfo_phalcon_db_adapter_pdo_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, listViews, arginfo_phalcon_db_adapter_pdo_listviews, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, describeIndexes, arginfo_phalcon_db_adapter_pdo_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, describeReferences, arginfo_phalcon_db_adapter_pdo_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, tableOptions, arginfo_phalcon_db_adapter_pdo_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, createSavepoint, arginfo_phalcon_db_adapter_pdo_createsavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, releaseSavepoint, arginfo_phalcon_db_adapter_pdo_releasesavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, rollbackSavepoint, arginfo_phalcon_db_adapter_pdo_rollbacksavepoint, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, setNestedTransactionsWithSavepoints, arginfo_phalcon_db_adapter_pdo_setnestedtransactionswithsavepoints, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, isNestedTransactionsWithSavepoints, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getNestedTransactionSavepointName, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getSQLStatement, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getRealSQLStatement, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getSQLVariables, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getSQLBindTypes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, close, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, escapeIdentifier, arginfo_phalcon_db_adapterinterface_escapeidentifier, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, escapeString, arginfo_phalcon_db_adapterinterface_escapestring, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, convertBoundParams, arginfo_phalcon_db_adapter_pdo_convertboundparams, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, lastInsertId, arginfo_phalcon_db_adapterinterface_lastinsertid, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, begin, arginfo_phalcon_db_adapter_pdo_begin, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, rollback, arginfo_phalcon_db_adapter_pdo_rollback, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, commit, arginfo_phalcon_db_adapter_pdo_commit, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getTransactionLevel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, isUnderTransaction, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getInternalHandler, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Adapter_Pdo, getErrorInfo, NULL, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Adapter\Pdo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Adapter_Pdo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Adapter, Pdo, db_adapter_pdo, phalcon_db_adapter_ce, phalcon_db_adapter_pdo_method_entry, ZEND_ACC_EXPLICIT_ABSTRACT_CLASS);

	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_pdo"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_affectedRows"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_schema"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_connectionId"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(phalcon_db_adapter_pdo_ce, SL("_connectionConsecutive"), 0, ZEND_ACC_PROTECTED|ZEND_ACC_STATIC);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_sqlStatement"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_sqlVariables"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(phalcon_db_adapter_pdo_ce, SL("_sqlBindTypes"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}

/**
 * Constructor for Phalcon\Db\Adapter\Pdo
 *
 * @param array $descriptor
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, __construct){

	zval *descriptor;

	phalcon_fetch_params(0, 1, 0, &descriptor);

	if (Z_TYPE_P(descriptor) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The descriptor must be an array");
		return;
	}

	PHALCON_CALL_METHOD(NULL, getThis(), "connect", descriptor);
	PHALCON_CALL_PARENT(NULL, phalcon_db_adapter_pdo_ce, getThis(), "__construct", descriptor);
}

/**
 * This method is automatically called in Phalcon\Db\Adapter\Pdo constructor.
 * Call it when you need to restore a database connection
 *
 *<code>
 * //Make a connection
 * $connection = new Phalcon\Db\Adapter\Pdo\Mysql(array(
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, connect)
{
	zval *_descriptor = NULL, descriptor = {}, username = {}, password = {}, options = {}, dsn_parts = {}, *value, dsn_attributes = {}, pdo_type = {}, dsn = {}, persistent = {}, pdo = {};
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

	phalcon_read_property(&pdo_type, getThis(), SL("_type"), PH_NOISY);

	PHALCON_CONCAT_VSV(&dsn, &pdo_type, ":", &dsn_attributes);

	/**
	 * Default options
	 */
	phalcon_array_update_long_long(&options, PDO_ATTR_ERRMODE, PDO_ERRMODE_EXCEPTION, PH_COPY);

	/**
	 * Check if the connection must be persistent
	 */
	if (phalcon_array_isset_fetch_str(&persistent, &descriptor, SL("persistent"))) {
		phalcon_array_unset_str(&descriptor, SL("persistent"), PH_SEPARATE);
		if (zend_is_true(&persistent)) {
			phalcon_array_update_long_bool(&options, PDO_ATTR_PERSISTENT, 1, PH_COPY);
		}
	}

	/**
	 * Create the connection using PDO
	 */
	ce = phalcon_fetch_str_class(SL("PDO"), ZEND_FETCH_CLASS_AUTO);

	PHALCON_OBJECT_INIT(&pdo, ce);
	PHALCON_CALL_METHOD(NULL, &pdo, "__construct", &dsn, &username, &password, &options);

	phalcon_update_property_zval(getThis(), SL("_pdo"), &pdo);
}

/**
 * Returns a PDO prepared statement to be executed with 'executePrepared'
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $pdoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param string $sqlStatement
 * @return \PDOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, prepare){

	zval *sql_statement, pdo = {};

	phalcon_fetch_params(0, 1, 0, &sql_statement);

	phalcon_update_property_zval(getThis(), SL("_sqlStatement"), sql_statement);

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "prepare", sql_statement);
}

/**
 * Executes a prepared statement binding. This function uses integer indexes starting from zero
 *
 *<code>
 * $statement = $connection->prepare('SELECT * FROM robots WHERE name = :name');
 * $pdoResult = $connection->executePrepared($statement, array('name' => 'Voltron'));
 *</code>
 *
 * @param \PDOStatement $statement
 * @param array $placeholders
 * @param array $dataTypes
 * @return \PDOStatement
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, executePrepared){

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
 * @param string $sqlStatement
 * @param array $bindParams
 * @param array $bindTypes
 * @return Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, query){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL;

	phalcon_fetch_params(0, 1, 2, &sql_statement, &bind_params, &bind_types);

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", sql_statement, bind_params, bind_types, &PHALCON_GLOBAL(z_true));
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
 * @param string $sqlStatement
 * @param array $bindParams
 * @param array $bindTypes
 * @param boolean $flag
 * @return boolean|Phalcon\Db\ResultInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, execute){

	zval *sql_statement, *bind_params = NULL, *bind_types = NULL, *flag = NULL, debug_message = {};
	zval events_manager = {}, event_name = {}, status = {}, affected_rows = {};
	zval statement = {}, new_statement = {};

	phalcon_fetch_params(0, 1, 3, &sql_statement, &bind_params, &bind_types, &flag);

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

	PHALCON_CALL_METHOD(&statement, getThis(), "prepare", sql_statement);
	PHALCON_CALL_METHOD(&new_statement, getThis(), "executeprepared", &statement, bind_params, bind_types);

	if (zend_is_true(flag)) {
		/**
		 * Execute the afterQuery event if a EventsManager is available
		 */
		if (likely(Z_TYPE(new_statement) == IS_OBJECT)) {
			if (Z_TYPE(events_manager) == IS_OBJECT) {
				PHALCON_STR(&event_name, "db:afterExecute");
				PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), bind_params);
			}

			object_init_ex(return_value, phalcon_db_result_pdo_ce);
			PHALCON_CALL_METHOD(NULL, return_value, "__construct", getThis(), &new_statement, sql_statement, bind_params, bind_types);
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
		PHALCON_CALL_METHOD(NULL, &events_manager, "fire", &event_name, getThis(), bind_params);
	}

	RETURN_TRUE;
}

/**
 * Returns the first row in a SQL query result
 *
 *<code>
 *	//Getting first robot
 *	$robot = $connection->fetchOne("SELECT * FROM robots");
 *	print_r($robot);
 *
 *	//Getting first robot with associative indexes only
 *	$robot = $connection->fetchOne("SELECT * FROM robots", Phalcon\Db::FETCH_ASSOC);
 *	print_r($robot);
 *</code>
 *
 * @param string $sqlQuery
 * @param int $fetchMode
 * @param array $bindParams
 * @param array $bindTypes
 * @param mixed $fetchArgument
 * @param array $ctorArgs
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, fetchOne){

	zval *sql_query, *_fetch_mode = NULL, *bind_params = NULL, *bind_types = NULL, *fetch_argument = NULL, *ctor_args = NULL, fetch_mode = {}, result = {};

	phalcon_fetch_params(0, 1, 5, &sql_query, &_fetch_mode, &bind_params, &bind_types, &fetch_argument, &ctor_args);

	if (!_fetch_mode) {
		ZVAL_LONG(&fetch_mode, PDO_FETCH_BOTH);
	} else {
		PHALCON_CPY_WRT(&fetch_mode, _fetch_mode);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (!fetch_argument) {
		fetch_argument = &PHALCON_GLOBAL(z_null);
	}

	if (!ctor_args) {
		ctor_args = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&result, getThis(), "query", sql_query, bind_params, bind_types);
	if (Z_TYPE(result) == IS_OBJECT) {
		if (Z_TYPE(fetch_mode) != IS_NULL) {
			if (Z_TYPE_P(fetch_argument) != IS_NULL) {
				if (Z_TYPE_P(ctor_args) != IS_NULL) {
					PHALCON_RETURN_CALL_METHOD(&result, "fetch", &fetch_mode, fetch_argument, ctor_args);
				} else {
					PHALCON_RETURN_CALL_METHOD(&result, "fetch", &fetch_mode, fetch_argument);
				}
			} else {
				PHALCON_CALL_METHOD(NULL, &result, "setfetchmode", &fetch_mode);
				PHALCON_RETURN_CALL_METHOD(&result, "fetch");
			}
		} else {
			PHALCON_RETURN_CALL_METHOD(&result, "fetch");
		}
		return;
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Dumps the complete result of a query into an array
 *
 *<code>
 *	//Getting all robots with associative indexes only
 *	$robots = $connection->fetchAll("SELECT * FROM robots", Phalcon\Db::FETCH_ASSOC);
 *	foreach ($robots as $robot) {
 *		print_r($robot);
 *	}
 *
 *  //Getting all robots that contains word "robot" withing the name
 *  $robots = $connection->fetchAll("SELECT * FROM robots WHERE name LIKE :name",
 *		Phalcon\Db::FETCH_ASSOC,
 *		array('name' => '%robot%')
 *  );
 *	foreach($robots as $robot){
 *		print_r($robot);
 *	}
 *</code>
 *
 * @param string $sqlQuery
 * @param int $fetchMode
 * @param array $bindParams
 * @param array $bindTypes
 * @param mixed $fetchArgument
 * @param array $ctorArgs
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, fetchAll){

	zval *sql_query, *_fetch_mode = NULL, *bind_params = NULL, *bind_types = NULL, *fetch_argument = NULL, *ctor_args = NULL, fetch_mode = {}, result = {};

	phalcon_fetch_params(0, 1, 5, &sql_query, &_fetch_mode, &bind_params, &bind_types, &fetch_argument, &ctor_args);

	if (!_fetch_mode) {
		ZVAL_LONG(&fetch_mode, PDO_FETCH_BOTH);
	} else {
		PHALCON_CPY_WRT(&fetch_mode, _fetch_mode);
	}

	if (!bind_params) {
		bind_params = &PHALCON_GLOBAL(z_null);
	}

	if (!bind_types) {
		bind_types = &PHALCON_GLOBAL(z_null);
	}

	if (!fetch_argument) {
		fetch_argument = &PHALCON_GLOBAL(z_null);
	}

	if (!ctor_args) {
		ctor_args = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&result, getThis(), "query", sql_query, bind_params, bind_types);
	if (likely(Z_TYPE(result) == IS_OBJECT)) {
		if (Z_TYPE(fetch_mode) != IS_NULL) {
			if (Z_TYPE_P(fetch_argument) != IS_NULL) {
				if (Z_TYPE_P(ctor_args) != IS_NULL) {
					PHALCON_RETURN_CALL_METHOD(&result, "fetchall", &fetch_mode, fetch_argument, ctor_args);
				} else {
					PHALCON_RETURN_CALL_METHOD(&result, "fetchall", &fetch_mode, fetch_argument);
				}
			} else {
				PHALCON_RETURN_CALL_METHOD(&result, "fetchall", &fetch_mode);
			}
		} else {
			PHALCON_RETURN_CALL_METHOD(&result, "fetchall");
		}
	}
}

/**
 * Inserts data into a table using custom RBDM SQL syntax
 *
 * <code>
 * //Inserting a new robot
 * $success = $connection->insert(
 *     "robots",
 *     array("Astro Boy", 1952),
 *     array("name", "year")
 * );
 *
 * //Next SQL sentence is sent to the database system
 * INSERT INTO `robots` (`name`, `year`) VALUES ("Astro boy", 1952);
 * </code>
 *
 * @param 	string $table
 * @param 	array $values
 * @param 	array $fields
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, insert){

	zval *table, *values, *fields = NULL, *data_types = NULL, exception_message = {}, placeholders = {}, insert_values = {}, bind_data_types = {}, *value;
	zval escaped_table = {}, joined_values = {}, escaped_fields = {}, *field, joined_fields, insert_sql = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 2, 2, &table, &values, &fields, &data_types);

	if (!fields) {
		fields = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (unlikely(Z_TYPE_P(values) != IS_ARRAY)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The second parameter for insert isn't an Array");
		return;
	}

	/**
	 * A valid array with more than one element is required
	 */
	if (!phalcon_fast_count_ev(values)) {
		PHALCON_CONCAT_SVS(&exception_message, "Unable to insert into ", table, " without data");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	array_init(&placeholders);
	array_init(&insert_values);

	if (Z_TYPE_P(data_types) == IS_ARRAY) {
		array_init(&bind_data_types);
	} else {
		PHALCON_CPY_WRT_CTOR(&bind_data_types, data_types);
	}

	/**
	 * Objects are casted using __toString, null values are converted to string 'null',
	 * everything else is passed as '?'
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), idx, str_key, value) {
		zval position = {}, str_value = {}, bind_type = {};
		if (str_key) {
			ZVAL_STR(&position, str_key);
		} else {
			ZVAL_LONG(&position, idx);
		}
		if (Z_TYPE_P(value) == IS_OBJECT) {
			phalcon_strval(&str_value, value);
			phalcon_array_append(&placeholders, &str_value, PH_COPY);
		} else {
			if (Z_TYPE_P(value) == IS_NULL) {
				phalcon_array_append_string(&placeholders, SL("null"), PH_COPY);
			} else {
				phalcon_array_append_string(&placeholders, SL("?"), PH_COPY);
				phalcon_array_append(&insert_values, value, PH_COPY);
				if (Z_TYPE_P(data_types) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&bind_type, data_types, &position, 0)) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Incomplete number of bind types");
						return;
					}

					phalcon_array_append(&bind_data_types, &bind_type, PH_COPY);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		PHALCON_CPY_WRT_CTOR(&escaped_table, table);
	}

	/**
	 * Build the final SQL INSERT statement
	 */
	phalcon_fast_join_str(&joined_values, SL(", "), &placeholders);
	if (Z_TYPE_P(fields) == IS_ARRAY) {
		if (PHALCON_GLOBAL(db).escape_identifiers) {
			array_init(&escaped_fields);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(fields), field) {
				zval escaped_field = {};
				PHALCON_CALL_METHOD(&escaped_field, getThis(), "escapeidentifier", field);
				phalcon_array_append(&escaped_fields, &escaped_field, PH_COPY);
			} ZEND_HASH_FOREACH_END();

		} else {
			PHALCON_CPY_WRT_CTOR(&escaped_fields, fields);
		}

		phalcon_fast_join_str(&joined_fields, SL(", "), &escaped_fields);

		PHALCON_CONCAT_SVSVSVS(&insert_sql, "INSERT INTO ", &escaped_table, " (", &joined_fields, ") VALUES (", &joined_values, ")");
	} else {
		PHALCON_CONCAT_SVSVS(&insert_sql, "INSERT INTO ", &escaped_table, " VALUES (", &joined_values, ")");
	}

	/**
	 * Perform the execution via execute
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &insert_sql, &insert_values, &bind_data_types);
}

/**
 * Updates data on a table using custom RBDM SQL syntax
 *
 * <code>
 * //Updating existing robot
 * $success = $connection->update(
 *     "robots",
 *     array("name"),
 *     array("New Astro Boy"),
 *     "id = 101"
 * );
 *
 * //Next SQL sentence is sent to the database system
 * UPDATE `robots` SET `name` = "Astro boy" WHERE id = 101
 * </code>
 *
 * @param 	string $table
 * @param 	array $fields
 * @param 	array $values
 * @param 	string $whereCondition
 * @param 	array $dataTypes
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, update){

	zval *table, *fields, *values, *where_condition = NULL, *data_types = NULL, placeholders = {}, update_values = {}, bind_data_types = {}, *value;
	zval escaped_table = {}, set_clause = {}, update_sql = {}, conditions = {}, where_bind = {}, where_types = {};
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 3, 2, &table, &fields, &values, &where_condition, &data_types);

	if (!where_condition) {
		where_condition = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	array_init(&placeholders);
	array_init(&update_values);

	if (Z_TYPE_P(data_types) == IS_ARRAY) {
		array_init(&bind_data_types);
	} else {
		PHALCON_CPY_WRT_CTOR(&bind_data_types, data_types);
	}

	/**
	 * Objects are casted using __toString, null values are converted to string 'null',
	 * everything else is passed as '?'
	 */
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(values), idx, str_key, value) {
		zval position = {}, field = {}, escaped_field = {}, set_clause_part = {}, bind_type = {};
		if (str_key) {
			ZVAL_STR(&position, str_key);
		} else {
			ZVAL_LONG(&position, idx);
		}
		if (!phalcon_array_isset_fetch(&field, fields, &position, 0)) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The number of values in the update is not the same as fields");
			return;
		}

		if (PHALCON_GLOBAL(db).escape_identifiers) {
			PHALCON_CALL_METHOD(&escaped_field, getThis(), "escapeidentifier", &field);
		} else {
			PHALCON_CPY_WRT_CTOR(&escaped_field, &field);
		}

		if (Z_TYPE_P(value) == IS_OBJECT) {
			PHALCON_CONCAT_VSV(&set_clause_part, &escaped_field, " = ", value);
			phalcon_array_append(&placeholders, &set_clause_part, PH_COPY);
		} else {
			if (Z_TYPE_P(value) == IS_NULL) {
				PHALCON_CONCAT_VS(&set_clause_part, &escaped_field, " = null");
			} else {
				PHALCON_CONCAT_VS(&set_clause_part, &escaped_field, " = ?");
				phalcon_array_append(&update_values, value, PH_COPY);
				if (Z_TYPE_P(data_types) == IS_ARRAY) {
					if (!phalcon_array_isset_fetch(&bind_type, data_types, &position, 0)) {
						PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Incomplete number of bind types");
						return;
					}

					phalcon_array_append(&bind_data_types, &bind_type, PH_COPY);
				}
			}
			phalcon_array_append(&placeholders, &set_clause_part, PH_COPY);
		}
	} ZEND_HASH_FOREACH_END();

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		PHALCON_CPY_WRT_CTOR(&escaped_table, table);
	}

	phalcon_fast_join_str(&set_clause, SL(", "), &placeholders);
	if (Z_TYPE_P(where_condition) != IS_NULL) {
		PHALCON_CONCAT_SVSVS(&update_sql, "UPDATE ", &escaped_table, " SET ", &set_clause, " WHERE ");

		/**
		 * String conditions are simply appended to the SQL
		 */
		if (Z_TYPE_P(where_condition) == IS_STRING) {
			phalcon_concat_self(&update_sql, where_condition);
		} else {
			/**
			 * Array conditions may have bound params and bound types
			 */
			if (unlikely(Z_TYPE_P(where_condition) != IS_ARRAY)) {
				PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid WHERE clause conditions");
				return;
			}

			/**
			 * If an index 'conditions' is present it contains string where conditions that are
			 * appended to the UPDATE sql
			 */
			if (phalcon_array_isset_fetch_str(&conditions, where_condition, SL("conditions"))) {
				phalcon_concat_self(&update_sql, &conditions);
			}

			/**
			 * Bound parameters are arbitrary values that are passed by separate
			 */
			if (phalcon_array_isset_fetch_str(&where_bind, where_condition, SL("bind"))) {
				phalcon_merge_append(&update_values, &where_bind);
			}

			/**
			 * Bind types is how the bound parameters must be casted before be sent to the
			 * database system
			 */
			if (phalcon_array_isset_fetch_str(&where_types, where_condition, SL("bindTypes"))) {
				phalcon_merge_append(&bind_data_types, &where_types);
			}
		}
	} else {
		PHALCON_CONCAT_SVSV(&update_sql, "UPDATE ", &escaped_table, " SET ", &set_clause);
	}

	/**
	 * Perform the update via execute
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &update_sql, &update_values, &bind_data_types);
}

/**
 * Deletes data from a table using custom RBDM SQL syntax
 *
 * <code>
 * //Deleting existing robot
 * $success = $connection->delete(
 *     "robots",
 *     "id = 101"
 * );
 *
 * //Next SQL sentence is generated
 * DELETE FROM `robots` WHERE `id` = 101
 * </code>
 *
 * @param  string $table
 * @param  string $whereCondition
 * @param  array $placeholders
 * @param  array $dataTypes
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, delete){

	zval *table, *where_condition = NULL, *placeholders = NULL, *data_types = NULL, escaped_table = {}, sql = {};

	phalcon_fetch_params(0, 1, 3, &table, &where_condition, &placeholders, &data_types);

	if (!where_condition) {
		where_condition = &PHALCON_GLOBAL(z_null);
	}

	if (!placeholders) {
		placeholders = &PHALCON_GLOBAL(z_null);
	}

	if (!data_types) {
		data_types = &PHALCON_GLOBAL(z_null);
	}

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		PHALCON_CALL_METHOD(&escaped_table, getThis(), "escapeidentifier", table);
	} else {
		PHALCON_CPY_WRT_CTOR(&escaped_table, table);
	}

	if (PHALCON_IS_NOT_EMPTY(where_condition)) {
		PHALCON_CONCAT_SVSV(&sql, "DELETE FROM ", &escaped_table, " WHERE ", where_condition);
	} else {
		PHALCON_CONCAT_SV(&sql, "DELETE FROM ", &escaped_table);
	}

	/**
	 * Perform the update via execute
	 */
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql, placeholders, data_types);
}

/**
 * Gets a list of columns
 *
 * @param array $columnList
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getColumnList){

	zval *column_list, dialect = {};

	phalcon_fetch_params(0, 1, 0, &column_list);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");
	PHALCON_RETURN_CALL_METHOD(&dialect, "getcolumnlist", column_list);
}

/**
 * Appends a LIMIT clause to $sqlQuery argument
 *
 * <code>
 * 	echo $connection->limit("SELECT * FROM robots", 5);
 * </code>
 *
 * @param  	string $sqlQuery
 * @param 	int $number
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, limit){

	zval *sql_query, *number, dialect = {};

	phalcon_fetch_params(0, 2, 0, &sql_query, &number);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");
	PHALCON_RETURN_CALL_METHOD(&dialect, "limit", sql_query, number);
}

/**
 * Generates SQL checking for the existence of a schema.table
 *
 * <code>
 * 	var_dump($connection->tableExists("blog", "posts"));
 * </code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, tableExists){

	zval *table_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, num = {}, first = {};

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "tableexists", table_name, schema_name);

	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHOD(&num, getThis(), "fetchone", &sql, &fetch_num);

	phalcon_array_fetch_long(&first, &num, 0, PH_NOISY);
	RETURN_CTOR(&first);
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 *<code>
 * var_dump($connection->viewExists("active_users", "posts"));
 *</code>
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, viewExists){

	zval *view_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, num = {}, first = {};

	phalcon_fetch_params(0, 1, 1, &view_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "viewexists", view_name, schema_name);

	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	PHALCON_CALL_METHOD(&num, getThis(), "fetchone", &sql, &fetch_num);

	phalcon_array_fetch_long(&first, &num, 0, PH_NOISY);
	RETURN_CTOR(&first);
}

/**
 * Returns a SQL modified with a FOR UPDATE clause
 *
 * @param string $sqlQuery
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, forUpdate){

	zval *sql_query, dialect = {};

	phalcon_fetch_params(0, 1, 0, &sql_query);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");
	PHALCON_RETURN_CALL_METHOD(&dialect, "forupdate", sql_query);
}

/**
 * Returns a SQL modified with a LOCK IN SHARE MODE clause
 *
 * @param string $sqlQuery
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, sharedLock){

	zval *sql_query, dialect = {};

	phalcon_fetch_params(0, 1, 0, &sql_query);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");
	PHALCON_RETURN_CALL_METHOD(&dialect, "sharedlock", sql_query);
}

/**
 * Creates a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createTable){

	zval *table_name, *schema_name, *definition, exception_message, columns, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_CONCAT_SVS(&exception_message, "Invalid definition to create the table '", table_name, "'");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_fetch_str(&columns, definition, SL("columns"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	if (!phalcon_fast_count_ev(&columns)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "createtable", table_name, schema_name, definition);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a table from a schema/database
 *
 * @param string $tableName
 * @param   string $schemaName
 * @param boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropTable){

	zval *table_name, *schema_name = NULL, *if_exists = NULL, dialect = {}, sql = {};

	phalcon_fetch_params(0, 1, 2, &table_name, &schema_name, &if_exists);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "droptable", table_name, schema_name, if_exists);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Creates a view
 *
 * @param string $tableName
 * @param array $definition
 * @param string $schemaName
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createView){

	zval *view_name, *definition, *schema_name = NULL, exception_message = {}, dialect = {}, sql = {};

	phalcon_fetch_params(0, 2, 1, &view_name, &definition, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_CONCAT_SVS(&exception_message, "Invalid definition to create the view '", view_name, "'");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
		return;
	}

	if (!phalcon_array_isset_str(definition, SL("sql"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The table must contain at least one column");
		return;
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "createview", view_name, definition, schema_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a view
 *
 * @param string $viewName
 * @param   string $schemaName
 * @param boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropView){

	zval *view_name, *schema_name = NULL, *if_exists = NULL, dialect = {}, sql = {};

	phalcon_fetch_params(0, 1, 2, &view_name, &schema_name, &if_exists);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	if (!if_exists) {
		if_exists = &PHALCON_GLOBAL(z_true);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "dropview", view_name, schema_name, if_exists);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds a column to a table
 *
 * @param string $tableName
 * @param 	string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addColumn){

	zval *table_name, *schema_name, *column, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "addcolumn", table_name, schema_name, column);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Modifies a table column based on a definition
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, modifyColumn){

	zval *table_name, *schema_name, *column, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "modifycolumn", table_name, schema_name, column);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a column from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $columnName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropColumn){

	zval *table_name, *schema_name, *column_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &column_name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "dropcolumn", table_name, schema_name, column_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addIndex){

	zval *table_name, *schema_name, *index, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "addindex", table_name, schema_name, index);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drop an index from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $indexName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropIndex){

	zval *table_name, *schema_name, *index_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index_name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "dropindex", table_name, schema_name, index_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds a primary key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addPrimaryKey){

	zval *table_name, *schema_name, *index, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &index);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "addprimarykey", table_name, schema_name, index);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a table's primary key
 *
 * @param string $tableName
 * @param string $schemaName
 * @return 	boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropPrimaryKey){

	zval *table_name, *schema_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 2, 0, &table_name, &schema_name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "dropprimarykey", table_name, schema_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Adds a foreign key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ReferenceInterface $reference
 * @return boolean true
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, addForeignKey){

	zval *table_name, *schema_name, *reference, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "addforeignkey", table_name, schema_name, reference);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Drops a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return boolean true
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, dropForeignKey){

	zval *table_name, *schema_name, *reference_name, dialect = {}, sql = {};

	phalcon_fetch_params(0, 3, 0, &table_name, &schema_name, &reference_name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "dropforeignkey", table_name, schema_name, reference_name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Returns the SQL column definition from a column
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getColumnDefinition){

	zval *column, dialect = {};

	phalcon_fetch_params(0, 1, 0, &column);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");
	PHALCON_RETURN_CALL_METHOD(&dialect, "getcolumndefinition", column);
}

/**
 * List all tables on a database
 *
 *<code>
 * 	print_r($connection->listTables("blog"));
 *</code>
 *
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, listTables){

	zval *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, tables = {}, *table;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	/**
	 * Get the SQL to list the tables
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "listtables", schema_name);

	/**
	 * Use fetch Num
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Execute the SQL returning the tables
	 */
	PHALCON_CALL_METHOD(&tables, getThis(), "fetchall", &sql, &fetch_num);

	if (Z_TYPE(tables) == IS_ARRAY) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL(tables)));
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tables), table) {
			zval table_name = {};
			if (phalcon_array_isset_fetch_long(&table_name, table, 0)) {
				phalcon_array_append(return_value, &table_name, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * List all views on a database
 *
 *<code>
 *	print_r($connection->listViews("blog")); ?>
 *</code>
 *
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, listViews){

	zval *schema_name = NULL, dialect = {}, sql = {}, fetch_num = {}, tables = {}, *table;

	phalcon_fetch_params(0, 0, 1, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	/**
	 * Get the SQL to list the tables
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "listviews", schema_name);

	/**
	 * Use fetch Num
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Execute the SQL returning the tables
	 */
	PHALCON_CALL_METHOD(&tables, getThis(), "fetchall", &sql, &fetch_num);

	if (Z_TYPE(tables) == IS_ARRAY) {
		array_init_size(return_value, zend_hash_num_elements(Z_ARRVAL(tables)));

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tables), table) {
			zval table_name = {};
			if (phalcon_array_isset_fetch_long(&table_name, table, 0)) {
				phalcon_array_append(return_value, &table_name, PH_COPY);
			}
		} ZEND_HASH_FOREACH_END();
	}
}

/**
 * Lists table indexes
 *
 *<code>
 *	print_r($connection->describeIndexes('robots_parts'));
 *</code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Index[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, describeIndexes){

	zval *table, *schema = NULL, dialect = {}, fetch_num = {}, sql = {}, describe = {}, *index, indexes = {}, *index_columns;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Get the SQL required to describe indexes from the Dialect
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "describeindexes", table, schema);

	/**
	 * Cryptic Guide: 2: table, 3: from, 4: to
	 */
	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);

	array_init(&indexes);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), index) {
		zval key_name = {}, column_name = {};
		phalcon_array_fetch_long(&key_name, index, 2, PH_NOISY);
		phalcon_array_fetch_long(&column_name, index, 4, PH_NOISY);
		phalcon_array_append_multi_2(&indexes, &key_name, &column_name, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(indexes), idx, str_key, index_columns) {
		zval name = {}, index = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		/**
		 * Every index is abstracted using a Phalcon\Db\Index instance
		 */
		object_init_ex(&index, phalcon_db_index_ce);
		PHALCON_CALL_METHOD(NULL, &index, "__construct", &name, index_columns);

		phalcon_array_update_zval(return_value, &name, &index, PH_COPY);
	} ZEND_HASH_FOREACH_END();
}

/**
 * Lists table references
 *
 *<code>
 * print_r($connection->describeReferences('robots_parts'));
 *</code>
 *
 * @param string $table
 * @param string $schema
 * @return Phalcon\Db\Reference[]
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, describeReferences){

	zval *table, *schema = NULL, dialect = {}, fetch_num = {}, sql = {}, empty_arr = {}, references = {}, describe = {}, *reference, *array_reference;
	zend_string *str_key;
	ulong idx;

	phalcon_fetch_params(0, 1, 1, &table, &schema);

	if (!schema) {
		schema = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	/**
	 * We're using FETCH_NUM to fetch the columns
	 */
	ZVAL_LONG(&fetch_num, PDO_FETCH_NUM);

	/**
	 * Get the SQL required to describe the references from the Dialect
	 */
	PHALCON_CALL_METHOD(&sql, &dialect, "describereferences", table, schema);

	array_init(&empty_arr);
	array_init(&references);

	/**
	 * Execute the SQL returning the
	 */
	PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_num);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(describe), reference) {
		zval constraint_name = {}, referenced_schema = {}, referenced_table = {}, reference_array = {}, column_name = {}, referenced_columns = {};
		phalcon_array_fetch_long(&constraint_name, reference, 2, PH_NOISY);
		if (!phalcon_array_isset(&references, &constraint_name)) {
			phalcon_array_fetch_long(&referenced_schema, reference, 3, PH_NOISY);
			phalcon_array_fetch_long(&referenced_table, reference, 4, PH_NOISY);

			array_init_size(&reference_array, 4);
			phalcon_array_update_str(&reference_array, SL("referencedSchema"), &referenced_schema, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("referencedTable"), &referenced_table, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("columns"), &empty_arr, PH_COPY);
			phalcon_array_update_str(&reference_array, SL("referencedColumns"), &empty_arr, PH_COPY);
			phalcon_array_update_zval(&references, &constraint_name, &reference_array, PH_COPY);
		}

		phalcon_array_fetch_long(&column_name, reference, 1, PH_NOISY);
		phalcon_array_update_zval_str_append_multi_3(&references, &constraint_name, SL("columns"), &column_name, 0);

		phalcon_array_fetch_long(&referenced_columns, reference, 5, PH_NOISY);
		phalcon_array_update_zval_str_append_multi_3(&references, &constraint_name, SL("referencedColumns"), &referenced_columns, 0);
	} ZEND_HASH_FOREACH_END();

	array_init(return_value);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(references), idx, str_key, array_reference) {
		zval name = {}, referenced_schema = {}, referenced_table = {}, columns = {}, referenced_columns = {}, definition = {}, reference = {};
		if (str_key) {
			ZVAL_STR(&name, str_key);
		} else {
			ZVAL_LONG(&name, idx);
		}

		phalcon_array_fetch_str(&referenced_schema, array_reference, SL("referencedSchema"), PH_NOISY);
		phalcon_array_fetch_str(&referenced_table, array_reference, SL("referencedTable"), PH_NOISY);
		phalcon_array_fetch_str(&columns, array_reference, SL("columns"), PH_NOISY);
		phalcon_array_fetch_str(&referenced_columns, array_reference, SL("referencedColumns"), PH_NOISY);

		array_init_size(&definition, 4);
		phalcon_array_update_str(&definition, SL("referencedSchema"), &referenced_schema, PH_COPY);
		phalcon_array_update_str(&definition, SL("referencedTable"), &referenced_table, PH_COPY);
		phalcon_array_update_str(&definition, SL("columns"), &columns, PH_COPY );
		phalcon_array_update_str(&definition, SL("referencedColumns"), &referenced_columns, PH_COPY);

		object_init_ex(&reference, phalcon_db_reference_ce);
		PHALCON_CALL_METHOD(NULL, &reference, "__construct", &name, &definition);

		phalcon_array_update_zval(return_value, &name, &reference, PH_COPY);
	} ZEND_HASH_FOREACH_END();
}

/**
 * Gets creation options from a table
 *
 *<code>
 * print_r($connection->tableOptions('robots'));
 *</code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, tableOptions){

	zval *table_name, *schema_name = NULL, dialect = {}, sql = {}, fetch_assoc = {}, describe = {}, first = {};

	phalcon_fetch_params(0, 1, 1, &table_name, &schema_name);

	if (!schema_name) {
		schema_name = &PHALCON_GLOBAL(z_null);
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&sql, &dialect, "tableoptions", table_name, schema_name);
	if (zend_is_true(&sql)) {
		ZVAL_LONG(&fetch_assoc, PDO_FETCH_ASSOC);

		PHALCON_CALL_METHOD(&describe, getThis(), "fetchall", &sql, &fetch_assoc);

		phalcon_array_fetch_long(&first, &describe, 0, PH_NOISY);
		RETURN_CTOR(&first);
	}

	RETURN_EMPTY_ARRAY();
}

/**
 * Creates a new savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, createSavepoint){

	zval *name, dialect = {}, supports_sp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter.");
		return;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "createsavepoint", name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Releases given savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, releaseSavepoint){

	zval *name, dialect = {}, supports_sp = {}, supports_rsp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	PHALCON_CALL_METHOD(&supports_rsp, &dialect, "supportsreleasesavepoints");
	if (!zend_is_true(&supports_rsp)) {
		RETURN_FALSE;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "releasesavepoint", name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Rollbacks given savepoint
 *
 * @param string $name
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollbackSavepoint){

	zval *name, dialect = {}, supports_sp = {}, sql = {};

	phalcon_fetch_params(0, 1, 0, &name);

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	PHALCON_CALL_METHOD(&sql, &dialect, "rollbacksavepoint", name);
	PHALCON_RETURN_CALL_METHOD(getThis(), "execute", &sql);
}

/**
 * Set if nested transactions should use savepoints
 *
 * @param boolean $nestedTransactionsWithSavepoints
 * @return Phalcon\Db\AdapterInterface
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, setNestedTransactionsWithSavepoints){

	zval *nested_transactions_with_savepoints, transaction_level = {}, dialect = {}, supports_sp = {};

	phalcon_fetch_params(0, 1, 0, &nested_transactions_with_savepoints);

	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY);
	if (PHALCON_GT_LONG(&transaction_level, 0)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Nested transaction with savepoints behavior cannot be changed while a transaction is open");
		return;
	}

	PHALCON_CALL_METHOD(&dialect, getThis(), "getdialect");

	PHALCON_CALL_METHOD(&supports_sp, &dialect, "supportssavepoints");
	if (!zend_is_true(&supports_sp)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Savepoints are not supported by this database adapter");
		return;
	}

	phalcon_update_property_zval(getThis(), SL("_transactionsWithSavepoints"), nested_transactions_with_savepoints);

	RETURN_THIS();
}

/**
 * Returns if nested transactions should use savepoints
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isNestedTransactionsWithSavepoints){


	RETURN_MEMBER(getThis(), "_transactionsWithSavepoints");
}

/**
 * Returns the savepoint name to use for nested transactions
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getNestedTransactionSavepointName){

	zval transaction_level = {};

	phalcon_read_property(&transaction_level, getThis(), SL("_transactionLevel"), PH_NOISY);
	PHALCON_CONCAT_SV(return_value, "PHALCON_SAVEPOINT_", &transaction_level);
}

/**
 * Active SQL statement in the object
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLStatement){


	RETURN_MEMBER(getThis(), "_sqlStatement");
}

/**
 * Active SQL statement in the object without replace bound paramters
 *
 * @return string
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getRealSQLStatement){


	RETURN_MEMBER(getThis(), "_sqlStatement");
}

/**
 * Active SQL statement in the object
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLVariables){


	RETURN_MEMBER(getThis(), "_sqlVariables");
}

/**
 * Active SQL statement in the object
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getSQLBindTypes){


	RETURN_MEMBER(getThis(), "_sqlBindTypes");
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, affectedRows){


	RETURN_MEMBER(getThis(), "_affectedRows");
}

/**
 * Closes the active connection returning success. Phalcon automatically closes and destroys
 * active connections when the request ends
 *
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, close){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (likely(Z_TYPE(pdo) == IS_OBJECT)) {
		phalcon_update_property_zval(getThis(), SL("_pdo"), &PHALCON_GLOBAL(z_null));
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeIdentifier){

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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, escapeString){

	zval *str, pdo = {};

	phalcon_fetch_params(0, 1, 0, &str);

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "quote", str);
}

/**
 * Converts bound parameters such as :name: or ?1 into PDO bind params ?
 *
 *<code>
 * print_r($connection->convertBoundParams('SELECT * FROM robots WHERE name = :name:', array('Bender')));
 *</code>
 *
 * @param string $sql
 * @param array $params
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, convertBoundParams){

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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, lastInsertId){

	zval *sequence_name = NULL, pdo;

	phalcon_fetch_params(0, 0, 1, &sequence_name);

	if (!sequence_name) {
		sequence_name = &PHALCON_GLOBAL(z_null);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
		RETURN_FALSE;
	}

	PHALCON_RETURN_CALL_METHOD(&pdo, "lastinsertid", sequence_name);
}

/**
 * Starts a transaction in the connection
 *
 * @param boolean $nesting
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, begin){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
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

		PHALCON_RETURN_CALL_METHOD(&pdo, "begintransaction");
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, rollback){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
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
		PHALCON_RETURN_CALL_METHOD(&pdo, "rollback");
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, commit){

	zval *nesting = NULL, pdo = {}, transaction_level = {}, events_manager = {}, event_name = {}, ntw_savepoint = {}, savepoint_name = {};
	zval debug_message = {};

	phalcon_fetch_params(0, 0, 1, &nesting);

	if (!nesting) {
		nesting = &PHALCON_GLOBAL(z_true);
	}

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (Z_TYPE(pdo) != IS_OBJECT) {
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
		PHALCON_RETURN_CALL_METHOD(&pdo, "commit");
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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getTransactionLevel){


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
PHP_METHOD(Phalcon_Db_Adapter_Pdo, isUnderTransaction){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	if (likely(Z_TYPE(pdo) == IS_OBJECT)) {
		PHALCON_RETURN_CALL_METHOD(&pdo, "intransaction");
		return;
	}

	RETURN_FALSE;
}

/**
 * Return internal PDO handler
 *
 * @return \PDO
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getInternalHandler){

	phalcon_read_property(return_value, getThis(), SL("_pdo"), PH_NOISY);
}

/**
 * Return the error info, if any
 *
 * @return array
 */
PHP_METHOD(Phalcon_Db_Adapter_Pdo, getErrorInfo){

	zval pdo = {};

	phalcon_read_property(&pdo, getThis(), SL("_pdo"), PH_NOISY);
	PHALCON_RETURN_CALL_METHOD(&pdo, "errorinfo");
}
