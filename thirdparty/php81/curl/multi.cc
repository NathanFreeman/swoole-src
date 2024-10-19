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
   +----------------------------------------------------------------------+
*/
#include "php_swoole_cxx.h"
#include "zend_object_handlers.h"

#if defined(SW_USE_CURL) && PHP_VERSION_ID >= 80100
#define ZEND_INCLUDE_FULL_WINDOWS_HEADERS
#include "php_swoole_curl.h"

using swoole::curl::Multi;
using swoole::curl::Selector;

SW_EXTERN_C_BEGIN
#include "curl_interface.h"
#include "curl_arginfo.h"

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <curl/easy.h>

#define SAVE_CURLM_ERROR(__handle, __err) (__handle)->err.no = (int) __err;

void swoole_curl_multi_set_in_coroutine(php_curlm *mh, bool value) {
    zend_update_property_bool(nullptr, &mh->std, ZEND_STRL("in_coroutine"), value);
}

bool swoole_curl_multi_is_in_coroutine(php_curlm *mh) {
    zval rv;
    zval *zv = zend_read_property_ex(nullptr, &mh->std, SW_ZSTR_KNOWN(SW_ZEND_STR_IN_COROUTINE), 1, &rv);
    return zval_is_true(zv);
}

/* CurlMultiHandle class */
zend_class_entry *swoole_coroutine_curl_multi_handle_ce;

static inline php_curlm *curl_multi_from_obj(zend_object *obj) {
    return (php_curlm *) ((char *) (obj) -XtOffsetOf(php_curlm, std));
}

#define Z_CURL_MULTI_P(zv) curl_multi_from_obj(Z_OBJ_P(zv))

static void _php_curl_multi_free(php_curlm *mh);

SW_EXTERN_C_END

/* {{{ Returns a new cURL multi handle */
PHP_FUNCTION(swoole_native_curl_multi_init) {
    php_curlm *mh;

    ZEND_PARSE_PARAMETERS_NONE();

    object_init_ex(return_value, swoole_coroutine_curl_multi_handle_ce);
    mh = Z_CURL_MULTI_P(return_value);
    mh->multi = new Multi();
    mh->multi->set_selector(new Selector());

    swoole_curl_multi_set_in_coroutine(mh, true);
    zend_llist_init(&mh->easyh, sizeof(zval), swoole_curl_multi_cleanup_list, 0);
}
/* }}} */

/* {{{ Add a normal cURL handle to a cURL multi handle */
PHP_FUNCTION(swoole_native_curl_multi_add_handle) {
    zval *z_mh;
    zval *z_ch;
    php_curlm *mh;
    php_curl *ch;
    CURLMcode error = CURLM_OK;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_OBJECT_OF_CLASS(z_ch, swoole_coroutine_curl_handle_ce)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);
    ch = Z_CURL_P(z_ch);

    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

#if PHP_VERSION_ID >= 80200
    swoole_curl_verify_handlers(ch, /* reporterror */ true);
#else
    swoole_curl_verify_handlers(ch, /* reporterror */ 1);
#endif

    swoole_curl_cleanup_handle(ch);

    auto handle = swoole::curl::get_handle(ch->cp);
    error = mh->multi->add_handle(handle);
    SAVE_CURLM_ERROR(mh, error);

#if PHP_VERSION_ID >= 80200
    if (error == CURLM_OK) {
#endif
        Z_ADDREF_P(z_ch);
        zend_llist_add_element(&mh->easyh, z_ch);
#if PHP_VERSION_ID >= 80200
    }
#endif

    swoole_trace_log(SW_TRACE_CO_CURL, "multi=%p, cp=%p, handle=%p, error=%d", mh->multi, ch->cp, handle, error);
    RETURN_LONG((zend_long) error);
}
/* }}} */

void swoole_curl_multi_cleanup_list(void *data) /* {{{ */
{
    zval *z_ch = (zval *) data;

    zval_ptr_dtor(z_ch);
}
/* }}} */

