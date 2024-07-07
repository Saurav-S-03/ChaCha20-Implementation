#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

/*------------------------------------formatting------------------------------------*/
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
/*------------------------------------------------------------------------------------*/

/*--------------------------------------chacha20--------------------------------------*/
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
/*---------------------------------------------------------------------------------------*/

/*------------------------------------------Main------------------------------------------*/
struct Text vars;

int main()
{
    char c;
    while(1)
    {
        printf("\n\n\n");
        printf("------Enter an input------\n");
        printf("      * e/E -> Encrypt\n");
        printf("      * d/D -> Decrypt\n");
        printf("      * Default-> Exit\n");
        printf("> ");
        scanf(" %c",&c);

        if(c=='e' || c=='E'){
            text Vars = &vars;
            if(Inputs_Setup(Vars,0) != 0){
                continue;
            }
            printf("..Done\n");
            printf("Encrypting...");
            chacha_encrypt(Vars->plain,Vars->cipher,Vars->key,Vars->nonce,Vars->Length);
            printf("...Done\n");
            print_on_terminal(Vars,0);
            write_back(Vars,0);
        }
        else if(c=='d' || c=='D'){
            text Vars = &vars;
            if(Inputs_Setup(Vars,1) != 0){
                continue;
            }
            printf("..Done\n");
            printf("Decrypting...");
            chacha_decrypt(Vars->cipher,Vars->plain,Vars->key,Vars->nonce,Vars->Length);
            printf("...Done\n");
            print_on_terminal(Vars,1);
            write_back(Vars,1);
        }
        else{
            break;
        }
    }
    printf("\nTerminating Program...");
    return 0;
}

/*----------------------------------------------------------------------------------------*/

/*--------------------------------------Formatting.c--------------------------------------*/
int Char_to_Hex(char c){
    //Ascii of 0=48
    if((c >= '0') && (c <='9')){
        return(c-48);
    }
    //Ascii of A=65 => A-55=10=0xa
    if((c >= 'A') && (c <= 'F')){
        return(c-55);
    }
    //Adcii of a=97 => a-87=10=0xa
    if((c >= 'a') && (c <= 'f')){
        return(c-87);
    }
    return(-1); //Invalid char
}

uint8_t Hex_to_uint8(char a,char b){
    int i,j;
    i = Char_to_Hex(a);
    j = Char_to_Hex(b);

    uint8_t res;
    res = (uint8_t)((i <<4) | (j));
    return res;
}

int Inputs_Setup(text inputs,int x){
    char fname[50];
    printf("Enter Inputs File-Name: ");
    scanf(" %s",fname);

    FILE *file = fopen(fname, "r");
    if (file == NULL) {
        perror(" [-] Error opening file\n");
        return -1;
    }
    int i, temp;
    char ch;

    printf("\nReading Key...");
    // Read and print the content from the current position
    i=0;
    while ((ch = fgetc(file)) != '\n') {
        if(ch == ' ')
            continue;
        inputs->key[i++] = Char_to_Hex(ch);
    }
    while((ch = fgetc(file)) == '\n');
    // Move the file pointer one character back
    fseek(file, -1, SEEK_CUR);

    printf("...Done\nReading Nonce...");
    i=0;
    while ((ch = fgetc(file)) != '\n') {
        if(ch == ' ')
            continue;
        inputs->nonce[i++] = Char_to_Hex(ch);
    }
    while((ch = fgetc(file)) == '\n');
    // Move the file pointer one character back
    fseek(file, -1, SEEK_CUR);
    printf("...Done");
    char a,b;
    i=0;
    if(x == 0){
        printf("\nReading Plain Text...");
        while (1) {
            a = fgetc(file);
            if(a=='\n' || a==EOF)
                break;
            if(a==' ')
                continue;
            b = fgetc(file);
            if(b==' ' || b=='\n'){
                // fseek(file, -1, SEEK_CUR);
                b = a;
                a = '0';
            }

            inputs->plain[i++] = Hex_to_uint8(a,b);
        }
        inputs->plain[i] = 0;
    }
    else{
        printf("\nReading Cipher Text...");
        while (1) {
            a = fgetc(file);
            if(a=='\n' || a==EOF)
                break;
            if(a==' ')
                continue;
            b = fgetc(file);
            if(b==' ' || b=='\n'){
                // fseek(file, -1, SEEK_CUR);
                b = a;
                a = '0';
            }

            inputs->cipher[i++] = Hex_to_uint8(a,b);
        }
        inputs->cipher[i] = 0;
    }
    inputs->Length = i;

    fclose(file);
    return 0;
}

