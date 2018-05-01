#ifndef _NGX_RTMP_AES_CTR_H_INCLUDED_
#define _NGX_RTMP_AES_CTR_H_INCLUDED_

#define NGX_RTMP_AES_CTR_IV_SIZE (8)
#define NGX_RTMP_AES_CTR_KEY_SIZE (16)

ngx_int_t
ngx_rtmp_aes_ctr_encrypt(ngx_rtmp_session_t *s, 
    const uint8_t *key, const uint8_t *nonce,
    uint8_t *data, size_t data_len);

#endif /* _NGX_RTMP_AES_CTR_H_INCLUDED_ */
