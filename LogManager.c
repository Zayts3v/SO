#include <unistd.h>
#include <fcntl.h>

const int magicnumberANDnullsize[2] = {0x00000C01,0};

const int magicnumber = 0x00000C01;
const int nullsize = 0;

/*
int openIDX_1DimInt(char * filename, int * Number_Of_Entrys){
    int res;
    if((res = open(filename,O_RDWR,0666))<0) return mkIDX_1DimInt(filename);
    int magicnumber;
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
} */


int openIDX (int nLinhas, int *desc){
    
    int fi = open("logs.idx", O_RDWR, 0666);
    
    if (fi < 0 ){ //falta testar o n magico no if
        fi = open("logs.idx", O_CREAT | O_TRUNC | O_RDWR, 0666);
        write(fi,magicnumberANDnullsize,sizeof(magicnumberANDnullsize));
        nLinhas = 0;
        *desc = fi;
        return 0;
    }
    else {
        int i;
        lseek(fi,4,SEEK_SET);
        read(fi,&i,sizeof(int));
        nLinhas = i;
        *desc = fi;
        return 1;
    }
}

int openLogs(){
    int fl = open("logs", O_RDWR, 0666);
    if ( fl < 0 ){
        fl = open("logs", O_CREAT | O_TRUNC | O_RDWR,0666);
        write(fl, '0', 1);
    }
    return fl; //always returns desc
}

void updateIDX(int index, int valor){ //se nao se souber o indice, dá-se um valor negativo para index
    int i, fi;
    openIDX(i,fi);
    
    lseek(fi,4,SEEK_SET); //passa o n magico
    write (fi, i++, sizeof(int)); //atualizar n linhas

    if(index<0){ //se nao se souber o index, escreve no final do idx
        lseek(fi,0,SEEK_END);
    }
    else lseek(fi,index,SEEK_CUR); //caso contrario escreve no index
    
    write(fi,valor,sizeof(valor));
    write(fi,'\n',sizeof('\n'));
}


int updateLogs(char * comando, int filetocopy){
    int output = open(filetocopy, O_RDONLY, 0666);
    if (output < 0 ) return -1;
    
    int fl = openLogs();
    int pos = lseek(fl,0,SEEK_END); //posiçao onde escreve

    write(fl,comando,strlen(comando)); //write comando
    write(fl, '\n', sizeof('\n'));
    
    ssize_t n; //write output
    char c;
    while (n = read(output,&c,1)> 0 && c!='\n'){ //tem '\n' a delimitar ? help
        write(fl,&c,n);
    }
    write(fl, '\n', sizeof('\n'));

    return pos;
}


int readIndexIDX(int index){
    
    int fi = open("logs.idx", O_RDONLY, 0666);
    if (fi < 0 ) return -1;

    lseek(fi,sizeof(magicnumberANDnullsize),SEEK_SET);
    lseek(fi,index,SEEK_CUR); //get to index

    int v;
    ssize_t n;
    n = read(fi,&v,sizeof(int));
    if (n < 0) return -1;
    
    return v;
}
