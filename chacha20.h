#ifndef CHACHA20
#define CHACHA20

// https://datatracker.ietf.org/doc/html/rfc7539

#include<stdint.h>

struct Chacha_state{
    uint32_t matrix[16];
};
typedef struct Chacha_state* chacha_state;

uint32_t Hex_to_32(int* arr);
void Uint_32_to_8(uint8_t* output,uint32_t input);
uint32_t ROTL(uint32_t x, int n);
chacha_state chacha_init(int* key,int* nonce,uint32_t counter);
void Quarter_Round(chacha_state block, int a, int b, int c, int d);
void chacha_round(chacha_state State);
uint8_t* serialize(chacha_state Block);
uint8_t* chacha_keystream_generator(int* key, int* nonce, uint32_t counter);
void chacha_encrypt(uint8_t* plain,uint8_t* cipher,int* key, int* nonce,int length);
void chacha_decrypt(uint8_t* cipher, uint8_t* plain, int* key, int* nonce,int length);

#endif