/* Used internally as comparison routine passed to zend_list_del_element */
static int curl_compare_objects(zval *z1, zval *z2) /* {{{ */
{
    return (Z_TYPE_P(z1) == Z_TYPE_P(z2) && Z_TYPE_P(z1) == IS_OBJECT && Z_OBJ_P(z1) == Z_OBJ_P(z2));
}
/* }}} */

/* Used to find the php_curl resource for a given curl easy handle */
static zval *_php_curl_multi_find_easy_handle(php_curlm *mh, CURL *easy) /* {{{ */
{
    php_curl *tmp_ch;
    zend_llist_position pos;
    zval *pz_ch_temp;

    for (pz_ch_temp = (zval *) zend_llist_get_first_ex(&mh->easyh, &pos); pz_ch_temp;
         pz_ch_temp = (zval *) zend_llist_get_next_ex(&mh->easyh, &pos)) {
        tmp_ch = swoole_curl_get_handle(pz_ch_temp, false, false);

        if (tmp_ch->cp == easy) {
            return pz_ch_temp;
        }
    }

    return NULL;
}
/* }}} */

/* {{{ Remove a multi handle from a set of cURL handles */
PHP_FUNCTION(swoole_native_curl_multi_remove_handle) {
    zval *z_mh;
    zval *z_ch;
    php_curlm *mh;
    php_curl *ch;
    CURLMcode error = CURLM_OK;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_OBJECT_OF_CLASS(z_ch, swoole_coroutine_curl_handle_ce)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);
    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

    ch = Z_CURL_P(z_ch);
    auto handle = swoole::curl::get_handle(ch->cp);
    if (handle && handle->multi) {
        error = mh->multi->remove_handle(handle);
    } else {
        error = curl_multi_remove_handle(mh->multi, ch->cp);
    }

    swoole_trace_log(SW_TRACE_CO_CURL, "multi=%p, cp=%p, handle=%p, error=%d", mh->multi, ch->cp, handle, error);
    SAVE_CURLM_ERROR(mh, error);

#if PHP_VERSION_ID >= 80200
    if (error == CURLM_OK) {
#endif
        zend_llist_del_element(&mh->easyh, z_ch, (int (*)(void *, void *)) curl_compare_objects);
#if PHP_VERSION_ID >= 80200
    }
#endif

    RETURN_LONG((zend_long) error);
}
/* }}} */

/* {{{ Get all the sockets associated with the cURL extension, which can then be "selected" */
PHP_FUNCTION(swoole_native_curl_multi_select) {
    zval *z_mh;
    php_curlm *mh;
    double timeout = 1.0;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_OPTIONAL
    Z_PARAM_DOUBLE(timeout)
    ZEND_PARSE_PARAMETERS_END();

#if PHP_VERSION_ID >= 80200
    if (!(timeout >= 0.0 && timeout <= ((double) INT_MAX / 1000.0))) {
        php_error_docref(NULL, E_WARNING, "timeout must be between 0 and %d", (int) ceilf((double) INT_MAX / 1000));
#ifdef CURLM_BAD_FUNCTION_ARGUMENT
        SAVE_CURLM_ERROR(mh, CURLM_BAD_FUNCTION_ARGUMENT);
#endif
        RETURN_LONG(-1);
    }
#endif

    mh = Z_CURL_MULTI_P(z_mh);
    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

    RETURN_LONG(mh->multi->select(mh, timeout));
}
/* }}} */

/* {{{ Run the sub-connections of the current cURL handle */
PHP_FUNCTION(swoole_native_curl_multi_exec) {
    zval *z_mh;
    zval *z_still_running;
    php_curlm *mh;
    int still_running;
    CURLMcode error = CURLM_OK;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_ZVAL(z_still_running)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);
    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

    {
        zend_llist_position pos;
        php_curl *ch;
        zval *pz_ch;

        for (pz_ch = (zval *) zend_llist_get_first_ex(&mh->easyh, &pos); pz_ch;
             pz_ch = (zval *) zend_llist_get_next_ex(&mh->easyh, &pos)) {
            ch = Z_CURL_P(pz_ch);

#if PHP_VERSION_ID >= 80200
            swoole_curl_verify_handlers(ch, true);
#else
            swoole_curl_verify_handlers(ch, 1);
#endif
        }
    }

    still_running = zval_get_long(z_still_running);
    error = curl_multi_perform(mh->multi, &still_running);
    ZEND_TRY_ASSIGN_REF_LONG(z_still_running, still_running);

    SAVE_CURLM_ERROR(mh, error);
    RETURN_LONG((zend_long) error);
}
/* }}} */

