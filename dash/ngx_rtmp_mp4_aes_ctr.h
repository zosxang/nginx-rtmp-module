#ifndef _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_
#define _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_

#define NGX_RTMP_MP4_AES_CTR_KEY_SIZE (16)
#define NGX_RTMP_MP4_AES_CTR_IV_SIZE (8)
#define NGX_RTMP_MP4_AES_CTR_COUNTER_BUFFER_SIZE (AES_BLOCK_SIZE * 64)

typedef struct {
    EVP_CIPHER_CTX* cipher;
    u_char     ivec[AES_BLOCK_SIZE];
    ngx_uint_t num;
    u_char     ecount[AES_BLOCK_SIZE];
} ngx_rtmp_mp4_aes_crt_state;

ngx_int_t ngx_rtmp_mp4_mp4_aes_ctr_init(ngx_rtmp_mp4_aes_ctr_state* state, request_context_t* request_context, u_char* key);
void ngx_rtmp_mp4_aes_ctr_set_iv(ngx_rtmp_mp4_aes_ctr_state* state, u_char* iv);
ngx_int_t ngx_rtmp_mp4_aes_ctr_process(ngx_rtmp_mp4_aes_ctr_state_t* state, u_char* dest, const u_char* src, uint32_t size);
void ngx_rtmp_mp4_aes_ctr_increment_be64(u_char* counter);
ngx_int_t ngx_rtmp_mp4_aes_ctr_write_encrypted(ngx_rtmp_mp4_aes_ctr_state_t* state, write_buffer_state_t* write_buffer, u_char* cur_pos, uint32_t write_size);

#endif /* _NGX_RTMP_MP4_AES_CTR_H_INCLUDED_ */
