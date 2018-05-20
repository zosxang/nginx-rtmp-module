#ifndef _NGX_RTMP_CENC_H_INCLUDED_
#define _NGX_RTMP_CENC_H_INCLUDED_

#define NGX_RTMP_CENC_IV_SIZE (8)
#define NGX_RTMP_CENC_KEY_SIZE (16)
#define NGX_RTMP_CENC_CK_SID \
    "\x10\x77\xef\xec\xc0\xb2\x4d\x02" \
    "\xac\xe3\x3c\x1e\x52\xe2\xfb\x4b"

ngx_int_t
ngx_rtmp_cenc_read_hex(ngx_str_t src, u_char* dst);

ngx_int_t
ngx_rtmp_cenc_rand_iv(u_char* iv);

void
ngx_rtmp_cenc_increment_iv(u_char* iv);

ngx_int_t
ngx_rtmp_cenc_encrypt(ngx_rtmp_session_t *s,
    uint8_t *key, uint8_t *iv, uint8_t *data, size_t data_len);

#endif /* _NGX_RTMP_CENC_H_INCLUDED_ */
