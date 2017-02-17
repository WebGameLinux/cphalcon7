
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

	zval *definition, escape_char = {}, columns = {}, selected_columns = {}, distinct = {}, *column, columns_sql = {}, tables = {}, selected_tables = {}, *table;
	zval tables_sql = {}, sql = {}, joins = {}, *join = NULL, where_conditions = {}, where_expression = {}, group_items = {}, group_fields = {};
	zval *group_field = NULL, group_sql = {}, group_clause = {}, having_conditions = {}, having_expression = {}, order_fields = {}, order_items = {}, *order_item;
	zval order_sql = {}, tmp1 = {}, tmp2 = {}, limit_value = {}, number = {}, offset = {};

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

	PHALCON_CALL_METHOD(&escape_char, getThis(), "getescapechar");

	if (Z_TYPE_P(&columns) == IS_ARRAY) {
		array_init(&selected_columns);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&columns), column) {
			zval column_item = {}, column_sql = {}, column_domain = {}, column_domain_sql = {}, column_alias = {}, column_alias_sql = {};
			/**
			 * Escape column name
			 */
			if (
				    phalcon_array_isset_fetch_long(&column_item, column, 0)
				 || phalcon_array_isset_fetch_str(&column_item, column, SL("column"))
			) {
				if (Z_TYPE_P(&column_item) == IS_ARRAY) {
					PHALCON_CALL_METHOD(&column_sql, getThis(), "getsqlexpression", &column_item, &escape_char);
				} else if (PHALCON_IS_STRING(&column_item, "*")) {
					PHALCON_CPY_WRT_CTOR(&column_sql, &column_item);
				} else if (PHALCON_GLOBAL(db).escape_identifiers) {
					PHALCON_CONCAT_VVV(&column_sql, &escape_char, &column_item, &escape_char);
				} else {
					PHALCON_CPY_WRT_CTOR(&column_sql, &column_item);
				}
			} else {
				PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid SELECT definition");
				return;
			}

			/**
			 * Escape column domain
			 */
			if (phalcon_array_isset_fetch_long(&column_domain, column, 1)) {
				if (zend_is_true(&column_domain)) {
					if (PHALCON_GLOBAL(db).escape_identifiers) {
						PHALCON_CONCAT_VVVSV(&column_domain_sql, &escape_char, &column_domain, &escape_char, ".", &column_sql);
					} else {
						PHALCON_CONCAT_VSV(&column_domain_sql, &column_domain, ".", &column_sql);
					}
				} else {
					PHALCON_CPY_WRT_CTOR(&column_domain_sql, &column_sql);
				}
			} else {
				PHALCON_CPY_WRT_CTOR(&column_domain_sql, &column_sql);
			}

			/**
			 * Escape column alias
			 */
			if (phalcon_array_isset_fetch_long(&column_alias, column, 2)) {
				if (zend_is_true(&column_alias)) {
					if (PHALCON_GLOBAL(db).escape_identifiers) {
						PHALCON_CONCAT_VSVVV(&column_alias_sql, &column_domain_sql, " AS ", &escape_char, &column_alias, &escape_char);
					} else {
						PHALCON_CONCAT_VSV(&column_alias_sql, &column_domain_sql, " AS ", &column_alias);
					}
				} else {
					PHALCON_CPY_WRT_CTOR(&column_alias_sql, &column_domain_sql);
				}
			} else {
				PHALCON_CPY_WRT_CTOR(&column_alias_sql, &column_domain_sql);
			}

			phalcon_array_append(&selected_columns, &column_alias_sql, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&columns_sql, SL(", "), &selected_columns);
	} else {
		PHALCON_CPY_WRT_CTOR(&columns_sql, &columns);
	}

	/**
	 * Check and escape tables
	 */
	if (Z_TYPE_P(&tables) == IS_ARRAY) {
		array_init(&selected_tables);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&tables), table) {
			zval sql_table = {};
			PHALCON_CALL_METHOD(&sql_table, getThis(), "getsqltable", table, &escape_char);
			phalcon_array_append(&selected_tables, &sql_table, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&tables_sql, SL(", "), &selected_tables);
	} else {
		PHALCON_CPY_WRT_CTOR(&tables_sql, &tables);
	}

	if (phalcon_array_isset_fetch_str(&distinct, definition, SL("distinct"))) {
		if (Z_TYPE(distinct) == IS_LONG) {
			if (Z_LVAL(distinct) == 0) {
				PHALCON_STR(&sql, "SELECT ALL ");
			} else if (Z_LVAL(distinct) == 1) {
				PHALCON_STR(&sql, "SELECT DISTINCT ");
			} else {
				PHALCON_STR(&sql, "SELECT ");
			}
		} else {
			PHALCON_STR(&sql, "SELECT ");
		}
	} else {
		PHALCON_STR(&sql, "SELECT ");
	}

	PHALCON_SCONCAT_VSV(&sql, &columns_sql, " FROM ", &tables_sql);

	/**
	 * Check for joins
	 */
	if (phalcon_array_isset_fetch_str(&joins, definition, SL("joins"))) {
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(joins), join) {
			zval type = {}, source = {}, sql_table = {}, sql_join = {}, join_conditions_array = {};
			zval *join_condition, join_expressions = {}, join_conditions = {};

			phalcon_array_fetch_str(&type, join, SL("type"), PH_NOISY);
			phalcon_array_fetch_str(&source, join, SL("source"), PH_NOISY);

			PHALCON_CALL_METHOD(&sql_table, getThis(), "getsqltable", &source, &escape_char);

			PHALCON_CONCAT_SVSV(&sql_join, " ", &type, " JOIN ", &sql_table);

			/**
			 * Check if the join has conditions
			 */
			if (phalcon_array_isset_fetch_str(&join_conditions_array, join, SL("conditions"))) {
				if (phalcon_fast_count_ev(&join_conditions_array)) {
					array_init(&join_expressions);

					ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&join_conditions_array), join_condition) {
						zval join_expression = {};
						PHALCON_CALL_METHOD(&join_expression, getThis(), "getsqlexpression", join_condition, &escape_char);
						phalcon_array_append(&join_expressions, &join_expression, PH_COPY);
					} ZEND_HASH_FOREACH_END();

					phalcon_fast_join_str(&join_conditions, SL(" AND "), &join_expressions);
					PHALCON_SCONCAT_SVS(&sql_join, " ON ", &join_conditions, " ");
				}
			}
			phalcon_concat_self(&sql, &sql_join);
		} ZEND_HASH_FOREACH_END();

	}

	/* Check for a WHERE clause */
	if (phalcon_array_isset_fetch_str(&where_conditions, definition, SL("where"))) {
		if (Z_TYPE_P(&where_conditions) == IS_ARRAY) {
			PHALCON_CALL_METHOD(&where_expression, getThis(), "getsqlexpression", &where_conditions, &escape_char);
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_expression);
		} else {
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_conditions);
		}
	}

	/* Check for a GROUP clause */
	if (phalcon_array_isset_fetch_str(&group_fields, definition, SL("group"))) {
		array_init(&group_items);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(group_fields), group_field) {
			zval group_expression = {};
			PHALCON_CALL_METHOD(&group_expression, getThis(), "getsqlexpression", group_field, &escape_char);
			phalcon_array_append(&group_items, &group_expression, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&group_sql, SL(", "), &group_items);

		PHALCON_CONCAT_SV(&group_clause, " GROUP BY ", &group_sql);
		phalcon_concat_self(&sql, &group_clause);
	}

	/* Check for a HAVING clause */
	if (phalcon_array_isset_fetch_str(&having_conditions, definition, SL("having"))) {
		PHALCON_CALL_METHOD(&having_expression, getThis(), "getsqlexpression", &having_conditions, &escape_char);
		PHALCON_SCONCAT_SV(&sql, " HAVING ", &having_expression);
	}

	/* Check for a ORDER clause */
	if (phalcon_array_isset_fetch_str(&order_fields, definition, SL("order"))) {
		array_init(&order_items);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(order_fields), order_item) {
			zval order_expression = {}, order_sql_item = {}, sql_order_type = {}, order_sql_item_type = {};

			phalcon_array_fetch_long(&order_expression, order_item, 0, PH_NOISY);
			PHALCON_CALL_METHOD(&order_sql_item, getThis(), "getsqlexpression", &order_expression, &escape_char);

			/**
			 * In the numeric 1 position could be a ASC/DESC clause
			 */
			if (phalcon_array_isset_fetch_long(&sql_order_type, order_item, 1)) {
				PHALCON_CONCAT_VSV(&order_sql_item_type, &order_sql_item, " ", &sql_order_type);
			} else {
				PHALCON_CPY_WRT_CTOR(&order_sql_item_type, &order_sql_item);
			}

			phalcon_array_append(&order_items, &order_sql_item_type, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&order_sql, SL(", "), &order_items);
		PHALCON_SCONCAT_SV(&sql, " ORDER BY ", &order_sql);
	}

	/**
	 * Check for a LIMIT condition
	 */
	if (phalcon_array_isset_fetch_str(&limit_value, definition, SL("limit"))) {
		if (likely(Z_TYPE_P(&limit_value) == IS_ARRAY)) {
			if (likely(phalcon_array_isset_fetch_str(&number, &limit_value, SL("number")))) {
				phalcon_array_fetch_str(&tmp1, &number, SL("value"), PH_NOISY);

				/**
				 * Check for a OFFSET condition
				 */
				if (phalcon_array_isset_fetch_str(&offset, &limit_value, SL("offset"))) {
					phalcon_array_fetch_str(&tmp2, &offset, SL("value"), PH_NOISY);
					PHALCON_SCONCAT_SVSV(&sql, " LIMIT ", &tmp1, " OFFSET ", &tmp2);
				} else {
					PHALCON_SCONCAT_SV(&sql, " LIMIT ", &tmp1);
				}
			}
		} else {
			PHALCON_SCONCAT_SV(&sql, " LIMIT ", &limit_value);
		}
	}

	/**
	 * Check for a FOR UPDATE clause
	 */
	if (phalcon_array_isset_str(definition, SL("forupdate"))) {
		PHALCON_RETURN_CALL_METHOD(getThis(), "forupdate", &sql);
		return;
	}

	RETURN_CTOR(&sql);
}

