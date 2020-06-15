
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv){
    if (argc!=2){
        printf("Poucos Argumentos\n");
        return -1;
        }

    printf("ENTRADO\n");
    sleep(atoi(argv[1]));
    printf("SAIDO\n");

    return 0;
}