/* {{{ Return the content of a cURL handle if CURLOPT_RETURNTRANSFER is set */
PHP_FUNCTION(swoole_native_curl_multi_getcontent) {
    zval *z_ch;
    php_curl *ch;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(z_ch, swoole_coroutine_curl_handle_ce)
    ZEND_PARSE_PARAMETERS_END();

    ch = Z_CURL_P(z_ch);

    if (ch->handlers.write->method == PHP_CURL_RETURN) {
        if (!ch->handlers.write->buf.s) {
            RETURN_EMPTY_STRING();
        }
        smart_str_0(&ch->handlers.write->buf);
        RETURN_STR_COPY(ch->handlers.write->buf.s);
    }

    RETURN_NULL();
}
/* }}} */

/* {{{ Get information about the current transfers */
PHP_FUNCTION(swoole_native_curl_multi_info_read) {
    zval *z_mh;
    php_curlm *mh;
    CURLMsg *tmp_msg;
    int queued_msgs;
    zval *zmsgs_in_queue = NULL;
    php_curl *ch;

    ZEND_PARSE_PARAMETERS_START(1, 2)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_OPTIONAL
    Z_PARAM_ZVAL(zmsgs_in_queue)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);
    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

    tmp_msg = curl_multi_info_read(mh->multi, &queued_msgs);
    if (tmp_msg == NULL) {
        RETURN_FALSE;
    }

    if (zmsgs_in_queue) {
        ZEND_TRY_ASSIGN_REF_LONG(zmsgs_in_queue, queued_msgs);
    }

    array_init(return_value);
    add_assoc_long(return_value, "msg", tmp_msg->msg);
    add_assoc_long(return_value, "result", tmp_msg->data.result);

    /* find the original easy curl handle */
    {
        zval *pz_ch = _php_curl_multi_find_easy_handle(mh, tmp_msg->easy_handle);
        if (pz_ch != NULL) {
            /* we must save result to be able to read error message */
            ch = swoole_curl_get_handle(pz_ch, false, false);
            SAVE_CURL_ERROR(ch, tmp_msg->data.result);

            Z_ADDREF_P(pz_ch);
            add_assoc_zval(return_value, "handle", pz_ch);
        }
    }
}
/* }}} */

/* {{{ Close a set of cURL handles */
PHP_FUNCTION(swoole_native_curl_multi_close) {
    php_curlm *mh;
    zval *z_mh;

    zend_llist_position pos;
    zval *pz_ch;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);

    for (pz_ch = (zval *) zend_llist_get_first_ex(&mh->easyh, &pos); pz_ch;
         pz_ch = (zval *) zend_llist_get_next_ex(&mh->easyh, &pos)) {
        php_curl *ch = Z_CURL_P(pz_ch);
        if (!ch) {
            continue;
        }

#if PHP_VERSION_ID >= 80200
        swoole_curl_verify_handlers(ch, true);
#else
        swoole_curl_verify_handlers(ch, 1);
#endif

        auto handle = swoole::curl::get_handle(ch->cp);
        if (handle) {
            mh->multi->remove_handle(handle);
        } else {
            curl_multi_remove_handle(mh->multi, ch->cp);
        }
    }
    zend_llist_clean(&mh->easyh);
}
/* }}} */

/* {{{ Return an integer containing the last multi curl error number */
PHP_FUNCTION(swoole_native_curl_multi_errno) {
    zval *z_mh;
    php_curlm *mh;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);

    RETURN_LONG(mh->err.no);
}
/* }}} */

/* {{{ return string describing error code */
PHP_FUNCTION(swoole_native_curl_multi_strerror) {
    zend_long code;
    const char *str;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(code)
    ZEND_PARSE_PARAMETERS_END();

    str = curl_multi_strerror((CURLMcode) code);
    if (str) {
        RETURN_STRING(str);
    } else {
        RETURN_NULL();
    }
}
/* }}} */

