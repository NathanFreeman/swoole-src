/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Sterling Hughes <sterling@php.net>                           |
   |         Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
*/

#if defined(SW_USE_CURL) && PHP_VERSION_ID >= 80100
#ifndef _PHP_CURL_PRIVATE_H
#define _PHP_CURL_PRIVATE_H

#include "php_curl.h"

#define PHP_CURL_DEBUG 0

#include "php_version.h"
#define PHP_CURL_VERSION PHP_VERSION

#include <curl/curl.h>
#include <curl/multi.h>

#define CURLOPT_RETURNTRANSFER 19913
#define CURLOPT_BINARYTRANSFER 19914 /* For Backward compatibility */
#define PHP_CURL_STDOUT 0
#define PHP_CURL_FILE 1
#define PHP_CURL_USER 2
#define PHP_CURL_DIRECT 3
#define PHP_CURL_RETURN 4
#define PHP_CURL_IGNORE 7

#define SAVE_CURL_ERROR(__handle, __err)                                                                               \
    do {                                                                                                               \
        (__handle)->err.no = (int) __err;                                                                              \
    } while (0)

PHP_MINIT_FUNCTION(curl);
PHP_MSHUTDOWN_FUNCTION(curl);
PHP_MINFO_FUNCTION(curl);

typedef struct {
    zval func_name;
    zend_fcall_info_cache fci_cache;
    FILE *fp;
    smart_str buf;
    int method;
    zval stream;
} php_curl_write;

typedef struct {
    zval func_name;
    zend_fcall_info_cache fci_cache;
    FILE *fp;
    zend_resource *res;
    int method;
    zval stream;
} php_curl_read;

typedef struct {
    zval func_name;
    zend_fcall_info_cache fci_cache;
} php_curl_callback;

typedef struct {
    php_curl_write *write;
    php_curl_write *write_header;
    php_curl_read *read;
    zval std_err;
    php_curl_callback *progress;

#if LIBCURL_VERSION_NUM >= 0x072000 && PHP_VERSION_ID >= 80200
    php_curl_callback *xferinfo;
#endif

#if LIBCURL_VERSION_NUM >= 0x071500
    php_curl_callback *fnmatch;
#endif

#if LIBCURL_VERSION_NUM >= 0x075400 && PHP_VERSION_ID >= 80300
    php_curl_callback *sshhostkey;
#endif

} php_curl_handlers;

struct _php_curl_error {
    char str[CURL_ERROR_SIZE + 1];
    int no;
};

struct _php_curl_send_headers {
    zend_string *str;
};

struct _php_curl_free {
    zend_llist post;
    zend_llist stream;
#if LIBCURL_VERSION_NUM < 0x073800 /* 7.56.0 */
    zend_llist buffers;
#endif
    HashTable *slist;
};

typedef struct {
    CURL *cp;
    php_curl_handlers handlers;
    struct _php_curl_free *to_free;
    struct _php_curl_send_headers header;
    struct _php_curl_error err;
    bool in_callback;
    uint32_t *clone;
    zval postfields;
    /* For CURLOPT_PRIVATE */
    zval private_data;
    /* CurlShareHandle object set using CURLOPT_SHARE. */
    struct _php_curlsh *share;
    zend_object std;
} php_curl;

#define CURLOPT_SAFE_UPLOAD -1

typedef struct {
    php_curl_callback *server_push;
} php_curlm_handlers;

namespace swoole {
namespace curl {
class Multi;
}
}  // namespace swoole

using swoole::curl::Multi;

typedef struct {
    Multi *multi;
    zend_llist easyh;
    php_curlm_handlers handlers;
    struct {
        int no;
    } err;
    zend_object std;
} php_curlm;

typedef struct _php_curlsh {
    CURLSH *share;
    struct {
        int no;
    } err;
    zend_object std;
} php_curlsh;

#if PHP_VERSION_ID >= 80200
typedef zend_result curl_result_t;
typedef bool curl_bool_t;
#else
typedef int curl_result_t;
typedef int curl_bool_t;
#endif

php_curl *swoole_curl_init_handle_into_zval(zval *curl);
void swoole_curl_init_handle(php_curl *ch);
void swoole_curl_cleanup_handle(php_curl *);
void swoole_curl_multi_cleanup_list(void *data);
void swoole_curl_verify_handlers(php_curl *ch, curl_bool_t reporterror);
void swoole_setup_easy_copy_handlers(php_curl *ch, php_curl *source);

static inline php_curl *curl_from_obj(zend_object *obj) {
    return (php_curl *) ((char *) (obj) -XtOffsetOf(php_curl, std));
}

#define Z_CURL_P(zv) curl_from_obj(Z_OBJ_P(zv))

static inline php_curlsh *curl_share_from_obj(zend_object *obj) {
    return (php_curlsh *) ((char *) (obj) -XtOffsetOf(php_curlsh, std));
}

#define Z_CURL_SHARE_P(zv) curl_share_from_obj(Z_OBJ_P(zv))
void curl_multi_register_handlers(void);
curl_result_t swoole_curl_cast_object(zend_object *obj, zval *result, int type);

php_curl *swoole_curl_get_handle(zval *zid, bool exclusive = true, bool required = true);

#endif /* _PHP_CURL_PRIVATE_H */
#endif
