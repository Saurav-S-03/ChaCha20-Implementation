#include "formatting.h"

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
