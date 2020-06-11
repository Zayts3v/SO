#include <unistd.h>
#include <fcntl.h>

const int magicnumberANDnullsize[2] = {0x00000C02,0};

int openIDX_1DimInt(char * filename, int * Number_Of_Entrys){
    int res;
    if((res = open(filename,O_RDWR,0666))<0) return mkIDX_1DimInt(filename);
    int magicnumber,ne;
    if (read(res,&magicnumber,4)<4) return mkIDX_1DimInt(filename);
    if(magicnumber!= magicnumberANDnullsize[0]) return mkIDX_1DimInt(filename);


}

int mkIDX_1DimInt(char * filename){
    int res = open(filename,O_CREAT | O_TRUNC | O_RDWR,0666);
    write(res,magicnumberANDnullsize,sizeof(magicnumberANDnullsize));
    return res;
}


int readIndexIDX(int index,int file){
    lseek(file,4,SEEK_SET);

}