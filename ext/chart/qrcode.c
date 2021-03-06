/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2013 Phalcon Team (http://www.phalconphp.com)       |
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_phalcon.h"
#include "phalcon.h"

#if PHALCON_USE_QRENCODE
# include <wand/MagickWand.h>
# ifdef PHALCON_USE_ZBAR
#  include <zbar.h>
# endif
# include <qrencode.h>
# define PNG_SKIP_SETJMP_CHECK 1
# include <png.h>
#endif

#include "php_open_temporary_file.h"

#include "kernel/main.h"
#include "kernel/memory.h"

#include "kernel/exception.h"
#include "kernel/object.h"
#include "kernel/fcall.h"
#include "kernel/array.h"
#include "kernel/hash.h"
#include "kernel/concat.h"
#include "kernel/operators.h"
#include "kernel/string.h"
#include "kernel/output.h"

#include "chart/qrcode.h"
#include "chart/exception.h"

/**
 * Phalcon\Chart\QRcode
 *
 *<code>
 * $qr = new \Phalcon\Chart\QRcode();
 * $ret = $qr->generate('Phalcon framework');
 * $data = $qr->render();
 * $data = $qr->render(NULL, NULL, 'FFCC00', '000000');
 * $ret = $qr->save('unit-tests/assets/qr.png');
 * $ret = $qr->save('unit-tests/assets/qr.png', NULL, NULL, 'FFCC00', '000000');
 * $ret = $qr->scan('unit-tests/assets/qr.png');
 *</code>
 */
zend_class_entry *phalcon_chart_qrcode_ce;

PHP_METHOD(Phalcon_Chart_QRcode, __construct);
PHP_METHOD(Phalcon_Chart_QRcode, generate);
PHP_METHOD(Phalcon_Chart_QRcode, render);
PHP_METHOD(Phalcon_Chart_QRcode, save);
PHP_METHOD(Phalcon_Chart_QRcode, scan);

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_generate, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, version, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, casesensitive, _IS_BOOL, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_scan, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_render, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, margin, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, foreground, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, background, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_phalcon_chart_qrcode_save, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, margin, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, foreground, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, background, IS_LONG, 1)
ZEND_END_ARG_INFO()