void write_uint8(FILE* file, uint8_t num){
    char tmp[3];
    snprintf(tmp,sizeof(tmp),"%x",num);
    fprintf(file,tmp);
    fprintf(file," ");
}

void write_Hex(FILE* file, int* arr, int length){
    char tmp[2];
    for(int i=0; i<length ;i++){
        snprintf(tmp,sizeof(tmp),"%x",arr[i]);
        fprintf(file,tmp);
        fprintf(file," ");
    }
}

void Time(char* timeString){
    time_t currentTime;
    time(&currentTime);

    // Convert the time to a struct tm
    struct tm *localTime = localtime(&currentTime);

    // Format the time as a string
    strftime(timeString, 30, "%d/%m/%Y -- %H:%M:%S", localTime);
}

int update_logs(text block, char* fname, char* mode, int enc_or_dec){
    char timeString[30];
    Time(timeString);
    
    FILE *logfile = fopen(fname,mode);
    // FILE *Outfile;
    if(logfile == NULL)
        return -1;

    fprintf(logfile,"\n\n\n");

    fprintf(logfile,"Log: ");
    fprintf(logfile,timeString);
    fprintf(logfile,"\n\n");

    if(enc_or_dec){
        fprintf(logfile,"Operation: Decryption");
    }
    else{
        fprintf(logfile,"Operation: Encryption");
    }
    fprintf(logfile,"\n\n");

    int a,b;
    uint8_t temp;

    // Writing Back Key..
    fprintf(logfile,"Key:\n");
    write_Hex(logfile,block->key,64);
    fprintf(logfile,"\n\n");

    
    // Writing Back Nonce...
    fprintf(logfile,"Nonce:\n");
    write_Hex(logfile,block->nonce,24);
    fprintf(logfile,"\n\n");

    char ab[2];
    ab[1] = '\0';
    // Writing Back Plain text...
    fprintf(logfile,"Plain-Text:\n");
    for(int i=0; i<block->Length ;i++){
        ab[0] = (char)block->plain[i];
        fprintf(logfile,ab);
    }
    fprintf(logfile,"\n\n");
    fprintf(logfile,"Plain-Text in Hexadecimal:\n");
    for(int i=0; i<block->Length ;i++){
        write_uint8(logfile,block->plain[i]);
    }
    fprintf(logfile,"\n\n");


    // Writing Back Cipher text
    fprintf(logfile,"Cipher-Text in Hexadecimal:\n");
    for(int i=0; i<(block->Length) ;i++){
        write_uint8(logfile,block->cipher[i]);
    }
    fprintf(logfile,"\n\n");

    fprintf(logfile,"Cipher-Text:\n");
    for(int i=0; i<(block->Length) ;i++){
        ab[0]  = (char)block->cipher[i];
        fprintf(logfile,ab);
    }
    fprintf(logfile,"\n\n\n");

    char line[] = "\n-----------------------------------------------------------------------------------------------------------------\n";
    fprintf(logfile,line);

    fclose(logfile);
    return 0;
}

