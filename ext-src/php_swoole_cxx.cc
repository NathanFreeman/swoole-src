#include "php_swoole_cxx.h"

//----------------------------------known string------------------------------------

static const char *sw_known_strings[] = {
#define _SW_ZEND_STR_DSC(id, str) str,
    SW_ZEND_KNOWN_STRINGS(_SW_ZEND_STR_DSC)
#undef _SW_ZEND_STR_DSC
        nullptr};

SW_API zend_string **sw_zend_known_strings = nullptr;

SW_API zend_refcounted *sw_refcount_ptr;

zend_refcounted *sw_get_refcount_ptr(zval *value) {
    return (sw_refcount_ptr = value->value.counted);
}

//----------------------------------known string------------------------------------
namespace zend {
void known_strings_init(void) {
    zend_string *str;
    sw_zend_known_strings = nullptr;

    /* known strings */
    sw_zend_known_strings = (zend_string **) pemalloc(
        sizeof(zend_string *) * ((sizeof(sw_known_strings) / sizeof(sw_known_strings[0]) - 1)), 1);
    for (unsigned int i = 0; i < (sizeof(sw_known_strings) / sizeof(sw_known_strings[0])) - 1; i++) {
        str = zend_string_init(sw_known_strings[i], strlen(sw_known_strings[i]), 1);
        sw_zend_known_strings[i] = zend_new_interned_string(str);
    }
}

void known_strings_dtor(void) {
    pefree(sw_zend_known_strings, 1);
    sw_zend_known_strings = nullptr;
}

namespace function {

bool call(zend_fcall_info_cache *fci_cache, uint32_t argc, zval *argv, zval *retval, const bool enable_coroutine) {
    bool success;
    if (enable_coroutine) {
        if (retval) {
            /* the coroutine has no return value */
            ZVAL_NULL(retval);
        }
        success = swoole::PHPCoroutine::create(fci_cache, argc, argv, nullptr) >= 0;
    } else {
        success = sw_zend_call_function_ex(nullptr, fci_cache, argc, argv, retval) == SUCCESS;
    }
    /* we have no chance to return to ZendVM to check the exception  */
    if (UNEXPECTED(EG(exception))) {
        zend_exception_error(EG(exception), E_ERROR);
    }
    return success;
}

Variable call(const std::string &func_name, int argc, zval *argv) {
    zval function_name;
    ZVAL_STRINGL(&function_name, func_name.c_str(), func_name.length());
    Variable retval;
    if (call_user_function(EG(function_table), NULL, &function_name, &retval.value, argc, argv) != SUCCESS) {
        ZVAL_NULL(&retval.value);
    }
    zval_dtor(&function_name);
    /* we have no chance to return to ZendVM to check the exception  */
    if (UNEXPECTED(EG(exception))) {
        zend_exception_error(EG(exception), E_ERROR);
    }
    return retval;
}

}  // namespace function

Callable::Callable(zval *_zfn) {
    ZVAL_UNDEF(&zfn);
    if (!zval_is_true(_zfn)) {
        php_swoole_fatal_error(E_WARNING, "illegal callback function");
        return;
    }
    if (!sw_zend_is_callable_ex(_zfn, nullptr, 0, &fn_name, nullptr, &fcc, nullptr)) {
        php_swoole_fatal_error(E_WARNING, "function '%s' is not callable", fn_name);
        return;
    }
    zfn = *_zfn;
    zval_add_ref(&zfn);
}

Callable::~Callable() {
    if (!ZVAL_IS_UNDEF(&zfn)) {
        zval_ptr_dtor(&zfn);
    }
    if (fn_name) {
        efree(fn_name);
    }
}

uint32_t Callable::refcount() {
    return zval_refcount_p(&zfn);
}
}  // namespace zend