static const zend_function_entry phalcon_chart_qrcode_method_entry[] = {
	PHP_ME(Phalcon_Chart_QRcode, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(Phalcon_Chart_QRcode, generate, arginfo_phalcon_chart_qrcode_generate, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, render, arginfo_phalcon_chart_qrcode_render, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, save, arginfo_phalcon_chart_qrcode_save, ZEND_ACC_PUBLIC)
	PHP_ME(Phalcon_Chart_QRcode, scan, arginfo_phalcon_chart_qrcode_scan, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

#if PHALCON_USE_QRENCODE
# define INCHES_PER_METER (100.0/2.54)

int  phalcon_qrcode_handle;
static int dpi = 72;

typedef struct {
    QRcode *c;
} php_qrcode;

static void phalcon_qr_dtor(zend_resource *rsrc)
{
    php_qrcode *qr = (php_qrcode *) rsrc->ptr;

    if (qr->c)
        QRcode_free (qr->c);
    efree (qr);
}

static int phalcon_color_set(unsigned int color[4], const char *value)
{
        int len = strlen(value);
        int count;
        if(len == 6) {
                count = sscanf(value, "%02x%02x%02x%n", &color[0], &color[1], &color[2], &len);
                if(count < 3 || len != 6) {
                        return -1;
                }
                color[3] = 255;
        } else if(len == 8) {
                count = sscanf(value, "%02x%02x%02x%02x%n", &color[0], &color[1], &color[2], &color[3], &len);
                if(count < 4 || len != 8) {
                        return -1;
                }
        } else {
                return -1;
        }
        return 0;
}
#endif

/**
 * Phalcon\Chart\QRcode initializer
 */
PHALCON_INIT_CLASS(Phalcon_Chart_QRcode){

	PHALCON_REGISTER_CLASS(Phalcon\\Chart, QRcode, chart_qrcode, phalcon_chart_qrcode_method_entry, 0);

#if PHALCON_USE_QRENCODE
    phalcon_qrcode_handle = zend_register_list_destructors_ex(phalcon_qr_dtor, NULL, phalcon_qrcode_handle_name, module_number);

	/* Mode */
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_NUL"), QR_MODE_NUL TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_NUM"), QR_MODE_NUM TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_AN"), QR_MODE_AN TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_8"), QR_MODE_8 TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("MODE_KANJI"), QR_MODE_KANJI TSRMLS_CC);

	/* Level */
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_L"), QR_ECLEVEL_L TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_M"), QR_ECLEVEL_M TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_Q"), QR_ECLEVEL_Q TSRMLS_CC);
	zend_declare_class_constant_long(phalcon_chart_qrcode_ce, SL("LEVEL_H"), QR_ECLEVEL_H TSRMLS_CC);

	zend_declare_property_null(phalcon_chart_qrcode_ce, SL("_qr"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(phalcon_chart_qrcode_ce, SL("_text"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_version"), 6, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_level"), QR_ECLEVEL_H, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_mode"), QR_MODE_8, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(phalcon_chart_qrcode_ce, SL("_casesensitive"), 1, ZEND_ACC_PROTECTED TSRMLS_CC);
#endif
	return SUCCESS;
}

/**
 * Phalcon\Chart\QRcode constructor
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
 *     $qr->save('qr.png');
 */
PHP_METHOD(Phalcon_Chart_QRcode, __construct){

#ifndef PHALCON_QRCODE
	PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "QRcode is not enable, check your configuration");
#endif
}

/**
 * Generate QR data
 *
 * @param string $text
 * @param int $version
 * @param int $level
 * @param int $mode
 * @param boolean $casesensitive
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_QRcode, generate){

#if PHALCON_USE_QRENCODE
	zval *text, *_version = NULL, *_level = NULL, *_mode = NULL, *_casesensitive = NULL, version = {}, level = {}, mode = {}, casesensitive = {};
	zval zid = {};
	php_qrcode *qr = NULL;

	phalcon_fetch_params(0, 1, 4, &text, &_version, &level, &_mode, &_casesensitive);

	phalcon_update_property_zval(getThis(), SL("_text"), text);

	if (!_version || Z_TYPE_P(_version) == IS_NULL) {
		phalcon_read_property(&version, getThis(), SL("_version"), PH_NOISY);
	} else {
		if (Z_LVAL_P(_version) < 1 || Z_LVAL_P(_version) > 40) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "version must be within the range of 1 to 40");
			return;
		}
		PHALCON_CPY_WRT(&version, _version);
	}

	if (!_level || Z_TYPE_P(_level) == IS_NULL) {
		phalcon_read_property(&level, getThis(), SL("_level"), PH_NOISY);
	} else {
		if (Z_LVAL_P(_level) != QR_ECLEVEL_L && Z_LVAL_P(_level) != QR_ECLEVEL_M && Z_LVAL_P(_level) != QR_ECLEVEL_Q && Z_LVAL_P(_level) != QR_ECLEVEL_H) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Error level. there are 4 values: LEVEL_L, LEVEL_M, LEVEL_Q, LEVEL_H");
			return;
		}
		PHALCON_CPY_WRT(&level, _level);
	}


	if (!_mode || Z_TYPE_P(_mode) == IS_NULL) {
		phalcon_read_property(&mode, getThis(), SL("_mode"), PH_NOISY);
	} else {
		if (Z_LVAL_P(_mode) != QR_MODE_NUL && Z_LVAL_P(_mode) != QR_MODE_NUM && Z_LVAL_P(_mode) != QR_MODE_8 && Z_LVAL_P(_mode) != QR_MODE_KANJI) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Error mode. there are 4 values: MODE_NUL, MODE_NUM, MODE_8, MODE_KANJI");
			return;
		}
		PHALCON_CPY_WRT(&mode, _mode);
	}

	if (!_casesensitive || Z_TYPE_P(_casesensitive) == IS_NULL) {
		phalcon_read_property(&casesensitive, getThis(), SL("_casesensitive"), PH_NOISY);
	} else {
		PHALCON_CPY_WRT(&casesensitive, _casesensitive);
	}

	qr = (php_qrcode *) emalloc (sizeof (php_qrcode));

	if (Z_LVAL(mode) == QR_MODE_8) {
		qr->c = QRcode_encodeString8bit(Z_STRVAL_P(text), Z_LVAL(version), Z_LVAL(level));
	} else {
		qr->c = QRcode_encodeString(Z_STRVAL_P(text), Z_LVAL(version), Z_LVAL(level), Z_LVAL(mode), zend_is_true(&casesensitive) ? 1 : 0);
	}

	if (qr->c == NULL)  {
		efree(qr);
		RETURN_FALSE;
	} else {
		ZVAL_RES(&zid, zend_register_resource(qr, phalcon_qrcode_handle));
		phalcon_update_property_zval(getThis(), SL("_qr"), &zid);
		RETURN_TRUE;
	}
#endif
}

/**
 * Render the image and return the binary string.
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework');
 *     $data = \Phalcon\Chart\QRcode::render();
 *
 * @param int $size
 * @param int $margin.
 * @param string $foreground
 * @param string $background
 * @return string
 */
PHP_METHOD(Phalcon_Chart_QRcode, render){

#if PHALCON_USE_QRENCODE
	zval *size = NULL, *margin = NULL, *foreground=NULL, *background=NULL, zid = {};
	FILE *fp = NULL;
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;
	png_byte alpha_values[2];
	php_qrcode *qr = NULL;
	static unsigned int fg_color[4] = {0, 0, 0, 255};
	static unsigned int bg_color[4] = {255, 255, 255, 255};
	long s = 3, m = 4;
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;
	zend_string *path;
	int b;
	char buf[4096];

	phalcon_fetch_params(0, 0, 4, &size, &margin, &foreground, &background);

	if (size && Z_TYPE_P(size) != IS_NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "size parameter must be int");
		return;
	}

	if (margin && Z_TYPE_P(margin) != IS_NULL) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "margin parameter must be int");
		return;
	}

	if (foreground && zend_is_true(foreground)) {
		if(phalcon_color_set(fg_color, Z_STRVAL_P(foreground))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid foreground color value");
			return;
		}
	}

	if (background && zend_is_true(background)) {
		if(phalcon_color_set(bg_color, Z_STRVAL_P(background))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid background color value");
			return;
		}
	}

	if (size && Z_TYPE_P(size) == IS_LONG) {
		s = Z_LVAL_P(size);
	}
	if (margin && Z_TYPE_P(margin) == IS_LONG) {
		m = Z_LVAL_P(margin);
	}

	phalcon_read_property(&zid, getThis(), SL("_qr"), PH_NOISY);

	if (Z_TYPE(zid) <= IS_NULL) {
		RETURN_FALSE;
	}

	if ((qr = (php_qrcode *)zend_fetch_resource(Z_RES(zid), phalcon_qrcode_handle_name, phalcon_qrcode_handle)) == NULL) {
		RETURN_FALSE;
	}

	fp = php_open_temporary_file(NULL, NULL, &path);
	if (!fp) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Unable to open temporary file for writing");
		return;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to initialize PNG writer");
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to initialize PNG writer");
		return;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to write PNG writer");
		return;
	}

	// width = version * 4 + 17
	realwidth = (qr->c->width + m * 2) * s;
	row = (unsigned char *) emalloc ((realwidth + 7) / 8);

	palette = (png_colorp) emalloc(sizeof(png_color) * 2);
	palette[0].red   = fg_color[0];
	palette[0].green = fg_color[1];
	palette[0].blue  = fg_color[2];
	palette[1].red   = bg_color[0];
	palette[1].green = bg_color[1];
	palette[1].blue  = bg_color[2];
	alpha_values[0] = fg_color[3];
	alpha_values[1] = bg_color[3];
	png_set_PLTE(png_ptr, info_ptr, palette, 2);
	png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr,
		realwidth, realwidth,
		1,
		PNG_COLOR_TYPE_PALETTE,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_pHYs(png_ptr, info_ptr,
		dpi * INCHES_PER_METER,
		dpi * INCHES_PER_METER,
		PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	memset(row, 0xff, (realwidth + 7) / 8);
	for(y = 0; y < m * s; y++) {
		png_write_row(png_ptr, row);
	}

	p = qr->c->data;

	for(y = 0; y < qr->c->width; y++) {
		bit = 7;
		memset(row, 0xff, (realwidth + 7) / 8);
		q = row;
		q += m * s / 8;
		bit = 7 - (m * s % 8);
		for(x = 0; x < qr->c->width; x++) {
			for(xx = 0; xx < s; xx++) {
				*q ^= (*p & 1) << bit;
				bit--;
				if(bit < 0) {
					q++;
					bit = 7;
				}
			}
			p++;
		}

		for(yy=0; yy < s; yy++) {
			png_write_row(png_ptr, row);
		}
	}

	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y < m * s; y++) {
		png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	efree (row);
	efree(palette);
	fseek (fp, 0, SEEK_SET);

	phalcon_ob_start(TSRMLS_C);

	while ((b = fread (buf, 1, sizeof(buf), fp)) > 0) {
		php_write(buf, b TSRMLS_CC);
	}

	phalcon_ob_get_contents(return_value TSRMLS_CC);
	phalcon_ob_end_clean(TSRMLS_C);

	fclose (fp);
	VCWD_UNLINK((const char *)ZSTR_VAL(path));
	zend_string_release(path);
	RETURN_TRUE;
#endif

	RETURN_FALSE;
}