int write_output(text block, char* fname, char* mode, int enc_or_dec){
    FILE *logfile = fopen(fname,mode);
    // FILE *Outfile;
    if(logfile == NULL)
        return -1;

    int a,b;
    uint8_t temp;

    // Writing Back Key..
    write_Hex(logfile,block->key,64);
    fprintf(logfile,"\n\n");

    // Writing Back Nonce...
    write_Hex(logfile,block->nonce,24);
    fprintf(logfile,"\n\n");

    char ab[2];
    ab[1] = '\0';
    if(enc_or_dec == 0){
        // Writing back input file for decryption
        for(int i=0; i<(block->Length) ;i++){
            write_uint8(logfile,block->cipher[i]);
        }
        fprintf(logfile,"\n\n");

        fprintf(logfile,"Cipher-Text:\n");
        for(int i=0; i<(block->Length) ;i++){
            ab[0]  = (char)block->cipher[i];
            fprintf(logfile,ab);
        }
    }
    else{
        // Writing back input file for encryption
        for(int i=0; i<block->Length ;i++){
            write_uint8(logfile,block->plain[i]);
        }
        fprintf(logfile,"\n\n\n");

        fprintf(logfile,"Plain-Text:\n");
        for(int i=0; i<block->Length ;i++){
            ab[0] = (char)block->plain[i];
            fprintf(logfile,ab);
        }
    }
    fclose(logfile);
    return 0;
}

void write_back(text block, int enc_or_dec){
    int ret;
    char check;
    
    printf("Update logs [Y/n]: ");
    scanf(" %c",&check);
    if(check=='Y' || check=='y'){
        printf("Updating logs...\n");
        ret = update_logs(block,"logs.txt","a",enc_or_dec);
        if(ret != 0)
            printf(" [-] Unable to Update Logs...\n");
        else
            printf(" [+] Updated logs\n");
    }
    
    printf("\nWrite Back to File [y/n]: ");
    scanf(" %c",&check);

    if(check=='Y' || check=='y'){
        char name[30];
        printf("Enter file Name: ");
        scanf(" %s",name);
        ret = update_logs(block,name,"w",enc_or_dec);
        if(ret != 0)
            printf(" [-] Unable to Write...\n");
        else
            printf(" [+] Write Back Success\n");
    }

    printf("\nPrepare input File [y/n]: ");
    scanf(" %c",&check);

    if(check=='Y' || check=='y'){
        char name[30];
        printf("Enter file Name: ");
        scanf(" %s",name);
        ret = write_output(block,name,"w",enc_or_dec);
        if(ret != 0)
            printf(" [-] Unable to Write...\n");
        else
            printf(" [+] Write Back Success\n");
    }
}

void print_on_terminal(text block, int enc_or_dec){
    printf("\nKey : ");
    for(int i=0; i<64 ;i++){
        printf("%x ",block->key[i]);
    }
    printf("\n\n");

    printf("Nonce : ");
    for(int i=0; i<24 ;i++){
        printf("%x ",block->nonce[i]);
    }
    printf("\n\n");

    if(enc_or_dec == 0){
        printf("Plain-Text :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%c",block->plain[i]);
        }
        printf("\n\n");

        printf("Plain-Text in Hexadecimal :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%x ",block->plain[i]);
        }
        printf("\n\n");

        printf("Cipher-Text in Hexadecimal :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%x ",block->cipher[i]);
        }
        printf("\n\n");

        printf("Cipher-Text :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%c",block->cipher[i]);
        }
        printf("\n\n");
    }
    else{
        printf("Cipher-Text :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%c",block->cipher[i]);
        }
        printf("\n\n");

        printf("Cipher-Text in Hexadecimal :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%x ",block->cipher[i]);
        }
        printf("\n\n");

        printf("Plain-Text in Hexadecimal :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%x ",block->plain[i]);
        }
        printf("\n\n");

        printf("Plain-Text :\n");
        for(int i=0; i<(block->Length) ;i++){
            printf("%c",block->plain[i]);
        }
        printf("\n\n");
    }
}

/*----------------------------------------------------------------------------------------*/

/*---------------------------------------Chacha20.c---------------------------------------*/
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

/*----------------------------------------------------------------------------------------*/
