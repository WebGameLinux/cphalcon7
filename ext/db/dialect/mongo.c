
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
#include "db/dialect.h"
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

PHP_METHOD(Phalcon_Db_Dialect_Mongo, getColumnDefinition);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, modifyColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropColumn);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addIndex);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropIndex);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropPrimaryKey);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropForeignKey);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, createTable);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropTable);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, createView);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropView);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, tableExists);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, viewExists);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeColumns);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, listTables);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, listViews);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeIndexes);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeIndex);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeReferences);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, tableOptions);
PHP_METHOD(Phalcon_Db_Dialect_Mongo, getDefaultValue);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_db_dialect_mongo_describeindex, 0, 0, 1)
	ZEND_ARG_INFO(0, indexName)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_db_dialect_mongo_method_entry[] = {
	PHP_ME(Phalcon_Db_Dialect_Mongo, getColumnDefinition, arginfo_phalcon_db_dialectinterface_getcolumndefinition, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, addColumn, arginfo_phalcon_db_dialectinterface_addcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, modifyColumn, arginfo_phalcon_db_dialectinterface_modifycolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropColumn, arginfo_phalcon_db_dialectinterface_dropcolumn, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, addIndex, arginfo_phalcon_db_dialectinterface_addindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropIndex, arginfo_phalcon_db_dialectinterface_dropindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, addPrimaryKey, arginfo_phalcon_db_dialectinterface_addprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropPrimaryKey, arginfo_phalcon_db_dialectinterface_dropprimarykey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, addForeignKey, arginfo_phalcon_db_dialectinterface_addforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropForeignKey, arginfo_phalcon_db_dialectinterface_dropforeignkey, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, createTable, arginfo_phalcon_db_dialectinterface_createtable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropTable, arginfo_phalcon_db_dialectinterface_droptable, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, createView, arginfo_phalcon_db_dialectinterface_createview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, dropView, arginfo_phalcon_db_dialectinterface_dropview, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, tableExists, arginfo_phalcon_db_dialectinterface_tableexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, viewExists, arginfo_phalcon_db_dialectinterface_viewexists, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, describeColumns, arginfo_phalcon_db_dialectinterface_describecolumns, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, listTables, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, listViews, arginfo_phalcon_db_dialectinterface_listtables, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, describeIndexes, arginfo_phalcon_db_dialectinterface_describeindexes, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, describeIndex, arginfo_phalcon_db_dialect_mongo_describeindex, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, describeReferences, arginfo_phalcon_db_dialectinterface_describereferences, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, tableOptions, arginfo_phalcon_db_dialectinterface_tableoptions, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Db_Dialect_Mongo, getDefaultValue, arginfo_phalcon_db_dialectinterface_getdefaultvalue, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Db\Dialect\Mongo initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_Dialect_Mongo){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Db\\Dialect, Mongo, db_dialect_mongo, phalcon_db_dialect_ce, phalcon_db_dialect_mongo_method_entry, 0);

	zend_declare_property_string(phalcon_db_dialect_mongo_ce, SL("_escapeChar"), "\"", ZEND_ACC_PROTECTED);

	zend_class_implements(phalcon_db_dialect_mongo_ce, 1, phalcon_db_dialectinterface_ce);

	return SUCCESS;
}

/**
 * Gets the column name in Mongo
 *
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, getColumnDefinition){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
	return;
}

/**
 * Generates SQL to add a column to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addColumn){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
	return;
}

/**
 * Generates SQL to modify a column in a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\ColumnInterface $column
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, modifyColumn){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to delete a column from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $columnName
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropColumn){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addIndex){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates SQL to delete an index from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $indexName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropIndex){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates SQL to add the primary key to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\IndexInterface $index
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addPrimaryKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates SQL to delete primary key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropPrimaryKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates SQL to add an index to a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param Phalcon\Db\Reference $reference
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, addForeignKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to delete a foreign key from a table
 *
 * @param string $tableName
 * @param string $schemaName
 * @param string $referenceName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropForeignKey){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to create a table in Mongo
 *
 * @param 	string $tableName
 * @param string $schemaName
 * @param array $definition
 * @return 	string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, createTable){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't need");
}

/**
 * Generates SQL to drop a table
 *
 * @param  string $tableName
 * @param  string $schemaName
 * @param  boolean $ifExists
 * @return boolean
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropTable){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't need");
}

/**
 * Generates SQL to create a view
 *
 * @param string $viewName
 * @param array $definition
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, createView){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to drop a view
 *
 * @param string $viewName
 * @param string $schemaName
 * @param boolean $ifExists
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, dropView){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL checking for the existence of a schema.table
 *
 * <code>echo $dialect->tableExists("posts", "blog")</code>
 * <code>echo $dialect->tableExists("posts")</code>
 *
 * @param string $tableName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, tableExists){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL checking for the existence of a schema.view
 *
 * @param string $viewName
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, viewExists){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates a SQL describing a table
 *
 * <code>print_r($dialect->describeColumns("posts") ?></code>
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeColumns){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL list all tables on database
 *
 * <code>print_r($dialect->listTables("blog")) ?></code>
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, listTables){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates the SQL to list all views of a schema or user
 *
 * @param string $schemaName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, listViews){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates SQL to query indexes detail on a table
 *
 * @param string $indexName
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeIndex){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Not implemented");
}

/**
 * Generates SQL to query foreign keys on a table
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, describeReferences){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Generates the SQL to describe the table creation options
 *
 * @param string $table
 * @param string $schema
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, tableOptions){

	PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Don't support by MongoDB");
}

/**
 * Return the default value
 *
 * @param string $defaultValue
 * @param string $columnDefinition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo, getDefaultValue){

	zval *default_value, *column_type, slash = {}, value_cslashes = {};
	int type;

	phalcon_fetch_params(0, 2, 0, &default_value, &column_type);

	type = Z_LVAL_P(column_type);

	if (Z_TYPE_P(column_type) == IS_LONG) {
		type = Z_LVAL_P(column_type);

		if (type == PHALCON_DB_COLUMN_TYPE_BOOLEAN) {
			if (zend_is_true(default_value)) {
				ZVAL_STRING(return_value, "true");
			} else {
				ZVAL_STRING(return_value, "false");
			}
			return;
		} else if (phalcon_comparestr_str(default_value, SL("CURRENT_TIMESTAMP"), &PHALCON_GLOBAL(z_false))) {
			ZVAL_STRING(return_value, "CURRENT_TIMESTAMP");
			return;
		}  else if (
			type == PHALCON_DB_COLUMN_TYPE_INTEGER
			|| type == PHALCON_DB_COLUMN_TYPE_BIGINTEGER
			|| type == PHALCON_DB_COLUMN_TYPE_FLOAT
			|| type == PHALCON_DB_COLUMN_TYPE_DOUBLE
			|| type == PHALCON_DB_COLUMN_TYPE_DECIMAL
			|| type == PHALCON_DB_COLUMN_TYPE_MONEY
		) {
			RETURN_CTOR(default_value);
		}
	}
	ZVAL_STRING(&slash, "\"");
	PHALCON_CALL_FUNCTION(&value_cslashes, "addcslashes", default_value, &slash);
	PHALCON_CONCAT_SVS(return_value, "\"", &value_cslashes, "\"");
}