/**
 * Save the image
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $qr->generate('Phalcon is a web framework', 4, \Phalcon\Chart\QRcode::LEVEL_L, \Phalcon\Chart\QRcode::MODE_KANJI, TRUE);
 *     $qr->save('qr.png');
 *
 * @param filename $filename
 * @param size $size
 * @param margin $margin.
 * @return boolean
 */
PHP_METHOD(Phalcon_Chart_QRcode, save){

#if PHALCON_USE_QRENCODE
	zval *filename, *size = NULL, *margin = NULL, *foreground=NULL, *background=NULL;
	zval zid = {}, exception_message = {};
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette;
	png_byte alpha_values[2];
	php_qrcode *qr = NULL;
	FILE *fp = NULL;
	static unsigned int fg_color[4] = {0, 0, 0, 255};
	static unsigned int bg_color[4] = {255, 255, 255, 255};
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;
	char *fn = NULL;
	long s = 3, m = 4;

	phalcon_fetch_params(0, 1, 4, &filename, &size, &margin, &foreground, &background);

	if (Z_TYPE_P(filename) != IS_STRING || PHALCON_IS_EMPTY(filename)) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "filename parameter must be string");
		return;
	}

	if (size && Z_TYPE_P(size) != IS_NULL && Z_TYPE_P(size) != IS_LONG) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "size parameter must be int");
		return;
	}

	if (margin && Z_TYPE_P(margin) != IS_NULL && Z_TYPE_P(margin) != IS_LONG) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "margin parameter must be int");
		return;
	}

	if (foreground && zend_is_true(foreground)) {
		PHALCON_SEPARATE_PARAM(foreground);
		convert_to_string(foreground);

		if(phalcon_color_set(fg_color, Z_STRVAL_P(foreground))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid foreground color value");
			return;
		}
	}

	if (background && zend_is_true(background)) {
		PHALCON_SEPARATE_PARAM(background);
		convert_to_string(background);

		if(phalcon_color_set(bg_color, Z_STRVAL_P(background))) {
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Invalid background color value");
			return;
		}
	}

	if (size && Z_TYPE_P(size) == IS_LONG) {
		s = Z_LVAL_P(size);
	}
	if (margin && Z_TYPE_P(margin) == IS_LONG) {
		m = Z_LVAL_P(margin);
	}

	fn = Z_STRVAL_P(filename);

	phalcon_read_property(&zid, getThis(), SL("_qr"), PH_NOISY);

	if (Z_TYPE(zid) == IS_NULL) {
		RETURN_FALSE;
	}


	if ((qr = (php_qrcode *)zend_fetch_resource(Z_RES(zid), phalcon_qrcode_handle_name, phalcon_qrcode_handle)) == NULL) {
		RETURN_FALSE;
	}

	fp = VCWD_FOPEN(fn, "wb");
	if (!fp) {
		PHALCON_CONCAT_SVS(&exception_message, "Unable to open '", filename, "' for writing");
		PHALCON_THROW_EXCEPTION_ZVAL(phalcon_chart_exception_ce, &exception_message);
		return;
	}

	realwidth = (qr->c->width + m * 2) * s;
	row = (unsigned char *) emalloc ((realwidth + 7) / 8);

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to initialize PNG writer");
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to initialize PNG writer");
		return;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to write PNG writer");
		return;
	}

	palette = (png_colorp) emalloc(sizeof(png_color) * 2);
	palette[0].red   = fg_color[0];
	palette[0].green = fg_color[1];
	palette[0].blue  = fg_color[2];
	palette[1].red   = bg_color[0];
	palette[1].green = bg_color[1];
	palette[1].blue  = bg_color[2];
	alpha_values[0] = fg_color[3];
	alpha_values[1] = bg_color[3];
	png_set_PLTE(png_ptr, info_ptr, palette, 2);
	png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr,
		realwidth, realwidth,
		1,
		PNG_COLOR_TYPE_PALETTE,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_set_pHYs(png_ptr, info_ptr,
		dpi * INCHES_PER_METER,
		dpi * INCHES_PER_METER,
		PNG_RESOLUTION_METER);

	png_write_info(png_ptr, info_ptr);

	memset(row, 0xff, (realwidth + 7) / 8);
	for(y = 0; y < m * s; y++) {
		png_write_row(png_ptr, row);
	}

	p = qr->c->data;
	for(y = 0; y < qr->c->width; y++) {
		bit = 7;
		memset(row, 0xff, (realwidth + 7) / 8);
		q = row;
		q += m * s / 8;
		bit = 7 - (m * s % 8);
		for(x = 0; x < qr->c->width; x++) {
			for(xx = 0; xx < s; xx++) {
				*q ^= (*p & 1) << bit;
				bit--;
				if(bit < 0) {
					q++;
					bit = 7;
				}
			}
			p++;
		}

		for(yy=0; yy < s; yy++) {
			png_write_row(png_ptr, row);
		}
	}

	memset(row, 0xff, (realwidth + 7) / 8);
	for(y=0; y < m * s; y++)
		png_write_row(png_ptr, row);

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	efree (row);
	efree (palette);

	fflush (fp);
	fclose (fp);

	RETURN_TRUE;
