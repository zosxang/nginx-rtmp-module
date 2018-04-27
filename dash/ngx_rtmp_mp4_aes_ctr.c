#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct ctr_state { 
    EVP_CIPHER_CTX* cipher;
    unsigned char ivec[AES_BLOCK_SIZE];
    unsigned int num;
    unsigned char ecount[AES_BLOCK_SIZE];
}; 


static void AES_ctr128_inc(unsigned char *counter) {
  unsigned char* cur_pos;

  for (cur_pos = counter + 15; cur_pos >= counter; cur_pos--) {
    (*cur_pos)++;
    if (*cur_pos != 0) {
      break;
    }
  }
}


void print_hex(unsigned char *c) {
  for(int i = 0; i < 16; i++) {
    printf("%02X.", c[i]);
  }
  printf("\n");
}

void init_ctr(struct ctr_state *state, unsigned char iv[16], unsigned char* key) {
  state->num = 0;
  memset(state->ecount, 0, 16);
  memset(state->ivec + 8, 0, 8);
  memcpy(state->ivec, iv, 8);
  state->cipher = EVP_CIPHER_CTX_new();
  EVP_EncryptInit_ex(state->cipher, EVP_aes_128_ecb(), NULL, key, NULL);
}

vod_status_t
mp4_aes_ctr_init(
  mp4_aes_ctr_state_t* state,
  request_context_t* request_context, 
  u_char* key)
{
  vod_pool_cleanup_t *cln;

  state->request_context = request_context;

  cln = vod_pool_cleanup_add(request_context->pool, 0);
  if (cln == NULL)
  {
    vod_log_debug0(VOD_LOG_DEBUG_LEVEL, request_context->log, 0,
      "mp4_aes_ctr_init: vod_pool_cleanup_add failed");
    return VOD_ALLOC_FAILED;
  }
  
  state->cipher = EVP_CIPHER_CTX_new();
  if (state->cipher == NULL)
  {
    vod_log_error(VOD_LOG_ERR, request_context->log, 0,
      "mp4_aes_ctr_init: EVP_CIPHER_CTX_new failed");
    return VOD_ALLOC_FAILED;
  }

  cln->handler = (vod_pool_cleanup_pt)mp4_aes_ctr_cleanup;
  cln->data = state;

  if (1 != EVP_EncryptInit_ex(state->cipher, EVP_aes_128_ecb(), NULL, key, NULL))
  {
    vod_log_error(VOD_LOG_ERR, request_context->log, 0,
      "mp4_aes_ctr_init: EVP_EncryptInit_ex failed");
    return VOD_ALLOC_FAILED;
  }

  return VOD_OK;
}

void 
mp4_aes_ctr_set_iv(
  mp4_aes_ctr_state_t* state, 
  u_char* iv)
{
  vod_memcpy(state->counter, iv, MP4_AES_CTR_IV_SIZE);
  vod_memzero(state->counter + MP4_AES_CTR_IV_SIZE, sizeof(state->counter) - MP4_AES_CTR_IV_SIZE);
  state->encrypted_pos = NULL;
  state->encrypted_end = NULL;
}

void AES_ctr128_EVPencrypt(EVP_CIPHER_CTX* cipher, const unsigned char *in, unsigned char *out,
  const unsigned long length,
  unsigned char counter[AES_BLOCK_SIZE],
  unsigned char ecount_buf[AES_BLOCK_SIZE],
  unsigned int *num) {

  int nb;
  unsigned int n;
  unsigned long l=length;

  n = *num;

  while (l--) {
    if (n == 0) {
      EVP_EncryptUpdate(cipher, ecount_buf, &nb, counter, AES_BLOCK_SIZE);
      AES_ctr128_inc(counter);
    }
    *(out++) = *(in++) ^ ecount_buf[n];
    n = (n+1) % AES_BLOCK_SIZE;
  }

  *num=n;
}

void fencrypt(char* read, char* write, unsigned char* enc_key) { 

  FILE *readFile;
  FILE *writeFile;

  int bytes_read;     
  unsigned char indata[AES_BLOCK_SIZE]; 
  unsigned char outdata[AES_BLOCK_SIZE];
  unsigned char iv[AES_BLOCK_SIZE];

  struct ctr_state state;     

  RAND_bytes(iv, AES_BLOCK_SIZE);
  //unsigned char *iv = (unsigned char *)"0123456789012345";
        
  readFile = fopen(read,"rb");
  writeFile = fopen(write,"wb");
    
  fwrite(iv, 1, AES_BLOCK_SIZE, writeFile); 
  //printf("saved IVec : ");
  //print_hex(iv);

  init_ctr(&state, iv, enc_key);
  //printf("first IVec : ");
  //print_hex(state.ivec);

  while(1) {
    bytes_read = fread(indata, 1, AES_BLOCK_SIZE, readFile);
    //printf("read data : ");
    //print_hex(indata);

    AES_ctr128_EVPencrypt(state.cipher, indata, outdata, bytes_read, state.ivec, state.ecount, &state.num);

    //printf("encrypt IVec : ");
    //print_hex(state.ivec);
    //printf("encrypt count : ");
    //print_hex(state.ecount);
    //printf("write data : ");
    //print_hex(outdata);

    fwrite(outdata, 1, bytes_read, writeFile); 
    if (bytes_read < AES_BLOCK_SIZE) {
      break;
    }
  }
    
  fclose(writeFile);
  fclose(readFile);
}

void fdecrypt(char* read, char* write, unsigned char* enc_key) { 

  FILE *readFile;
  FILE *writeFile;

  int bytes_read;     
  unsigned char indata[AES_BLOCK_SIZE]; 
  unsigned char outdata[AES_BLOCK_SIZE];
  unsigned char iv[AES_BLOCK_SIZE];

  struct ctr_state state;     

  readFile = fopen(read,"rb");
  writeFile = fopen(write,"wb");
    
  fread(iv, 1, AES_BLOCK_SIZE, readFile); 
  //printf("read IV : ");
  //print_hex(iv);

  init_ctr(&state, iv, enc_key);
  //printf("first IVec : ");
  //print_hex(state.ivec);

  while(1) {
    bytes_read = fread(indata, 1, AES_BLOCK_SIZE, readFile); 
    //printf("read data : ");
    //print_hex(indata);

    AES_ctr128_EVPencrypt(state.cipher, indata, outdata, bytes_read, state.ivec, state.ecount, &state.num);

    //printf("decrypt IVec : ");
    //print_hex(state.ivec);
    //printf("decrypt count : ");
    //print_hex(state.ecount);
    //printf("write data : ");
    //print_hex(outdata);
        
    fwrite(outdata, 1, bytes_read, writeFile); 
    if (bytes_read < AES_BLOCK_SIZE) {
      break;
    }
  }
    
  fclose(writeFile);
  fclose(readFile);
}
    
