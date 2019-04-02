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

#include "couchbase.h"

typedef struct {
    opcookie_res header;
    char *key;
    int key_len;
} opcookie_durability_res;

void durability_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_durability_resp_t *resp)
{
    opcookie_durability_res *result = ecalloc(1, sizeof(opcookie_durability_res));
    TSRMLS_FETCH();

    result->header.err = error;
    if (resp->v.v0.key) {
        result->key = estrndup(resp->v.v0.key, resp->v.v0.nkey);
    }

    opcookie_push((opcookie *)cookie, &result->header);
}

static lcb_error_t proc_durability_results(pcbc_bucket_t *bucket, zval *return_value, opcookie *cookie,
                                           int is_mapped TSRMLS_DC)
{
    opcookie_durability_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // If we are not mapped, we need to throw any op errors
    if (is_mapped == 0) {
        err = opcookie_get_first_error(cookie);
    }

    if (err == LCB_SUCCESS) {
        FOREACH_OPCOOKIE_RES(opcookie_durability_res, res, cookie)
        {
            zval *doc = bop_get_return_doc(return_value, res->key, res->key_len, is_mapped TSRMLS_CC);

            if (res->header.err == LCB_SUCCESS) {
                pcbc_document_init(doc, bucket, NULL, 0, 0, 0, NULL TSRMLS_CC);
            } else {
                pcbc_document_init_error(doc, &res->header TSRMLS_CC);
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_durability_res, res, cookie)
    {
        if (res->key) {
            efree(res->key);
        }
    }

    return err;
}

PHP_METHOD(Bucket, durability)
{
    pcbc_bucket_t *obj = Z_BUCKET_OBJ_P(getThis());
    lcb_durability_cmd_t *cmd = NULL;
    lcb_durability_opts_t opts = {0};
    lcb_durability_cmd_t **cmds = NULL;
    int ii, num_cmds;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zcas, *zgroupid, *zpersist, *zreplica;
    opcookie *cookie;
    lcb_error_t err;

    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state, "id||cas,groupid,persist_to,replicate_to", &id, &zcas,
                      &zgroupid, &zpersist, &zreplica) != SUCCESS) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    num_cmds = pcbc_pp_keycount(&pp_state);
    cmd = emalloc(sizeof(lcb_durability_cmd_t) * num_cmds);
    cmds = emalloc(sizeof(lcb_durability_cmd_t *) * num_cmds);
    memset(cmd, 0, sizeof(lcb_durability_cmd_t) * num_cmds);

    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        PCBC_CHECK_ZVAL_STRING(zcas, "cas must be a string");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");
        PCBC_CHECK_ZVAL_LONG(zpersist, "persist_to must be an integer");
        PCBC_CHECK_ZVAL_LONG(zreplica, "replicate_to must be an integer");

        cmd[ii].version = 0;
        cmd[ii].v.v0.key = id.str;
        cmd[ii].v.v0.nkey = id.len;
        if (zcas) {
            cmd[ii].v.v0.cas = pcbc_cas_decode(zcas TSRMLS_CC);
        }
        if (zgroupid) {
            cmd[ii].v.v0.hashkey = Z_STRVAL_P(zgroupid);
            cmd[ii].v.v0.nhashkey = Z_STRLEN_P(zgroupid);
        }

        // These are written through each iteration, but only used once.
        if (zpersist) {
            opts.v.v0.persist_to = (lcb_U16)Z_LVAL_P(zpersist);
        }
        if (zreplica) {
            opts.v.v0.replicate_to = (lcb_U16)Z_LVAL_P(zreplica);
        }

        cmds[ii] = &cmd[ii];
    }

    cookie = opcookie_init();

    err = lcb_durability_poll(obj->conn->lcb, cookie, &opts, num_cmds, (const lcb_durability_cmd_t *const *)cmds);

    if (err == LCB_SUCCESS) {
        lcb_wait(obj->conn->lcb);

        err = proc_durability_results(obj, return_value, cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);
    efree(cmds);
    efree(cmd);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
