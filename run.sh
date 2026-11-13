CCX=g++

mkdir -p build/

set -x 

$CCX -O2 -march=native -o build/$1 src/$1.cpp -pthread -lm
./build/$1