/**
 * Builds a INSERT statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,insert)
{
	zval *definition, table = {}, fields = {}, number_fields = {}, values = {}, exception_message = {}, escaped_table = {}, escape_char = {};
	zval *row_values = NULL, joined_rows = {}, joined_values = {}, escaped_fields = {}, *field, joined_fields = {};

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

	phalcon_fast_count(&number_fields, &fields);

	PHALCON_CALL_METHOD(&escaped_table, getThis(), "getsqltable", &table);
	PHALCON_CALL_METHOD(&escape_char, getThis(), "getescapechar");

	/**
	 * Build the final SQL INSERT statement
	 */
	array_init(&joined_rows);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(values), row_values) {
		zval number_values = {}, insert_row_values = {}, *value, joined_row = {};

		phalcon_fast_count(&number_values, row_values);

		/**
		 * The number of calculated values must be equal to the number of fields in the
		 * model
		 */
		if (!PHALCON_IS_EQUAL(&number_fields, &number_values)) {
			PHALCON_CONCAT_SVSVS(&exception_message, "The fields count(", &number_fields, ") does not match the values count(", &number_values, ")");
			PHALCON_THROW_EXCEPTION_ZVAL(phalcon_db_exception_ce, &exception_message);
			return;
		}

		array_init(&insert_row_values);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(row_values), value) {
			zval insert_value = {};

			PHALCON_CALL_METHOD(&insert_value, getThis(), "getsqlexpression", value, &escape_char);

			phalcon_array_append(&insert_row_values, &insert_value, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&joined_row, SL(", "), &insert_row_values);
		phalcon_array_append(&joined_rows, &joined_row, 0);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&joined_values, SL("), ("), &joined_rows);

	if (Z_TYPE(fields) == IS_ARRAY) {
		if (PHALCON_GLOBAL(db).escape_identifiers) {
			array_init(&escaped_fields);

			ZEND_HASH_FOREACH_VAL(Z_ARRVAL(fields), field) {
				zval escaped_field = {};
				PHALCON_CONCAT_VVV(&escaped_field, &escape_char, field, &escape_char);
				phalcon_array_append(&escaped_fields, &escaped_field, PH_COPY);
			} ZEND_HASH_FOREACH_END();

		} else {
			PHALCON_CPY_WRT_CTOR(&escaped_fields, &fields);
		}

		phalcon_fast_join_str(&joined_fields, SL(", "), &escaped_fields);

		PHALCON_CONCAT_SVSVSVS(return_value, "INSERT INTO ", &escaped_table, " (", &joined_fields, ") VALUES (", &joined_values, ")");
	} else {
		PHALCON_CONCAT_SVSVS(return_value, "INSERT INTO ", &escaped_table, " VALUES (", &joined_values, ")");
	}
}

