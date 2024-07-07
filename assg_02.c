#include "chacha20.h"
#include "formatting.h"

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
