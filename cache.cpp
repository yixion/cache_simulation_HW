# include "cache.h"

using namespace std;

// cache specification
unsigned int long cache_size = 0; // 8, 16, 32, 64, 128, 256 (KB)
unsigned int long block_size = 0; // 4, 8, 16, 32, 64, 128 (B)
unsigned int long cache_sets = 0; // index in the cache


unsigned int long num_block_cache = 0;

// output stuff
string input_file;
unsigned long int num_access = 0;
unsigned long int num_hit = 0;
unsigned long int num_miss = 0;
unsigned long int num_read = 0;
unsigned long int num_write = 0;
unsigned long int num_bytes_mem = 0;
unsigned long int num_bytes_cache = 0;

short unsigned int bit_block = 0;
short unsigned int bit_index = 0;
short unsigned int bit_tag = 0;

// statics
float hit_rate = 0.0;
float miss_rate = 0.0;

bitset<35> cache_item[MAX_CACHE_LINE]; //[34]:valid, [33]:hit, [32]:dirty, [31]-[0]: data
unsigned long int LRU_priority[MAX_CACHE_LINE];
unsigned long int current_line = 0;
unsigned long int current_set = 0;

ASSOC t_assoc;
REPLACE t_replace;
WRITE t_write = write_back;

int BitNumCal(unsigned long int number){
    int result = 0;
    while(number){
        number>>=1;
        result++;
    }
    result--;
    return result;
}

int CacheInit(int c_size, int b_size, string m_method, string r_policy){
    cache_size = c_size;
    block_size = b_size;
    num_block_cache = (cache_size << 10) / block_size; // cache 裡面有幾個 blocks
    cache_sets = num_block_cache;
    short unsigned int bit_block = BitNumCal(block_size);

    // 只需維護 cache 裡有幾個 blocks，因為一次就搬一個 block。
    for(int i=0; i < num_block_cache; i++){
        cache_item[i].reset(); // [31]:valid, [30]: hit, [29]:dirty, [28]-[0]:data
        if(r_policy == "LRU"){
            LRU_priority[i] =0;
        }
    }

    // check which map type that cache belong
    if(m_method == "1"){
        t_assoc = direct_map;
    }else if(m_method == "f"){
        t_assoc = full_associative;
    }else{
        t_assoc = set_associative;
    }

    // calculate the index bit
    if(t_assoc == direct_map){
        bit_index = BitNumCal(cache_sets); // index is how much block in the cache, but also how much sets in the cache. I mean direct mapped block size equals to set size.
    }else if(t_assoc == set_associative){
        cache_sets /= stoi(m_method); // index is how much sets in the cache
        bit_index = BitNumCal(cache_sets);
    }else{
        cache_sets = 1ul;
        bit_index = BitNumCal(cache_sets);
    }

    bit_tag = 32ul - bit_block - bit_index;
    if(r_policy == "LRU"){
        t_replace = LRU;
    }else if(r_policy == "FIFO"){
        t_replace = FIFO;
    }

}

void CacheSimulation(string file){
    input_file = file;
    for(int i=0; i < num_block_cache; i++){
        cache_item[i][31] = true;
    }
    ifstream src_file;
    string input_path = "./test_data/"+file;
    src_file.open(input_path);
    short unsigned int wr_bit;
    if(src_file.is_open()){
        string input;
        while(getline(src_file, input)){
            if(input.length() == 1){
                wr_bit = stoi(input);
            }else{
                bitset<32> address(stoi(input, nullptr, 16));
                // cout<< address<<endl;
                // return;
            }
        }

    }
    src_file.close();
}

void cache_process(string address, short unsigned int rw_bit){
    bool hit = false;
    bool is_write = false;
    bool is_read = false;
    bool is_iload = false;
    switch (rw_bit)
    {
    case 0:
        is_read = true;
        break;
    case 1:
        is_write = true;
        break;
    case 2:
        is_iload = true;
    default:
        break;
    }
    bitset<32> flags(stoi(address, nullptr, 16)); // 轉成 2 進位
    hit = IsHit(flags);
    if(hit && is_iload){
        num_hit++;
        num_access++;

    }else if(hit && is_read){
        num_read++;
        num_access++;
        num_hit++;
    }else if(hit && is_write){
        num_write++;
        num_bytes_cache++;
        num_hit++;
    }else if((!hit) && is_iload){
        num_access++;
        num_miss++;
    }else if((!hit) && is_read){ // not hit, fetch instruction
        num_access++;
        num_miss++;
        num_read++;
        mem_Read(flags); // read data from memory
    }else if((!hit) && is_write){
        num_access++;
        num_write++;
        num_miss++;

    }

}

bool IsHit(bitset<32> flag){
    bool result = false;
    unsigned long int i, j;
    if(t_assoc == direct_map){
        bitset<32> flags_line; // a temp variable
        for(j=0, i = (bit_block); i<(bit_block + bit_index); j++, i++){
            flags_line[j] = flag[i];
        }
        current_line = flags_line.to_ulong();
        if (cache_item[current_line][30] == true){
            result = true;
            for(i = 31, j = 28; i > (31ul - bit_tag); i--, j--){ // 比 tag
                if(flag[i] != cache_item[current_line][j]){
                    result = false;
                    break;
                }
            }
        }
    }
    else if(t_assoc == full_associative){
        unsigned long int temp;
        for(temp=0; temp < num_block_cache; temp++){
            if(cache_item[temp][30] == true){
                result = true;
                for(i = 31, j=28; i > (31ul - bit_tag); i--, j--){ // 比 tag
                    if(flag[i] != cache_item[temp][j]){
                        result = false;
                        break;
                    }
                }
            }
            if(result == true){
                current_line = temp;
                break;
            }
        }
    }else if(t_assoc == set_associative){
        bitset<32> flags_set;
        unsigned long int temp;
        for(j=0, i=(bit_block); i < (bit_block + bit_index); j++, i++){
            flags_set[j] = flag[i];
        }
        current_set = flags_set.to_ulong();
        for(temp = (current_set * bit_index); temp <((current_set + 1) * bit_index); temp++){ // 比 set
            if(cache_item[temp][30] == true){ 
                result = true;
                
                for(i=31, j=28; i > (31ul - bit_tag); i--, j--){ // 比 tag
                    if(flag[i] != cache_item[temp][j]){
                        result = false;
                        break;
                    }
                }
            }

            if(result == true){
                current_line = temp;
                break;
            }
        }
    }
    
    return result;
}

void mem_Read(bitset<32> flag){

}

void LRUHitProcess(){
    unsigned long int i, j;
    if(t_assoc == full_associative){
        for(i=0; i < num_block_cache; i++){
            if(LRU_priority[i] < LRU_priority[current_line] && cache_item[current_line][30] == true){
                LRU_priority[i]++;
            }
        }
        LRU_priority[current_line] = 0;
    }
}


void Result_output(){
    cout<<"Input file = "<< input_file << endl;
    cout<<"Demand fetch = " << num_access << endl;
    cout<<"Cache hit = " << num_hit << endl;
    cout<<"Cache miss = " << num_miss << endl;
    cout<<"Miss rate = "<<num_miss/num_access<<endl;
    cout<<"Read data = " << num_read << endl;
    cout<<"Write data = "<< num_write<< endl;
    num_bytes_mem = num_miss * block_size;
    cout<<"Bytes from memory = " << num_bytes_mem << endl;
    cout<<"Bytes to memory = " << num_bytes_cache << endl;
}