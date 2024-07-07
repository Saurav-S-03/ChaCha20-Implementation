#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>

#include "chacha20.h"

uint32_t Hex_to_32(int* arr){
    uint32_t res;
    // In Little Endian
    res = (uint32_t)( (arr[6]<<12) | (arr[7]<<8) | (arr[4]<<4) | (arr[5]<<0) )<<16;
    res |= (uint32_t)( (arr[2]<<12) | (arr[3]<<8) | (arr[0]<<4) | (arr[1]<<0) );
    
    return res;
}

void Uint_32_to_8(uint8_t* output,uint32_t input){
    output[0] = (uint8_t)((input) & 0xFF);      // Least significant byte
    output[1] = (uint8_t)((input >> 8) & 0xFF);
    output[2] = (uint8_t)((input >> 16) & 0xFF);
    output[3] = (uint8_t)((input >> 24) & 0xFF);    // Most significant byte
}

uint32_t ROTL(uint32_t x, int n){
    x = ((x <<n) | (x >>(32-n)));
    return x;
}

chacha_state chacha_init(int* key,int* nonce,uint32_t counter){
    chacha_state State;
    State = (chacha_state)malloc(sizeof(struct Chacha_state));
    
    if(State == NULL){
        printf("NOT ABLE TO CREATE A KEY BLOCK....!!!!\nTERMINATING...!!!\n");
        exit(0);
    }
    //Declaring Constant part of the key block
    State->matrix[0] = 0x61707865;  // "expa"
    State->matrix[1] = 0x3320646e;  // "nd 3"
    State->matrix[2] = 0x79622d32;  // "2-by"
    State->matrix[3] = 0x6b206574;  // "te k"

    /*-------Storing key into block-------*/
    //2nd Row
    State->matrix[4] = Hex_to_32(key + 0);
    State->matrix[5] = Hex_to_32(key + 8);
    State->matrix[6] = Hex_to_32(key + 16);
    State->matrix[7] = Hex_to_32(key + 24);
    //3rd Row
    State->matrix[8] = Hex_to_32(key + 32);
    State->matrix[9] = Hex_to_32(key + 40);
    State->matrix[10] = Hex_to_32(key + 48);
    State->matrix[11] = Hex_to_32(key + 56);
    /*-------------------------------------*/

    //Declaring Counter
    State->matrix[12] = counter;

    /*-------Storing Nonce------*/
    State->matrix[13] = Hex_to_32(nonce + 0);
    State->matrix[14] = Hex_to_32(nonce + 8);
    State->matrix[15] = Hex_to_32(nonce + 16);
    /*--------------------------*/

    return State;
}

void Quarter_Round(chacha_state block, int a, int b, int c, int d){

    // a += b; d ^= a; d = ROTL(d, 16);
    block->matrix[a] += block->matrix[b];
    block->matrix[d] ^= block->matrix[a];
    block->matrix[d] = ROTL(block->matrix[d],16);

    // c += d; b ^= c; b = ROTL(b, 12);
    block->matrix[c] += block->matrix[d];
    block->matrix[b] ^= block->matrix[c];
    block->matrix[b] = ROTL(block->matrix[b],12);

    // a += b; d ^= a; d = ROTL(d, 8);
    block->matrix[a] += block->matrix[b];
    block->matrix[d] ^= block->matrix[a];
    block->matrix[d] = ROTL(block->matrix[d],8);

    // c += d; b ^= c; b = ROTL(b, 7);
    block->matrix[c] += block->matrix[d];
    block->matrix[b] ^= block->matrix[c];
    block->matrix[b] = ROTL(block->matrix[b],7);

}

void chacha_round(chacha_state State){
    /*-----------Odd Round-----------*/
    Quarter_Round(State, 0, 4, 8,12);   //Column-0
    Quarter_Round(State, 1, 5, 9,13);   //Column-1
    Quarter_Round(State, 2, 6,10,14);   //Column-2
    Quarter_Round(State, 3, 7,11,15);   //Column-3
    /*----Elements of each Column----*/

    /*----------Even Round----------*/
    Quarter_Round(State, 0, 5,10,15);
    Quarter_Round(State, 1, 6,11,12);
    Quarter_Round(State, 2, 7, 8,13);
    Quarter_Round(State, 3, 4, 9,14);
    /*-------Diagonal Elements-------*/
}

uint8_t* serialize(chacha_state Block){
    uint8_t* res;
    res = (uint8_t*)malloc(64 * sizeof(uint8_t));
    if(res == NULL){
        printf("NOT able to Serialize Key-Block...\nTerminating..!!\n");
        exit(0);
    }
    int i;
    for(i=0; i<16; i++){
        Uint_32_to_8((res + 4*i),Block->matrix[i]);
    }
    
    return res;
}

uint8_t* chacha_keystream_generator(int* key, int* nonce, uint32_t counter){
    /*-----------Key, Nonce should be in Hex-----------*/
    // printf("\nN = ");
    // for(int z=0;z<24;z++){
    //     printf("%x ",nonce[z]);
    // }

    chacha_state State, init_state;
    init_state = chacha_init(key,nonce,counter);
    State = chacha_init(key,nonce,counter);

    // printf("\n\nState:");
    // for(int z=0;z<16;z++){
    //     printf("%x ",init_state->matrix[z]);
    // }

    for(int i=0;i<10;i++){
        chacha_round(State);
    }

    // printf("\n\nChanged State:");
    // for(int z=0;z<16;z++){
    //     printf("%x ",State->matrix[z]);
    // }

    for(int i=0;i<16;i++){
        State->matrix[i] += init_state->matrix[i];
    }
    free(init_state);

    // printf("\n\nFinal State:");
    // for(int z=0;z<16;z++){
    //     printf("%x ",State->matrix[z]);
    // }

    uint8_t* keystream = serialize(State);
    free(State);

    // printf("\nKey-Stream:\n");
    // for(int z=0;z<64;z++){
    //     printf("%x ",keystream[z]);
    // }

    return(keystream);
}

void chacha_encrypt(uint8_t* plain,uint8_t* cipher,int* key, int* nonce,int length){
    /*------------Key, Nonce should be in Hex------------*/
    
    // printf("Nonce = ");
    // for(int z=0;z<24;z++){
    //     printf("%x ",nonce[z]);
    // }
    // printf("\n");

    uint32_t counter=1;
    uint8_t* key_stream;

    int i;
    for(i=0; i<length ;i+=64)
    {
        key_stream = chacha_keystream_generator(key,nonce,counter);

        int len;
        len = ((length-i) >= 64)? 64 : (length-i);
        for(int j=0; j<len ;j++){
            cipher[i + j] = plain[i + j] ^ key_stream[j];
        }
        free(key_stream);
        counter++;
    }

    // printf("\nCipher = \n");
    // for(int z=0;z<length;z++){
    //     printf("%x ",cipher[z]);
    // }
}
void chacha_decrypt(uint8_t* cipher, uint8_t* plain, int* key, int* nonce,int length){
    chacha_encrypt(cipher,plain,key,nonce,length);
}

