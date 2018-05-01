

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include "ngx_rtmp_mp4_aes_ctr.h"


static ngx_int_t ngx_rtmp_mp4_aes_ctr_encrypt(const uint8_t *key, const uint8_t *nonce, uint8_t *data, size_t data_len) {

    EVP_CIPHER_CTX* cipher;
    size_t j, len, left = data_len;
    int i, w;
    uint8_t *pos = data;
    uint8_t counter[AES_BLOCK_SIZE], buf[AES_BLOCK_SIZE];

    ngx_memset(counter + 8, 0, 8);
    ngx_memcpy(counter, nonce, 8);

    cipher = EVP_CIPHER_CTX_new();
    /* test return NGX_ERROR */
    EVP_EncryptInit_ex(cipher, EVP_aes_128_ecb(), NULL, key, NULL);
    /* test */

    while (left > 0) {
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
 

