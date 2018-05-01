

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include "ngx_rtmp_aes_ctr.h"


void 
print_counter(ngx_rtmp_session_t *s, uint8_t *c, size_t l)
{
    u_char hex[AES_BLOCK_SIZE*2+1];

    ngx_hex_dump(hex, c, AES_BLOCK_SIZE);
    hex[AES_BLOCK_SIZE*2] = '\0';

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->connection->log, 0,
        "dash aes_counter: %ui %s", l, hex);
}


ngx_int_t
ngx_rtmp_aes_ctr_encrypt(ngx_rtmp_session_t *s, 
    const uint8_t *key, const uint8_t *nonce,
    uint8_t *data, size_t data_len)
{

    EVP_CIPHER_CTX* cipher;
    size_t          j, len, left = data_len;
    int             i, w;
    uint8_t        *pos = data;
    uint8_t         counter[AES_BLOCK_SIZE], buf[AES_BLOCK_SIZE];

    ngx_memset(counter + NGX_RTMP_AES_CTR_IV_SIZE, 0, NGX_RTMP_AES_CTR_IV_SIZE);
    ngx_memcpy(counter, nonce, NGX_RTMP_AES_CTR_IV_SIZE);

    cipher = EVP_CIPHER_CTX_new();
    if (cipher == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->connection->log, ngx_errno,
                      "dash rtmp_aes_ctr_encrypt: evp_cipher_ctx failed");
        return NGX_ERROR;
    }

    if (EVP_EncryptInit_ex(cipher, EVP_aes_128_ecb(), NULL, key, NULL) != 1) {
        ngx_log_error(NGX_LOG_ERR, s->connection->log, ngx_errno,
                      "dash rtmp_aes_ctr_encrypt: evp_encrypt_init failed");
        return NGX_ERROR;
    }

    while (left > 0) {

        print_counter(s, counter, left);
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
 

