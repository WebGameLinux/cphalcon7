
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

#include "logger/formatter/syslog.h"
#include "logger/formatter.h"
#include "logger/formatterinterface.h"

#include "kernel/main.h"
#include "kernel/array.h"
#include "kernel/fcall.h"

/**
 * Phalcon\Logger\Formatter\Syslog
 *
 * Prepares a message to be used in a Syslog backend
 */
zend_class_entry *phalcon_logger_formatter_syslog_ce;

PHP_METHOD(Phalcon_Logger_Formatter_Syslog, format);

static const zend_function_entry phalcon_logger_formatter_syslog_method_entry[] = {
	PHP_ME(Phalcon_Logger_Formatter_Syslog, format, arginfo_phalcon_logger_formatterinterface_format, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/**
 * Phalcon\Logger\Formatter\Syslog initializer
 */
PHALCON_INIT_CLASS(Phalcon_Logger_Formatter_Syslog){

	PHALCON_REGISTER_CLASS_EX(Phalcon\\Logger\\Formatter, Syslog, logger_formatter_syslog, phalcon_logger_formatter_ce, phalcon_logger_formatter_syslog_method_entry, 0);

	zend_class_implements(phalcon_logger_formatter_syslog_ce, 1, phalcon_logger_formatterinterface_ce);

	return SUCCESS;
}

/**
 * Applies a format to a message before sent it to the internal log
 *
 * @param string $message
 * @param int $type
 * @param int $timestamp
 * @param array $context
 * @return array
 */
PHP_METHOD(Phalcon_Logger_Formatter_Syslog, format){

	zval *message, *type, *timestamp, *context, interpolated = {};

	phalcon_fetch_params(0, 4, 0, &message, &type, &timestamp, &context);
	
	if (Z_TYPE_P(context) == IS_ARRAY) {
		PHALCON_CALL_METHOD(&interpolated, getThis(), "interpolate", message, context);
	} else {
		PHALCON_CPY_WRT(&interpolated, message);
	}

	array_init_size(return_value, 2);
	phalcon_array_append(return_value, type, PH_COPY);
	phalcon_array_append(return_value, &interpolated, PH_COPY);
}
