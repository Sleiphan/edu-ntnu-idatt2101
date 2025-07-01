

#include <random>
#include <vector>
#include <stdexcept>
#include <string>
#include <chrono>
#include <iostream>



template <class random_iterator>
void fill_with_test_data(random_iterator begin, random_iterator end) {
    const int mask = (1 << 16) - 1;
    *begin = std::rand() & mask;
    begin++;

    while (begin != end) {
        *begin = *(begin - 1) + 1 + (std::rand() & mask);
        begin++;
    }
}

template <class T>
void swap(T& v1, T& v2) {
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

template <class random_iterator>
void shuffle(random_iterator begin, random_iterator end) {
    random_iterator current = begin;
    size_t size = end - begin;

    while (current != end) {
        swap(*current, *(begin + (std::rand() % size)));
        current++;
    }
}

template <class T>
int msb(T val) {
    const int max_bit = sizeof(T) * 8 - 1;
    const T   max_val = (T)1 << max_bit;

    int r = max_bit;
    T mask = max_val;
    
    while ((mask & val) == 0 && r > 0) {
        r--;
        mask >>= 1;
    }

    return r;
}

inline size_t multhash(size_t k, const int bit_resolution) {
    // return k & ((1 << bit_resolution) - 1);
    return (k * 2'654'435'769) >> (sizeof(size_t) * 8 - bit_resolution);
}

bool is_prime(int n) {
    if (n <= 1)
        return false;
    
    for (int i = 2; i <= sqrt(n); i++)
        if (n % i == 0)
            return false;

    return true;
}
 
int nearest_prime(int n) {
    int prime = 0;

    for (int i = n-1; i >= 2; i--)
        if (is_prime(i)) {
            prime = i;
            break;
        }

    return prime;
}



class hash_table {
protected:
    typedef struct node {
        int key = -1;
        int val = 0;
    } node;

    node* data;
    const size_t data_size;

public:
    hash_table(size_t capacity) :
        data_size(capacity),
        data(new node[capacity]) {}

    ~hash_table() { delete[] data; }
    
    size_t size() { return data_size; }

    virtual int  get(int key) const = 0;
    virtual bool put(int key, int val) = 0;

    void clear() {
        node* p   = data;
        node* end = data + data_size;

        while (p != end) {
            p->key = -1;
            p->val = 0;
            ++p;
        }
    }
};



class hash_table_linear_probing : public hash_table {
protected:
    // const size_t cap_mask;

    inline size_t hash_func(int key) const {
        return key % data_size;
    }

public:
    // hash_table_linear_probing(size_t capacity) :
    //         hash_table(1 << (msb(capacity) + 1)),
    //         cap_mask((1 << (msb(capacity) + 1)) - 1) {}

    hash_table_linear_probing(size_t capacity) : hash_table(capacity) {}

    int get(int key) const override {
        const size_t h = hash_func(key);

        for (size_t i = h; i < data_size; ++i) {
            if (data[i].key == -1)  return -1;
            if (data[i].key == key) return data[i].val;
        }

        for (size_t i = 0; i < h; ++i) {
            if (data[i].key == -1)  return -1;
            if (data[i].key == key) return data[i].val;
        }

        return -1;
    }

    bool put(int key, int val) override {
        const size_t h = hash_func(key);
        node* hash_pos = data + h;

        if (hash_pos->key == -1) {
            hash_pos->key = key;
            hash_pos->val = val;
            return false;
        }

        node* p = hash_pos;
        node* end = data + data_size;

        while ((p != end) & (p->key != -1)) {
            ++p;
        }

        if (p->key != -1)
            p = data;

        while ((p != hash_pos) & (p->key != -1)) {
            ++p;
        }

        if (p->key != -1)
            throw std::runtime_error("This hash table is full");
        
        p->key = key;
        p->val = val;

        return true;
    }
};



class hash_table_double_hashing : public hash_table {
private:

protected:
    const size_t lower_prime;
    // const size_t cap_mask;

    inline size_t hash_func_1(int key) const {
        return key % data_size;
    }

    inline size_t hash_func_2(int key) const {
        return lower_prime - (key % lower_prime);
    }

public:
    // hash_table_double_hashing(size_t capacity) :
    //     hash_table((1 << (msb(capacity) + 1))), 
    //     lower_prime(nearest_prime((1 << (msb(capacity) + 1)))),
    //     cap_mask((1 << (msb(capacity) + 1)) - 1) {}

    hash_table_double_hashing(size_t capacity) :
        hash_table(capacity), 
        lower_prime(nearest_prime(capacity)) {}

    int get(int key) const override {
        const size_t h = hash_func_1(key);

        if (data[h].key == key)
            return data[h].val;

        const size_t i = hash_func_2(key);

        size_t p = (h + i) % data_size;

        while ((data[p].key != key) & (p != h))
            p = (p + i) % data_size;

        if (data[p].key != key)
            return -1;
        
        return data[p].val;
    }

    bool put(int key, int val) override {
        const size_t h = hash_func_1(key);

        if (data[h].key == -1) {
            data[h].key = key;
            data[h].val = val;
            return false;
        }

        const size_t i = hash_func_2(key);

        size_t p = (h + i) % data_size;

        while ((data[p].key != -1) & (p != h))
            p = (p + i) % data_size;

        if (data[p].key != -1)
            throw std::runtime_error("This hash table is full");
        
        data[p].key = key;
        data[p].val = val;

        return true;
    }
};



void hash_table_put_test(int* test_data_begin, int* test_data_end, hash_table* table) {
    size_t collisions = 0;
    int* p = test_data_begin;
    const size_t data_size = test_data_end - test_data_begin;


    // Start time measuring
    auto t_start = std::chrono::high_resolution_clock::now();
    // Put the data into the hash table
    while (p != test_data_end) {
        collisions += table->put(*p, *p);
        ++p;
    }
    // Calculate elapsed time, in seconds.
    double duration = ((long double) (std::chrono::high_resolution_clock::now() - t_start).count()) / 1'000'000'000;


    bool test = true;
    p = test_data_begin;

    // Test that the data can be retrieved correctly
    while (p != test_data_end) {
        test &= table->get(*p) == *p;
        ++p;
    }

    std::string test_msg = test ? "success" : "FAILED";

    std::cout << "Test:                   " + test_msg + "\n";
    std::cout << "Duration:               " + std::to_string(duration) + "\n";
    std::cout << "Load_factor:            " + std::to_string((double) data_size / table->size()) + "\n";
    std::cout << "Collisions:             " + std::to_string(collisions) + "\n";
    std::cout << "Collisions_per_element: " + std::to_string((long double)collisions / data_size) + "\n";
}



int main() {
    // std::vector<int> test_data((1 << 24) - 1);
    std::vector<int> test_data(10'000'019);

    int* begin = &test_data.front();
    int* end_100 = begin + (size_t) (test_data.size() * 1.00);
    int* end_99  = begin + (size_t) (test_data.size() * 0.99);
    int* end_90  = begin + (size_t) (test_data.size() * 0.90);
    int* end_80  = begin + (size_t) (test_data.size() * 0.80);
    int* end_50  = begin + (size_t) (test_data.size() * 0.50);

    fill_with_test_data(begin, end_100);
    shuffle(begin, end_100);

    hash_table_linear_probing t_linear(test_data.size());
    hash_table_double_hashing t_double(test_data.size());

    std::cout << "NB: The first test may run for a long time! About 40 seconds or so.\n";

    std::cout << "\nLinear probing - 100% of test data:\n"; hash_table_put_test(begin, end_100, &t_linear); t_linear.clear();
    std::cout << "\nLinear probing - 99%  of test data:\n"; hash_table_put_test(begin, end_99,  &t_linear); t_linear.clear();
    std::cout << "\nLinear probing - 90%  of test data:\n"; hash_table_put_test(begin, end_90,  &t_linear); t_linear.clear();
    std::cout << "\nLinear probing - 80%  of test data:\n"; hash_table_put_test(begin, end_80,  &t_linear); t_linear.clear();
    std::cout << "\nLinear probing - 50%  of test data:\n"; hash_table_put_test(begin, end_50,  &t_linear); t_linear.clear();

    std::cout << "\nDouble hashing - 100% of test data:\n"; hash_table_put_test(begin, end_100, &t_double); t_double.clear();
    std::cout << "\nDouble hashing - 99%  of test data:\n"; hash_table_put_test(begin, end_99,  &t_double); t_double.clear();
    std::cout << "\nDouble hashing - 90%  of test data:\n"; hash_table_put_test(begin, end_90,  &t_double); t_double.clear();
    std::cout << "\nDouble hashing - 80%  of test data:\n"; hash_table_put_test(begin, end_80,  &t_double); t_double.clear();
    std::cout << "\nDouble hashing - 50%  of test data:\n"; hash_table_put_test(begin, end_50,  &t_double); t_double.clear();

    return 0;
}
