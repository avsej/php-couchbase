/**
 *     Copyright 2016-2019 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/**
 * A FTS query that queries fields explicitly indexed as boolean.
 */
#include "couchbase.h"

typedef struct {

    double boost;
    char *field;
    zend_bool value;
    zend_object std;
} pcbc_boolean_field_search_query_t;

static inline pcbc_boolean_field_search_query_t *pcbc_boolean_field_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_boolean_field_search_query_t *)((char *)obj - XtOffsetOf(pcbc_boolean_field_search_query_t, std));
}
#define Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ(zo) (pcbc_boolean_field_search_query_fetch_object(zo))
#define Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(zv) (pcbc_boolean_field_search_query_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/boolean_field_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_boolean_field_search_query_ce;

/* {{{ proto void BooleanFieldSearchQuery::__construct() */
PHP_METHOD(BooleanFieldSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\BooleanFieldSearchQuery BooleanFieldSearchQuery::field(string $field)
 */
PHP_METHOD(BooleanFieldSearchQuery, field)
{
    pcbc_boolean_field_search_query_t *obj;
    char *field = NULL;
    int rv;
    size_t field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\BooleanFieldSearchQuery BooleanFieldSearchQuery::boost(double $boost)
 */
PHP_METHOD(BooleanFieldSearchQuery, boost)
{
    pcbc_boolean_field_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array BooleanFieldSearchQuery::jsonSerialize()
 */
PHP_METHOD(BooleanFieldSearchQuery, jsonSerialize)
{
    pcbc_boolean_field_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_BOOL_EX(return_value, "bool", obj->value);
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_BooleanFieldSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BooleanFieldSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BooleanFieldSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry boolean_field_search_query_methods[] = {
    PHP_ME(BooleanFieldSearchQuery, __construct, ai_BooleanFieldSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(BooleanFieldSearchQuery, jsonSerialize, ai_BooleanFieldSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(BooleanFieldSearchQuery, boost, ai_BooleanFieldSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(BooleanFieldSearchQuery, field, ai_BooleanFieldSearchQuery_field, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_boolean_field_search_query_init(zval *return_value, zend_bool value TSRMLS_DC)
{
    pcbc_boolean_field_search_query_t *obj;

    object_init_ex(return_value, pcbc_boolean_field_search_query_ce);
    obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->value = value;
    obj->field = NULL;
}

zend_object_handlers boolean_field_search_query_handlers;

static void boolean_field_search_query_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_boolean_field_search_query_t *obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *boolean_field_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_boolean_field_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_boolean_field_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &boolean_field_search_query_handlers;
    return &obj->std;
}

static HashTable *pcbc_boolean_field_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_boolean_field_search_query_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_BOOLEAN_FIELD_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_BOOL_EX(&retval, "bool", obj->value);
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(BooleanFieldSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BooleanFieldSearchQuery", boolean_field_search_query_methods);
    pcbc_boolean_field_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_boolean_field_search_query_ce->create_object = boolean_field_search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_boolean_field_search_query_ce);

    zend_class_implements(pcbc_boolean_field_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_boolean_field_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&boolean_field_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    boolean_field_search_query_handlers.get_debug_info = pcbc_boolean_field_search_query_get_debug_info;
    boolean_field_search_query_handlers.free_obj = boolean_field_search_query_free_object;
    boolean_field_search_query_handlers.offset = XtOffsetOf(pcbc_boolean_field_search_query_t, std);

    zend_register_class_alias("\\CouchbaseBooleanFieldSearchQuery", pcbc_boolean_field_search_query_ce);
    return SUCCESS;
}