#if LIBCURL_VERSION_NUM >= 0x072C00 /* Available since 7.44.0 */

static int _php_server_push_callback(
    CURL *parent_ch, CURL *easy, size_t num_headers, struct curl_pushheaders *push_headers, void *userp) /* {{{ */
{
    php_curl *ch;
    php_curl *parent;
    php_curlm *mh = (php_curlm *) userp;
    size_t rval = CURL_PUSH_DENY;
    php_curl_callback *t = mh->handlers.server_push;
    zval *pz_parent_ch = NULL;
    zval pz_ch;
    zval headers;
    zval retval;
    char *header;
    curl_result_t error;
    zend_fcall_info fci = empty_fcall_info;

    pz_parent_ch = _php_curl_multi_find_easy_handle(mh, parent_ch);
    if (pz_parent_ch == NULL) {
        return rval;
    }

    if (UNEXPECTED(zend_fcall_info_init(&t->func_name, 0, &fci, &t->fci_cache, NULL, NULL) == FAILURE)) {
        php_error_docref(NULL, E_WARNING, "Cannot call the CURLMOPT_PUSHFUNCTION");
        return rval;
    }

    parent = Z_CURL_P(pz_parent_ch);

    ch = swoole_curl_init_handle_into_zval(&pz_ch);
    ch->cp = easy;
    swoole_setup_easy_copy_handlers(ch, parent);

    auto parent_handle = swoole::curl::get_handle(parent->cp);
    if (parent_handle) {
        auto handle = swoole::curl::create_handle(easy);
        handle->multi = parent_handle->multi;
    }

    size_t i;
    array_init(&headers);
    for (i = 0; i < num_headers; i++) {
        header = curl_pushheader_bynum(push_headers, i);
        add_next_index_string(&headers, header);
    }

#if PHP_VERSION_ID >= 80300
    ZEND_ASSERT(pz_parent_ch);
    zval call_args[3] = {*pz_parent_ch, pz_ch, headers};
#else
    zend_fcall_info_argn(&fci, 3, pz_parent_ch, &pz_ch, &headers);
#endif

#if PHP_VERSION_ID >= 80300
    fci.param_count = 3;
    fci.params = call_args;
#endif

    fci.retval = &retval;

    error = zend_call_function(&fci, &t->fci_cache);

#if PHP_VERSION_ID < 80300
    zend_fcall_info_args_clear(&fci, 1);
#endif

    zval_ptr_dtor_nogc(&headers);

    if (error == FAILURE) {
        php_error_docref(NULL, E_WARNING, "Cannot call the CURLMOPT_PUSHFUNCTION");
    } else if (!Z_ISUNDEF(retval)) {
        if (CURL_PUSH_DENY != zval_get_long(&retval)) {
            rval = CURL_PUSH_OK;
            zend_llist_add_element(&mh->easyh, &pz_ch);
        } else {
            /* libcurl will free this easy handle, avoid double free */
            ch->cp = NULL;
        }
    }

    return rval;
}
/* }}} */

#endif

