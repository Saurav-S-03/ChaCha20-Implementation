#ifndef FORMAT
#define FORMAT

#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<time.h>

struct Text{
    uint8_t plain[700];
    uint8_t cipher[700];
    int key[64];
    int nonce[24];
    int Length;
};
typedef struct Text* text;

int Char_to_Hex(char c);
uint8_t Hex_to_uint8(char a,char b);
int Inputs_Setup(text inputs,int x);
void write_uint8(FILE* file, uint8_t num);
void write_Hex(FILE* file, int* arr, int length);
void Time(char* timeString);
int update_logs(text block, char* fname, char* mode, int enc_or_dec);
int write_output(text block, char* fname, char* mode, int enc_or_dec);
void write_back(text block, int enc_or_dec);
void print_on_terminal(text block, int enc_or_dec);

#endif