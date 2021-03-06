/* Copyright 2015 greenbytes GmbH (https://www.greenbytes.de)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __mod_h2__h2_util__
#define __mod_h2__h2_util__

struct nghttp2_frame;

size_t h2_util_hex_dump(char *buffer, size_t maxlen,
                        const char *data, size_t datalen);

size_t h2_util_header_print(char *buffer, size_t maxlen,
                            const char *name, size_t namelen,
                            const char *value, size_t valuelen);

char *h2_strlwr(char *s);

void h2_util_camel_case_header(char *s, size_t len);

/**
 * Return != 0 iff the string s contains the token, as specified in
 * HTTP header syntax, rfc7230.
 */
int h2_util_contains_token(apr_pool_t *pool, const char *s, const char *token);

const char *h2_util_first_token_match(apr_pool_t *pool, const char *s, 
                                      const char *tokens[], apr_size_t len);

/**
 * I always wanted to write my own base64url decoder...not. See 
 * https://tools.ietf.org/html/rfc4648#section-5 for description.
 */
apr_size_t h2_util_base64url_decode(const char **decoded, 
                                    const char *encoded, 
                                    apr_pool_t *pool);

#define H2_HD_MATCH_LIT(l, name, nlen)  \
    ((nlen == sizeof(l) - 1) && !apr_strnatcasecmp(l, name))

#define H2_HD_MATCH_LIT_CS(l, name)  \
    ((strlen(name) == sizeof(l) - 1) && !apr_strnatcasecmp(l, name))

#define H2_CREATE_NV_LIT_CS(nv, NAME, VALUE) nv->name = (uint8_t *)NAME;      \
                                             nv->namelen = sizeof(NAME) - 1;  \
                                             nv->value = (uint8_t *)VALUE;    \
                                             nv->valuelen = strlen(VALUE)

#define H2_CREATE_NV_CS_LIT(nv, NAME, VALUE) nv->name = (uint8_t *)NAME;      \
                                             nv->namelen = strlen(NAME);      \
                                             nv->value = (uint8_t *)VALUE;    \
                                             nv->valuelen = sizeof(VALUE) - 1

#define H2_CREATE_NV_CS_CS(nv, NAME, VALUE) nv->name = (uint8_t *)NAME;       \
                                            nv->namelen = strlen(NAME);       \
                                            nv->value = (uint8_t *)VALUE;     \
                                            nv->valuelen = strlen(VALUE)

/**
 * Moves data from one brigade into another. If maxlen > 0, it only
 * moves up to maxlen bytes into the target brigade, making bucket splits
 * if needed.
 * @param to the brigade to move the data to
 * @param from the brigade to get the data from
 * @param maxlen of bytes to move, 0 for all
 * @param pfile_buckets_allowed how many file buckets may be moved, 
 *        may be 0 or NULL
 * @param msg message for use in logging
 */
apr_status_t h2_util_move(apr_bucket_brigade *to, apr_bucket_brigade *from, 
                          apr_size_t maxlen, int *pfile_buckets_allowed, 
                          const char *msg);

/**
 * Copies buckets from one brigade into another. If maxlen > 0, it only
 * copies up to maxlen bytes into the target brigade, making bucket splits
 * if needed.
 * @param to the brigade to copy the data to
 * @param from the brigade to get the data from
 * @param maxlen of bytes to copy, 0 for all
 * @param msg message for use in logging
 */
apr_status_t h2_util_copy(apr_bucket_brigade *to, apr_bucket_brigade *from, 
                          apr_size_t maxlen, const char *msg);

/**
 * Return != 0 iff there is a FLUSH or EOS bucket in the brigade.
 * @param bb the brigade to check on
 * @return != 0 iff brigade holds FLUSH or EOS bucket (or both)
 */
int h2_util_has_flush_or_eos(apr_bucket_brigade *bb);
int h2_util_has_eos(apr_bucket_brigade *bb, apr_size_t len);
int h2_util_bb_has_data(apr_bucket_brigade *bb);
int h2_util_bb_has_data_or_eos(apr_bucket_brigade *bb);

/**
 * Check how many bytes of the desired amount are available and if the
 * end of stream is reached by that amount.
 * @param bb the brigade to check
 * @param plen the desired length and, on return, the available length
 * @param on return, if eos has been reached
 */
apr_status_t h2_util_bb_avail(apr_bucket_brigade *bb, 
                              apr_size_t *plen, int *peos);

typedef apr_status_t h2_util_pass_cb(void *ctx, 
                                       const char *data, apr_size_t len);

/**
 * Read at most *plen bytes from the brigade and pass them into the
 * given callback. If cb is NULL, just return the amount of data that
 * could have been read.
 * If an EOS was/would be encountered, set *peos != 0.
 * @param bb the brigade to read from
 * @param cb the callback to invoke for the read data
 * @param ctx optional data passed to callback
 * @param plen inout, as input gives the maximum number of bytes to read,
 *    on return specifies the actual/would be number of bytes
 * @param peos != 0 iff an EOS bucket was/would be encountered.
 */
apr_status_t h2_util_bb_readx(apr_bucket_brigade *bb, 
                              h2_util_pass_cb *cb, void *ctx, 
                              apr_size_t *plen, int *peos);

/**
 * Logs the bucket brigade (which bucket types with what length)
 * to the log at the given level.
 * @param c the connection to log for
 * @param stream_id the stream identifier this brigade belongs to
 * @param level the log level (as in APLOG_*)
 * @param tag a short message text about the context
 * @param bb the brigade to log
 */
void h2_util_bb_log(conn_rec *c, int stream_id, int level, 
                    const char *tag, apr_bucket_brigade *bb);

#endif /* defined(__mod_h2__h2_util__) */
