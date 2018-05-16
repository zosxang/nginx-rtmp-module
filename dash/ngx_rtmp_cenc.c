

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include "ngx_rtmp_cenc.h"


void 
debug_counter(ngx_rtmp_session_t *s, uint8_t *c, uint8_t *k, size_t l)
{
    u_char hexc[AES_BLOCK_SIZE*2+1];
    u_char hexk[AES_BLOCK_SIZE*2+1];

    ngx_hex_dump(hexc, c, AES_BLOCK_SIZE);
    ngx_hex_dump(hexk, k, AES_BLOCK_SIZE);
    hexc[AES_BLOCK_SIZE*2] = '\0';
    hexk[AES_BLOCK_SIZE*2] = '\0';

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->connection->log, 0,
        "dash cenc_counter: %ui %s %s", l, hexc, hexk);
}


ngx_int_t
ngx_rtmp_cenc_read_hex(ngx_str_t src, u_char* dst)
{
    u_char  l, h;
    size_t  i;

    if (src.len != NGX_RTMP_CENC_KEY_SIZE*2) {
        return NGX_ERROR;
    }

    for (i = 0; i < NGX_RTMP_CENC_KEY_SIZE; i++) {
       l = tolower(src.data[i*2]);       
       h = tolower(src.data[i*2+1]);       
       l = l >= 'a' ? l - 'a' + 10 : l - '0'; 
       h = h >= 'a' ? h - 'a' + 10 : h - '0'; 
       dst[i] = (l <<= 4) | h; 
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_cenc_rand_iv(u_char* iv)
{
    if(RAND_bytes(iv, NGX_RTMP_CENC_IV_SIZE) != 1) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


void
ngx_rtmp_cenc_increment_iv(u_char* iv)
{
    int i;

    for (i = NGX_RTMP_CENC_IV_SIZE - 1; i >= 0; i--) {
        iv[i]++;
        if (iv[i])
            break;
    }
}


ngx_int_t
ngx_rtmp_cenc_encrypt(ngx_rtmp_session_t *s, uint8_t *key, uint8_t *iv,
    uint8_t *data, size_t data_len)
{
    /* aes-ctr implementation */

    EVP_CIPHER_CTX* cipher;
    size_t          j, len, left = data_len;
    int             i, w;
    uint8_t        *pos = data;
    uint8_t         counter[AES_BLOCK_SIZE], buf[AES_BLOCK_SIZE];

    ngx_memset(counter + NGX_RTMP_CENC_IV_SIZE, 0, NGX_RTMP_CENC_IV_SIZE);
    ngx_memcpy(counter, iv, NGX_RTMP_CENC_IV_SIZE);

    cipher = EVP_CIPHER_CTX_new();
    if (cipher == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->connection->log, 0,
                      "dash rtmp_cenc_encrypt: evp_cipher_ctx failed");
        return NGX_ERROR;
    }

    if (EVP_EncryptInit_ex(cipher, EVP_aes_128_ecb(), NULL, key, NULL) != 1) {
        ngx_log_error(NGX_LOG_ERR, s->connection->log, 0,
                      "dash rtmp_cenc_encrypt: evp_encrypt_init failed");
        return NGX_ERROR;
    }

    while (left > 0) {

        debug_counter(s, counter, key, left);
        EVP_EncryptUpdate(cipher, buf, &w, counter, AES_BLOCK_SIZE);

        len = (left < AES_BLOCK_SIZE) ? left : AES_BLOCK_SIZE;
        for (j = 0; j < len; j++)
            pos[j] ^= buf[j];
        pos += len;
        left -= len;

        for (i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
            counter[i]++;
            if (counter[i])
                break;
        }
    }

    EVP_CIPHER_CTX_free(cipher);

    return NGX_OK;
}
 