/**
 * Builds a UPDATE statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,update){

	zval *definition, tables = {}, fields = {}, values = {}, escape_char = {}, updated_tables = {}, *table, tables_sql = {}, sql = {};
	zval updated_fields = {}, *column, columns_sql = {}, where_conditions = {}, where_expression = {};
	zval order_fields = {}, order_items = {}, *order_item, order_sql = {}, limit_value = {}, number = {}, offset = {}, tmp1 = {}, tmp2 = {};
	zend_string *str_key;
	ulong idx;

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

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		phalcon_return_property(&escape_char, getThis(), SL("_escapeChar"));
	}

	/**
	 * Check and escape tables
	 */
	array_init(&updated_tables);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL(tables), table) {
		zval table_expression = {};
		PHALCON_CALL_METHOD(&table_expression, getThis(), "getsqltable", table, &escape_char);
		phalcon_array_append(&updated_tables, &table_expression, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&tables_sql, SL(", "), &updated_tables);

	PHALCON_SCONCAT_SV(&sql, "UPDATE ", &tables_sql);

	array_init(&updated_fields);

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(fields), idx, str_key, column) {
		zval position = {}, column_name = {}, column_quoted = {}, value_expr = {}, value = {}, value_expression = {}, column_expression = {};

		if (str_key) {
			ZVAL_STR(&position, str_key);
		} else {
			ZVAL_LONG(&position, idx);
		}
		if (Z_TYPE_P(column) != IS_ARRAY) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid UPDATE definition");
			return;
		}

		phalcon_array_fetch_str(&column_name, column, SL("name"), PH_NOISY);
		phalcon_array_fetch(&value_expr, &values, &position, PH_NOISY);
		phalcon_array_fetch_str(&value, &value_expr, SL("value"), PH_NOISY);

		PHALCON_CALL_METHOD(&value_expression, getThis(), "getsqlexpression", &value, &escape_char);

		PHALCON_CONCAT_VVV(&column_quoted, &escape_char, &column_name, &escape_char);
		PHALCON_CONCAT_VSV(&column_expression, &column_quoted, " = ", &value_expression);

		phalcon_array_append(&updated_fields, &column_expression, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&columns_sql, SL(", "), &updated_fields);

	PHALCON_SCONCAT_SV(&sql, " SET ", &columns_sql);

	/* Check for a WHERE clause */
	if (phalcon_array_isset_fetch_str(&where_conditions, definition, SL("where"))) {
		if (Z_TYPE(where_conditions) == IS_ARRAY) {
			PHALCON_CALL_METHOD(&where_expression, getThis(), "getsqlexpression", &where_conditions, &escape_char);
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_expression);
		} else {
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_conditions);
		}
	}

	/* Check for a ORDER clause */
	if (phalcon_array_isset_fetch_str(&order_fields, definition, SL("order"))) {
		array_init(&order_items);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(order_fields), order_item) {
			zval order_expression = {}, order_sql_item = {}, sql_order_type = {}, order_sql_item_type = {};

			phalcon_array_fetch_long(&order_expression, order_item, 0, PH_NOISY);

			PHALCON_CALL_METHOD(&order_sql_item, getThis(), "getsqlexpression", &order_expression, &escape_char);

			/**
			 * In the numeric 1 position could be a ASC/DESC clause
			 */
			if (phalcon_array_isset_fetch_long(&sql_order_type, order_item, 1)) {
				PHALCON_CONCAT_VSV(&order_sql_item_type, &order_sql_item, " ", &sql_order_type);
			} else {
				PHALCON_CPY_WRT_CTOR(&order_sql_item_type, &order_sql_item);
			}
			phalcon_array_append(&order_items, &order_sql_item_type, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&order_sql, SL(", "), &order_items);
		PHALCON_SCONCAT_SV(&sql, " ORDER BY ", &order_sql);
	}

	/**
	 * Check for a LIMIT condition
	 */
	if (phalcon_array_isset_fetch_str(&limit_value, definition, SL("limit"))) {
		if (likely(Z_TYPE_P(&limit_value) == IS_ARRAY)) {
			if (likely(phalcon_array_isset_fetch_str(&number, &limit_value, SL("number")))) {
				phalcon_array_fetch_str(&tmp1, &number, SL("value"), PH_NOISY);

				/**
				 * Check for a OFFSET condition
				 */
				if (phalcon_array_isset_fetch_str(&offset, &limit_value, SL("offset"))) {
					phalcon_array_fetch_str(&tmp2, &offset, SL("value"), PH_NOISY);
					PHALCON_SCONCAT_SVSV(&sql, " LIMIT ", &tmp1, " OFFSET ", &tmp2);
				} else {
					PHALCON_SCONCAT_SV(&sql, " LIMIT ", &tmp1);
				}
			}
		} else {
			PHALCON_SCONCAT_SV(&sql, " LIMIT ", &limit_value);
		}
	}

	RETURN_CTOR(&sql);
}

