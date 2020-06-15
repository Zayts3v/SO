#!bin/bash
#Ficheiro de testes

make
./argusd
./argus -i 19
./argus -e ls -l | wc -l
./argus --stopserver

make clean
