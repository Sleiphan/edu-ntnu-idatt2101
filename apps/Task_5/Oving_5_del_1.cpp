#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdint.h>

char* read_entire_file(const char* file_path) {
    errno = 0;
    FILE* f = fopen(file_path, "r");

    if (!f) {
        fprintf(stderr, "Failed to open file: \"%.64s\"\n", file_path);
        return nullptr;
    }

    fseek(f, 0L, SEEK_END);
    size_t buf_size = ftell(f);
    rewind(f);
    
    char* buf = new char[buf_size + 1];

    if (fread(buf, sizeof(buf[0]), buf_size, f) != buf_size) {
        fprintf(stderr, "Failed to read file: \"%.64s\"\n", file_path);
        return nullptr;
    }

    fclose(f);
    
    buf[buf_size] = '\0';
    return buf;
}



std::vector<std::string> read_lines(const char* file_path) {
    char* content = read_entire_file(file_path);

    if (!content)
        return std::vector<std::string>();
    
    char* it1 = content;
    char* it2 = content;

    std::vector<std::string> result;

    bool end_reached = false;

    while (!end_reached) {
        while (*it2 != '\n' && *it2 != '\0')
            it2++;

        if (*it2 == '\0')
            end_reached = true;
        else {
            *it2 = '\0';
            result.push_back(std::string(it1));
        }

        it2++;
        it1 = it2;
        
    }

    delete[] content;

    return result;
}


template <class T>
int msb(T val) {
    const int max_bit = sizeof(T) * 8 - 1;
    const T   max_val = (T)1 << (sizeof(T) * 8) - 1;

    int r = max_bit;
    T mask = max_val;
    
    while ((mask & val) == 0 && r > 0) {
        r--;
        mask >>= 1;
    }

    return r;
}

template <class T>
T multhash(T k, int bit_resolution) {
    return (k * 2'654'435'769) >> (sizeof(T) * 8 - bit_resolution);
}

size_t hash_string(const char* s) {
    const static int prime = 53;
    unsigned long long h = 0;
    while (*s)
        h = h * prime + (unsigned) *s++;
    return h;
};



class hash_table {
private:
    typedef struct node {
        std::string key;
        int32_t val;
        node* next = 0;

        node() {}
        node(std::string key, int32_t val, node* next) : key(key), val(val), next(next) {}
    } node;

    size_t hash_func(std::string key) {
        return multhash(hash_string(key.c_str()), capacity_msb);
    }

    std::vector<node*> table;
    const int capacity_msb;

public:
    hash_table(size_t capacity) : capacity_msb(msb(capacity) + 1), table(1 << (msb(capacity) + 1)) {
        for (auto it = table.begin(); it != table.end(); it++)
            *it = nullptr;
    }

    std::string put(std::string key, int32_t val) {
        size_t i = hash_func(key);
        table[i] = new node(key, val, table[i]);

        if (table[i]->next)
            return table[i]->next->key;
        else
            return std::string();
    }

    int32_t get(std::string key) {
        size_t i = hash_func(key);

        node* get_node = table[i];
        
        while (get_node && key != get_node->key)
            get_node = get_node->next;

        return get_node ? get_node->val : -1;
    }

    size_t count_collisions() {
        size_t count = 0;

        node* current_node = 0;
        
        for (auto it = table.begin(); it != table.end(); it++) {
            current_node = *it;

            while (current_node && current_node->next) {
                count++;
                current_node = current_node->next;
            }
        }

        return count;
    }

    size_t values_count() {
        size_t count = 0;
        node* current_node = 0;

        for (auto it = table.begin(); it != table.end(); it++) {
            current_node = *it;

            while (current_node) {
                count++;
                current_node = current_node->next;
            }
        }

        return count;
    }

    float load_factor() {
        return (float) ((long double) values_count() / table.size());
    }
};



void demo_hash_table_string() {
    std::vector<std::string> lines = read_lines("navn.txt");

    hash_table t(lines.size());

    for (int i = 0; i < lines.size(); i++) {
        std::string collision = t.put(lines[i], i);
        if (collision.length())
            std::cout << lines[i] + " =/= " + collision + "\n";
    }
    
    bool test_success = true;
    for (int i = 0; i < lines.size(); i++)
        if (t.get(lines[i]) != i) {
            test_success = false;
            break;
        }
    
    std::string test_msg = test_success ? "success" : "FAILED";



    std::cout << "Test:                   " + test_msg + "\n";
    std::cout << "Load factor:            " + std::to_string(t.load_factor()) + "\n";
    std::cout << "Number of collisions:   " + std::to_string(t.count_collisions()) + "\n";
    std::cout << "Number of elements:     " + std::to_string(t.values_count()) + "\n";
    std::cout << "Collisions per element: " + std::to_string((long double) t.count_collisions() / t.values_count()) + "\n";
}



int main() {
    demo_hash_table_string();

    return 0;
}