#else
	RETURN_FALSE;
#endif
}

#ifdef PHALCON_USE_ZBAR
static zbar_image_t *_php_zbarcode_image_create(unsigned long width, unsigned long height, unsigned char *image_data)
{
	zbar_image_t *image = zbar_image_create();

	if (!image)
		return NULL;

	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, width, height);
	zbar_image_set_data(image, (void *)image_data, width * height, zbar_image_free_data);
	return image;
}

static zbar_image_t *_php_zbarcode_get_page(MagickWand *wand)
{
	unsigned long width, height;
	unsigned char *image_data;

	if (MagickSetImageDepth(wand, 8) == MagickFalse) {
		return NULL;
	}

	if (MagickSetImageFormat(wand, "GRAY") == MagickFalse) {
		return NULL;
	}

	width  = MagickGetImageWidth(wand);
	height = MagickGetImageHeight(wand);

	image_data = emalloc(width * height);

	if (!MagickExportImagePixels(wand, 0, 0, width, height, "I", CharPixel, image_data)) {
		return NULL;
	}

	return _php_zbarcode_image_create(width, height, image_data);
}

static void _php_zbarcode_scan_page(zbar_image_scanner_t *scanner, zbar_image_t *image, zend_bool extended, zval *return_array)
{
	const zbar_symbol_t *symbol;

	array_init(return_array);

	/* scan the image for barcodes */
	zbar_scan_image(scanner, image);

	/* extract results */
	symbol = zbar_image_first_symbol(image);

	/* Loop through all all symbols */
	for(; symbol; symbol = zbar_symbol_next(symbol)) {
		zval fromtext = {}, totext = {}, from = {}, to = {}, symbol_array = {}, loc_array = {}, coords = {};
		zbar_symbol_type_t symbol_type;
		const char *data;
		const char *type;
		int quality;
		unsigned int loc_size, i;

		array_init(&symbol_array);

		/* Get symbol type and data in it */
		symbol_type = zbar_symbol_get_type(symbol);
		data = zbar_symbol_get_data(symbol);
		type = zbar_get_symbol_name(symbol_type);
		quality = zbar_symbol_get_quality(symbol);

        if (phalcon_function_exists_ex(SS("mb_convert_encoding") TSRMLS_CC) == SUCCESS) {
			ZVAL_STRING(&fromtext, data);
			ZVAL_STRING(&from, "shift-jis");
			ZVAL_STRING(&to, "utf-8");
			ZVAL_STRING(&fromtext, data);

			PHALCON_CALL_FUNCTION(&totext, "mb_convert_encoding", &fromtext, &from, &to);
			phalcon_array_update_str(&symbol_array, SL("data"), &totext, PH_COPY);
        } else {
			phalcon_array_update_str_str(&symbol_array, SL("data"), (char *)data, strlen(data), PH_COPY);
		}
		phalcon_array_update_str_str(&symbol_array, SL("type"), (char *)type, strlen(type), PH_COPY);
		phalcon_array_update_str_long(&symbol_array, SL("quality"), quality, 0);

		if (extended) {
			array_init(&loc_array);
			loc_size = zbar_symbol_get_loc_size(symbol);

			for (i = 0; i < loc_size; i++) {
				array_init(&coords);
				phalcon_array_update_str_long(&coords, SL("x"), zbar_symbol_get_loc_x(symbol, i), PH_COPY);
				phalcon_array_update_str_long(&coords, SL("y"), zbar_symbol_get_loc_y(symbol, i), PH_COPY);

				phalcon_array_append(&loc_array, &coords, PH_COPY);
			}
			phalcon_array_update_str(&symbol_array, SL("location"), &loc_array, PH_COPY);
		}
		phalcon_array_append(return_array, &symbol_array, PH_COPY);
	}
}
#endif