static curl_bool_t _php_curl_multi_setopt(php_curlm *mh, zend_long option, zval *zvalue, zval *return_value) /* {{{ */
{
    CURLMcode error = CURLM_OK;

    switch (option) {
    case CURLMOPT_PIPELINING:
    case CURLMOPT_MAXCONNECTS:
#if LIBCURL_VERSION_NUM >= 0x071e00 /* 7.30.0 */
    case CURLMOPT_CHUNK_LENGTH_PENALTY_SIZE:
    case CURLMOPT_CONTENT_LENGTH_PENALTY_SIZE:
    case CURLMOPT_MAX_HOST_CONNECTIONS:
    case CURLMOPT_MAX_PIPELINE_LENGTH:
    case CURLMOPT_MAX_TOTAL_CONNECTIONS:
#endif
#if LIBCURL_VERSION_NUM >= 0x074300 && PHP_VERSION_ID >= 80200 /* Available since 7.67.0 */
    case CURLMOPT_MAX_CONCURRENT_STREAMS:
#endif
    {
        zend_long lval = zval_get_long(zvalue);

        if (option == CURLMOPT_PIPELINING && (lval & 1)) {
#if LIBCURL_VERSION_NUM >= 0x073e00 /* 7.62.0 */
            php_error_docref(NULL, E_WARNING, "CURLPIPE_HTTP1 is no longer supported");
#else
            php_error_docref(NULL, E_DEPRECATED, "CURLPIPE_HTTP1 is deprecated");
#endif
        }
        error = curl_multi_setopt(mh->multi, (CURLMoption) option, lval);
        break;
    }
#if LIBCURL_VERSION_NUM > 0x072D00 /* Available since 7.45.0 */
    case CURLMOPT_PUSHFUNCTION:
        if (mh->handlers.server_push == NULL) {
            mh->handlers.server_push = (php_curl_callback *) ecalloc(1, sizeof(php_curl_callback));
        } else if (!Z_ISUNDEF(mh->handlers.server_push->func_name)) {
            zval_ptr_dtor(&mh->handlers.server_push->func_name);
            mh->handlers.server_push->fci_cache = empty_fcall_info_cache;
        }

        ZVAL_COPY(&mh->handlers.server_push->func_name, zvalue);

#if PHP_VERSION_ID >= 80200
        error = curl_multi_setopt(mh->multi, CURLMOPT_PUSHFUNCTION, _php_server_push_callback);
#else
        error = curl_multi_setopt(mh->multi, option, _php_server_push_callback);
#endif

        if (error != CURLM_OK) {
#if PHP_VERSION_ID >= 80200
            return false;
#else
            return 0;
#endif
        }
        error = curl_multi_setopt(mh->multi, CURLMOPT_PUSHDATA, mh);
        break;
#endif
    default:
        zend_argument_value_error(2, "is not a valid cURL multi option");
        error = CURLM_UNKNOWN_OPTION;
        break;
    }

    SAVE_CURLM_ERROR(mh, error);

#if PHP_VERSION_ID >= 80200
    return error == CURLM_OK;
#else
    return error != CURLM_OK;
#endif
}
/* }}} */