/**
 * Builds a DELETE statement
 *
 * @param array $definition
 * @return string
 */
PHP_METHOD(Phalcon_Db_Dialect_Mongo,delete){

	zval *definition, tables = {}, escape_char = {}, updated_tables = {}, *table, tables_sql = {}, sql = {}, where_conditions = {}, where_expression = {};
	zval order_fields = {}, order_items = {}, *order_item, order_sql = {}, limit_value = {}, number = {}, offset = {}, tmp1 = {}, tmp2 = {};

	phalcon_fetch_params(0, 1, 0, &definition);

	if (Z_TYPE_P(definition) != IS_ARRAY) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "Invalid Update definition");
		return;
	}
	if (!phalcon_array_isset_fetch_str(&tables, definition, SL("tables"))) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_db_exception_ce, "The index 'tables' is required in the definition array");
		return;
	}

	if (PHALCON_GLOBAL(db).escape_identifiers) {
		phalcon_return_property(&escape_char, getThis(), SL("_escapeChar"));
	}

	/**
	 * Check and escape tables
	 */
	array_init(&updated_tables);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(&tables), table) {
		zval sql_table = {};
		PHALCON_CALL_METHOD(&sql_table, getThis(), "getsqltable", table, &escape_char);
		phalcon_array_append(&updated_tables, &sql_table, PH_COPY);
	} ZEND_HASH_FOREACH_END();

	phalcon_fast_join_str(&tables_sql, SL(", "), &updated_tables);

	PHALCON_SCONCAT_SV(&sql, "DELETE FROM ", &tables_sql);

	/* Check for a WHERE clause */
	if (phalcon_array_isset_fetch_str(&where_conditions, definition, SL("where"))) {
		if (Z_TYPE_P(&where_conditions) == IS_ARRAY) {
			PHALCON_CALL_METHOD(&where_expression, getThis(), "getsqlexpression", &where_conditions, &escape_char);
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_expression);
		} else {
			PHALCON_SCONCAT_SV(&sql, " WHERE ", &where_conditions);
		}
	}

	/* Check for a ORDER clause */
	if (phalcon_array_isset_fetch_str(&order_fields, definition, SL("order"))) {
		array_init(&order_items);

		ZEND_HASH_FOREACH_VAL(Z_ARRVAL(order_fields), order_item) {
			zval order_expression = {}, order_sql_item = {}, sql_order_type = {}, order_sql_item_type = {};

			phalcon_array_fetch_long(&order_expression, order_item, 0, PH_NOISY);

			PHALCON_CALL_METHOD(&order_sql_item, getThis(), "getsqlexpression", &order_expression, &escape_char);

			/**
			 * In the numeric 1 position could be a ASC/DESC clause
			 */
			if (phalcon_array_isset_fetch_long(&sql_order_type, order_item, 1)) {
				PHALCON_CONCAT_VSV(&order_sql_item_type, &order_sql_item, " ", &sql_order_type);
			} else {
				PHALCON_CPY_WRT_CTOR(&order_sql_item_type, &order_sql_item);
			}
			phalcon_array_append(&order_items, &order_sql_item_type, PH_COPY);
		} ZEND_HASH_FOREACH_END();

		phalcon_fast_join_str(&order_sql, SL(", "), &order_items);
		PHALCON_SCONCAT_SV(&sql, " ORDER BY ", &order_sql);
	}

	/**
	 * Check for a LIMIT condition
	 */
	if (phalcon_array_isset_fetch_str(&limit_value, definition, SL("limit"))) {
		if (likely(Z_TYPE_P(&limit_value) == IS_ARRAY)) {
			if (likely(phalcon_array_isset_fetch_str(&number, &limit_value, SL("number")))) {
				phalcon_array_fetch_str(&tmp1, &number, SL("value"), PH_NOISY);

				/**
				 * Check for a OFFSET condition
				 */
				if (phalcon_array_isset_fetch_str(&offset, &limit_value, SL("offset"))) {
					phalcon_array_fetch_str(&tmp2, &offset, SL("value"), PH_NOISY);
					PHALCON_SCONCAT_SVSV(&sql, " LIMIT ", &tmp1, " OFFSET ", &tmp2);
				} else {
					PHALCON_SCONCAT_SV(&sql, " LIMIT ", &tmp1);
				}
			}
		} else {
			PHALCON_SCONCAT_SV(&sql, " LIMIT ", &limit_value);
		}
	}

	RETURN_CTOR(&sql);
}