/**
 * Scan the image.
 *
 *     $qr = new \Phalcon\Chart\QRcode;
 *     $ret = $qr->san('qr.png');
 *
 * @param string filename
 * @return string
 */
PHP_METHOD(Phalcon_Chart_QRcode, scan){

#if PHALCON_USE_ZBAR
	zval *filename, *enhance = NULL, *extended = NULL;
	MagickWand *magick_wand;
	zbar_image_scanner_t *zbar_scanner;
	zbar_image_t *zbar_page;
	zend_bool ext = 0;
	long i = 1, e = 0, image_count = 0;

	phalcon_fetch_params(0, 1, 3, &filename, &enhance, &extended);

	if (enhance && Z_TYPE_P(enhance) == IS_LONG) {
		e = Z_LVAL_P(enhance);
	}

	if (extended) {
		ext = zend_is_true(extended) ? 1 : 0;
	}

	magick_wand = NewMagickWand();

	if (e & 1) {
		MagickSetResolution(magick_wand, 200, 200);
	}

	if (MagickReadImage(magick_wand, Z_STRVAL_P(filename)) == MagickFalse) {
		ClearMagickWand(magick_wand);
		DestroyMagickWand(magick_wand);
		RETURN_FALSE;
	}

	if (e & 2) {
		MagickEnhanceImage(magick_wand);
	}

	if (e & 4) {
		MagickSharpenImage(magick_wand, 0, 0.5);
	}

	image_count = MagickGetNumberImages(magick_wand);

	if (image_count == 0) {
		PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "The image object does not contain images");
		return;
	}

	zbar_scanner = zbar_image_scanner_create();

	if (image_count == 1) {
		if (MagickSetIteratorIndex(magick_wand, 0) == MagickFalse) {
			zbar_image_scanner_destroy(zbar_scanner);
			DestroyMagickWand(magick_wand);
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to set the page number");
			return;
		}

		/* Read page */
		zbar_page = _php_zbarcode_get_page(magick_wand);

		if (!zbar_page) {
			zbar_image_scanner_destroy(zbar_scanner);
			DestroyMagickWand(magick_wand);
			PHALCON_THROW_EXCEPTION_STR(phalcon_chart_exception_ce, "Failed to get the page");
			return;
		}

		/* Scan the page for barcodes */
		_php_zbarcode_scan_page(zbar_scanner, zbar_page, ext, return_value);
	} else {
		array_init(return_value);

		MagickResetIterator(magick_wand);
		while (MagickNextImage(magick_wand) != MagickFalse) {
			zval page_array = {};

			/* Read the current page */
			zbar_page = _php_zbarcode_get_page(magick_wand);

			/* Reading current page failed */
			if (!zbar_page) {
				i++;
				continue;
			}

			/* Scan the page for barcodes */
			ZVAL_NULL(&page_array);
			_php_zbarcode_scan_page(zbar_scanner, zbar_page, ext, &page_array);
			add_index_zval(return_value, i++, &page_array);
		}
	}

	zbar_image_scanner_destroy(zbar_scanner);
	DestroyMagickWand(magick_wand);
#endif
}
