
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

#include "db/dialectinterface.h"
#include "kernel/main.h"

zend_class_entry *phalcon_db_dialectinterface_ce;

static const zend_function_entry phalcon_db_dialectinterface_method_entry[] = {
	PHP_ABSTRACT_ME(Phalcon_Db_DialectInterface, select, arginfo_phalcon_db_dialectinterface_select)
	PHP_ABSTRACT_ME(Phalcon_Db_DialectInterface, insert, arginfo_phalcon_db_dialectinterface_insert)
	PHP_ABSTRACT_ME(Phalcon_Db_DialectInterface, update, arginfo_phalcon_db_dialectinterface_update)
	PHP_ABSTRACT_ME(Phalcon_Db_DialectInterface, delete, arginfo_phalcon_db_dialectinterface_delete)
	PHP_FE_END
};

/**
 * Phalcon\Db\DialectInterface initializer
 */
PHALCON_INIT_CLASS(Phalcon_Db_DialectInterface){

	PHALCON_REGISTER_INTERFACE(Phalcon\\Db, DialectInterface, db_dialectinterface, phalcon_db_dialectinterface_method_entry);

	return SUCCESS;
}

/**
 * Builds a INSERT statement
 *
 * @param array $definition
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_DialectInterface, select);

/**
 * Builds a SELECT statement
 *
 * @param array $definition
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_DialectInterface, insert);

/**
 * Builds a UPDATE statement
 *
 * @param array $definition
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_DialectInterface, update);

/**
 * Builds a DELETE statement
 *
 * @param array $definition
 * @return string
 */
PHALCON_DOC_METHOD(Phalcon_Db_DialectInterface, delete);