/* {{{ Set an option for the curl multi handle */
PHP_FUNCTION(swoole_native_curl_multi_setopt) {
    zval *z_mh, *zvalue;
    zend_long options;
    php_curlm *mh;

    ZEND_PARSE_PARAMETERS_START(3, 3)
    Z_PARAM_OBJECT_OF_CLASS(z_mh, swoole_coroutine_curl_multi_handle_ce)
    Z_PARAM_LONG(options)
    Z_PARAM_ZVAL(zvalue)
    ZEND_PARSE_PARAMETERS_END();

    mh = Z_CURL_MULTI_P(z_mh);
    if (!(swoole_curl_multi_is_in_coroutine(mh))) {
        swoole_fatal_error(SW_ERROR_WRONG_OPERATION,
                           "The given object is not a valid coroutine CurlMultiHandle object");
        RETURN_FALSE;
    }

#if PHP_VERSION_ID >= 80200
    if (_php_curl_multi_setopt(mh, options, zvalue, return_value)) {
#else
    if (!_php_curl_multi_setopt(mh, options, zvalue, return_value)) {
#endif
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}
/* }}} */

/* CurlMultiHandle class */

static zend_object_handlers swoole_coroutine_curl_multi_handle_handlers;

static zend_object *swoole_curl_multi_create_object(zend_class_entry *class_type) {
    php_curlm *intern = (php_curlm *) zend_object_alloc(sizeof(php_curlm), class_type);

    zend_object_std_init(&intern->std, class_type);
    object_properties_init(&intern->std, class_type);

#if PHP_VERSION_ID < 800200
    intern->std.handlers = &swoole_coroutine_curl_multi_handle_handlers;
#endif

    return &intern->std;
}

static zend_function *swoole_curl_multi_get_constructor(zend_object *object) {
    zend_throw_error(NULL, "Cannot directly construct CurlMultiHandle, use curl_multi_init() instead");
    return NULL;
}

static void swoole_curl_multi_free_obj(zend_object *object) {
    php_curlm *mh = curl_multi_from_obj(object);

    if (!mh->multi) {
        /* Can happen if constructor throws. */
        zend_object_std_dtor(&mh->std);
        return;
    }

    _php_curl_multi_free(mh);
    zend_object_std_dtor(&mh->std);
}

static HashTable *swoole_curl_multi_get_gc(zend_object *object, zval **table, int *n) {
    php_curlm *curl_multi = curl_multi_from_obj(object);

    zend_get_gc_buffer *gc_buffer = zend_get_gc_buffer_create();

    if (curl_multi->handlers.server_push) {
        zend_get_gc_buffer_add_zval(gc_buffer, &curl_multi->handlers.server_push->func_name);
    }

    zend_llist_position pos;
    for (zval *pz_ch = (zval *) zend_llist_get_first_ex(&curl_multi->easyh, &pos); pz_ch;
         pz_ch = (zval *) zend_llist_get_next_ex(&curl_multi->easyh, &pos)) {
        zend_get_gc_buffer_add_zval(gc_buffer, pz_ch);
    }

    zend_get_gc_buffer_use(gc_buffer, table, n);

    return zend_std_get_properties(object);
}

void curl_multi_register_handlers(void) {
    swoole_coroutine_curl_multi_handle_ce = curl_multi_ce;
    swoole_coroutine_curl_multi_handle_ce->create_object = swoole_curl_multi_create_object;

#if PHP_VERSION_ID >= 80300
    swoole_coroutine_curl_multi_handle_ce->default_object_handlers = &swoole_coroutine_curl_multi_handle_handlers;
#endif

    memcpy(&swoole_coroutine_curl_multi_handle_handlers, &std_object_handlers, sizeof(zend_object_handlers));
    swoole_coroutine_curl_multi_handle_handlers.offset = XtOffsetOf(php_curlm, std);
    swoole_coroutine_curl_multi_handle_handlers.free_obj = swoole_curl_multi_free_obj;
    swoole_coroutine_curl_multi_handle_handlers.get_gc = swoole_curl_multi_get_gc;
    swoole_coroutine_curl_multi_handle_handlers.get_constructor = swoole_curl_multi_get_constructor;
    swoole_coroutine_curl_multi_handle_handlers.clone_obj = NULL;
    swoole_coroutine_curl_multi_handle_handlers.cast_object = swoole_curl_cast_object;
    swoole_coroutine_curl_multi_handle_handlers.compare = [](zval *o1, zval *o2) { return ZEND_UNCOMPARABLE; };

    zend_declare_property_bool(swoole_coroutine_curl_multi_handle_ce, ZEND_STRL("in_coroutine"), 0, ZEND_ACC_PUBLIC);
}

static void _php_curl_multi_free(php_curlm *mh) {
    bool is_in_coroutine = swoole_curl_multi_is_in_coroutine(mh);
    for (zend_llist_element *element = mh->easyh.head; element; element = element->next) {
        zval *z_ch = (zval *) element->data;
        php_curl *ch;
        if (OBJ_FLAGS(Z_OBJ_P(z_ch)) & IS_OBJ_FREE_CALLED) {
            continue;
        }
        if ((ch = swoole_curl_get_handle(z_ch, true, false))) {
#if PHP_VERSION_ID >= 80200
            swoole_curl_verify_handlers(ch, false);
#else
            swoole_curl_verify_handlers(ch, 0);
#endif

            auto handle = swoole::curl::get_handle(ch->cp);
            if (is_in_coroutine && handle) {
                mh->multi->remove_handle(handle);
            } else {
                curl_multi_remove_handle(mh->multi, ch->cp);
            }
        }
    }
    if (mh->multi) {
        if (is_in_coroutine) {
            delete mh->multi;
        } else {
            curl_multi_cleanup(mh->multi);
        }
        mh->multi = nullptr;
    }
    zend_llist_clean(&mh->easyh);
    if (mh->handlers.server_push) {
        zval_ptr_dtor(&mh->handlers.server_push->func_name);
        efree(mh->handlers.server_push);
    }
}
#endif
