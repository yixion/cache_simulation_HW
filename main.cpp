# include "cache.h"

int main(int argc, char* argv[]){
    bitset<32> cache[CacheInit(stoi(argv[1]), stoi(argv[2]), argv[3], argv[4])];
    CacheSimulation(argv[5]);
    return 0;
}