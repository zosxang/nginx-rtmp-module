#ifndef _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_
#define _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_

static ngx_int_t
ngx_rtmp_mp4_aes_ctr_encrypt(const uint8_t *key, const uint8_t *nonce,
                             uint8_t *data, size_t data_len);

#endif /* _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_ */
