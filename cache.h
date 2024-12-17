#ifndef HEADERFILE
#define HEADERFILE
# include <iostream>
# include <bitset>
# include <string>
# include <fstream>

# define MAX_CACHE_LINE 65536 // 64K 

# ifndef STRUCT_TYPE
# define STRUCT_TYPE

//dircet map, full associative map etc.
enum associative{
    direct_map = 1,
    set_associative,
    full_associative
};

// replacement strategies
enum replacement_way{
    none, // 直接替換
    FIFO,
    LRU
};

//write way
enum write_way{
    write_back,
    write_through
};

# endif // STRUCT_TYPE
using namespace std;
typedef enum associative ASSOC;
typedef enum replacement_way REPLACE;
typedef enum write_way WRITE;

// function
void CacheSimulation(string file);
int CacheInit(int c_size, int b_size, string m_method, string r_policy);
#endif