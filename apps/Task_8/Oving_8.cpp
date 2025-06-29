


#include <iostream>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <limits>
#include <functional>
#include <algorithm>
#include <math.h>



template <class T>
class que {
private:
    typedef struct _node {
        T val;
        _node* next;
    } node;

    node* head = 0;
    node* tail = 0;

    std::mutex read_lock;
    std::condition_variable cv;

public:
    que() {
        head = new node;
        head->next = head;
        tail = head;
    }

    ~que() {
        cv.notify_all();
        // std::this_thread::yield();

        node* n = head;
        node* end = head;

        do {
            node* temp = n;
            n = n->next;
            delete temp;
        } while (n != end);
    }

    inline void put(T val) {
        read_lock.lock(); // Lock this local scope

        if (head->next == tail) { // Check if we have no more space
            head->next = new node;
            head->next->next = tail;
        }
        
        head->next->val = val; // Set value in the next node.
        head = head->next; // Move the head forward.

        // Manual unlocking is done before notifying, to avoid waking up the waiting thread only to block again
        read_lock.unlock();
        cv.notify_one();
    }

    T wait_next() {
        std::unique_lock<std::mutex> lk(read_lock);

        bool locked = head == tail;

        if (locked)
            cv.wait(lk);

        if (tail != head)
            tail = tail->next;

        T result = tail->val;

        if (locked) lk.unlock();
        
        return result;
    }

    T next() {
        std::lock_guard<std::mutex> lock(read_lock); // Lock this local scope

        if (tail != head)
            tail = tail->next;

        return tail->val;
    }

    inline bool empty() {
        std::lock_guard<std::mutex> lock(read_lock); // Lock this local scope

        return head == tail;
    }
};



template <class T>
class stack {
private:
    typedef struct node {
        T val;
        node* previous;

        node(T v) : node(v, 0) {}
        node(T v, node* p) : val(v), previous(p) {}
    } node;

    node*  head  = 0;
    size_t _size = 0;

public:
    ~stack() {
        while (head) {
            node* n = head;
            head = head->previous;
            delete n;
        }
    }

    void push(T val) {
        // Create a new node that points back to the previous head
        head = new node(val, head);

        // Increment the size counter
        _size++;
    }

    void push_sort(T val, const std::function<bool(const T&, const T&)>& cmp) {
        if (head == 0 || cmp(val, head->val)) {
            push(val);
            return;
        }

        // Iterator
        node* n = head;

        // Seek to the correct position
        while (n->previous && cmp(n->previous->val, val))
            n = n->previous;
        
        // Create new node
        n->previous = new node(val, n->previous);

        // Increment the size counter
        _size++;
    }

    void push_sort_ascending(T val) {
        if (head == 0 || head->val >= val) {
            push(val);
            return;
        }

        // Iterator
        node* n = head;

        // Seek to the correct position
        while (n->previous && n->previous->val < val)
            n = n->previous;
        
        // Create new node
        n->previous = new node(val, n->previous);

        // Increment the size counter
        _size++;
    }

    T pop() {
        // Copy the head node's value
        T pop_val = head->val;
        
        // Copy the pionter to the head node
        node* current = head;

        // Move the head back one position
        head = head->previous;

        // Delete the current head node
        delete current;

        // Decrement the size counter
        _size--;

        // Return the value of the deleted head
        return pop_val;
    }

    T peek() const { return head->val; }

    size_t size() const { return _size; }
};



template <class T>
class circular_list {
public:
    typedef struct node {
        T val;
        node* next;

        node() {};
        node(T val) : val(val) {};
    } node;

private:
    node* start;
    const size_t size;

public:

    circular_list() : size(1) {
        start = new circular_list::node();
    };



    circular_list(size_t size) : circular_list() {
        this->size = size;

        circular_list::node* n = start;

        for (size_t i = 1; i < size; i++) {
            n->next = new circular_list::node();
            n = n->next;
        }

        n->next = start;
    };



    circular_list(const circular_list& other) {
        this->size = other.size;
        start = new circular_list::node(other.start->val);

        circular_list::node* n = start;
        circular_list::node* n_other = other.start;
        for (size_t i = 1; i < size; i++) {
            n->next = new circular_list::node(n_other->next->val);

            n = n->next;
            n_other = n_other->next;
        }

        n->next = start;
    };



    ~circular_list() {
        circular_list::node* n = start;

        do {
            circular_list::node* next = n->next;
            delete n;
            n = next;
        } while (n != start);
    }

    circular_list::node get_start_node() const {
        return start;
    }



    void foreach(const std::function<void(T&)>& f) {
        circular_list::node* n = start;

        do {
            f(n->val);
            n = n->next;
        } while (n != start);
    }
};



// The data type used to represent one 'byte' in the huffman-encoding
#define SYMBOL uint8_t

// The data type used to represent one 'byte' in the huffman-encoding
#define BITCODE unsigned long long

typedef struct char_count {
    SYMBOL symbol = 0;
    size_t count = 0;

    bool operator<(const char_count& other) { return count < other.count; }
    bool operator>(const char_count& other) { return count > other.count; }
} char_count;

std::vector<char_count> count_occurrences(FILE* src) {
    static const int BUFFER_SIZE = 1024*1024; // Buffer size of 1 MB
    static const int NUM_POSSIBLE_SYMBOLS = 1 << (sizeof(char_count::symbol) * 8);

    char_count* occurrences = new char_count[NUM_POSSIBLE_SYMBOLS];

    SYMBOL* buffer = new SYMBOL[BUFFER_SIZE];

    bool eof = false;

    while (!eof) {
        size_t read = fread(buffer, sizeof(SYMBOL), BUFFER_SIZE, src);

        if (read != BUFFER_SIZE) eof = true;

        for (size_t i = 0; i < read; i++)
            occurrences[buffer[i]].count++;
    }

    delete[] buffer;

    std::vector<char_count> result;

    for (size_t i = 0; i < NUM_POSSIBLE_SYMBOLS; ++i) {
        if (occurrences[i].count == 0) continue;

        occurrences[i].symbol = (SYMBOL) i;
        result.push_back(occurrences[i]);
    }

    delete[] occurrences;

    return result;
}



typedef struct tree_node {
    const bool leaf = true;

    tree_node* n1 = 0;
    tree_node* n2 = 0;
    const size_t sum = 0;
    const SYMBOL symbol = 0;

    tree_node() : leaf(false), sum(0), symbol(0), n1(0), n2(0) {}
    tree_node(SYMBOL c) : leaf(true), sum(0), symbol(c) {}
    tree_node(char_count c) : leaf(true), sum(c.count), symbol(c.symbol) {}
    tree_node(tree_node* n1, tree_node* n2) : leaf(false), n1(n1), n2(n2), sum(n1->sum + n2->sum) {}
    
    bool operator<(const tree_node& other) { return sum < other.sum; }
    bool operator>(const tree_node& other) { return sum > other.sum; }

    ~tree_node() {
        if (n1) delete n1;
        if (n2) delete n2;
    }

} tree_node;

bool tree_node_less_than(tree_node* t1, tree_node* t2) {
    return t1->sum < t2->sum;
}

tree_node* create_huffman_tree(std::vector<char_count> chars) {
    stack<tree_node*> stack;

    // Sort the symbol frequencies into the queue
    for (auto& c : chars)
        stack.push_sort(new tree_node(c), tree_node_less_than);
    
    while (stack.size() > 1) {
        tree_node* t1 = stack.pop();
        tree_node* t2 = stack.pop();
        stack.push_sort(new tree_node(t1, t2), tree_node_less_than);
    }

    return stack.pop();
}



typedef struct dictionary_entry {
    SYMBOL symbol;
    BITCODE code;
    int code_length;

} dictionary_entry;

void create_dictionary_traverse(tree_node* n, std::vector<dictionary_entry>& list, BITCODE code, int code_length) {
    if (n->leaf) {
        dictionary_entry e;
        e.symbol = n->symbol;
        e.code = code;
        e.code_length = code_length;
        list.push_back({ n->symbol, code, code_length });
        return;
    }

    code <<= 1;
    ++code_length;

    SYMBOL code_n1 = code | 0;
    SYMBOL code_n2 = code | 1;

    create_dictionary_traverse(n->n1, list, code | 0, code_length);
    create_dictionary_traverse(n->n2, list, code | 1, code_length);
}

std::vector<dictionary_entry> huffman_encode(tree_node* huffmann_tree) {
    std::vector<dictionary_entry> list;

    create_dictionary_traverse(huffmann_tree, list, 0, 0);

    return list;
}

std::vector<dictionary_entry> create_dictionary(tree_node* huffmann_tree) {
    std::vector<dictionary_entry> list = huffman_encode(huffmann_tree);

    std::vector<dictionary_entry> dictonary(1 << (sizeof(char_count::symbol) * 8));

    for (const dictionary_entry& d : list)
        dictonary[d.symbol] = d;

    return dictonary;
}



class bit_stream {
private:
    static const decltype(dictionary_entry::code) MASK_RESET = 1 << (sizeof(uint8_t) * 8 - 1);

    que<uint8_t> byte_que;
    
    // The mask used for reading bits from the queue
    uint8_t mask_in = MASK_RESET;

    // The mask used for writing bits to the queue
    uint8_t mask_out = MASK_RESET;

    // The intermediate byte for reading from the queue
    uint8_t intermediate_in = 0;

    // The intermediate byte for writing to the queue
    uint8_t intermediate_out = 0;

public:
    void write_bit(bool bit) {
        // Write the bit to the intermediate byte
        if (bit) intermediate_out |= mask_out;

        // Move mask one position to the right
        mask_out >>= 1;

        // If we have traversed the whole mask
        if (!mask_out) {

            // Push intermediate byte to the queue
            byte_que.put(intermediate_out);
        
            // Reset the intermediate byte
            intermediate_out = 0;

            // Reset the mask if we have traversed it completely
            mask_out = MASK_RESET;
        }
    }

    void write_byte(uint8_t byte) {
        const int mask_pos = (int) log2f(mask_out) + 1;

        intermediate_out |= byte >> (8 - mask_pos);

        byte_que.put(intermediate_out);
        
        intermediate_out = byte << mask_pos;
    }

    /// @brief Sends all bits stored in the intermediate byte to the queue, making them available to read. 
    /// The unwritten bits in the resulting byte are set to 0.
    /// If no bits are written to the intermediate byte, no byte is written to the queue and the function returns 0.
    /// @return The number of bits added to send the intermediate byte to the queue.
    int flush_byte() {
        // The number of bits added to send the intermediate byte to the queue
        int result = ((int) log2f(mask_out) + 1) & 7;

        if (mask_out != MASK_RESET)
            byte_que.put(intermediate_out);
        
        intermediate_out = 0;
        mask_out = MASK_RESET;

        return result;
    }

    bool read_bit() {
        // Read next byte if needed
        if (mask_in == MASK_RESET)
            intermediate_in = byte_que.wait_next();

        // Read the next bit
        bool result = intermediate_in & mask_in;

        // Move mask one position to the right
        mask_in >>= 1;

        // Reset the mask if we have traversed it completely
        if (!mask_in)
            mask_in = MASK_RESET;

        return result;
    }
    
    uint8_t read_byte() {
        const int mask_pos = ((int) log2f(mask_in) + 1) & 7;

        uint8_t next = byte_que.wait_next();

        uint8_t left = intermediate_in << (8 - mask_pos);
        uint8_t right = next >> mask_pos;
        uint8_t result = left | right;

        intermediate_in = next;

        return result;
    }

    int available_bits() {
        return (((int) log2f(mask_in) + 1) & 7) + !byte_que.empty() * 8;
    }

    bool available_byte() {
        return !byte_que.empty();
    }
};



class bit_reader {
    static const decltype(dictionary_entry::code) MASK_RESET = 1 << (sizeof(uint8_t) * 8 - 1);

    // The mask used for reading bits from the queue
    uint8_t mask_in = MASK_RESET;

    // The intermediate byte for reading from the queue
    uint8_t intermediate_in = 0;

    const uint8_t* begin;
    const uint8_t* end;
    uint8_t* read_pos;

public:
    bit_reader(uint8_t* begin, uint8_t* end): begin(begin), end(end), read_pos(begin) {}
    bit_reader(uint8_t* begin, uint8_t* end, int padded_bits): begin(begin), end(end), read_pos(begin) {}

    bool read_bit() {
        // Read next byte if needed
        if (mask_in == MASK_RESET)
            intermediate_in = *read_pos++;

        // Read the next bit
        bool result = intermediate_in & mask_in;

        // Move mask one position to the right
        mask_in >>= 1;

        // Reset the mask if we have traversed it completely
        if (!mask_in)
            mask_in = MASK_RESET;

        return result;
    }
    
    uint8_t read_byte() {
        const int mask_pos = ((int) log2f(mask_in) + 1) & 7;

        uint8_t next = *read_pos++;

        uint8_t left = intermediate_in << (8 - mask_pos);
        uint8_t right = next >> mask_pos;
        uint8_t result = left | right;

        intermediate_in = next;

        return result;
    }

    size_t available_bits() {
        return (((int) log2f(mask_in) + 1) & 7) + available_bytes() * 8;
    }

    size_t available_bytes() {
        return end - read_pos;
    }
};

class bit_writer {
    static const decltype(dictionary_entry::code) MASK_RESET = 1 << (sizeof(uint8_t) * 8 - 1);

    // The mask used for reading bits from the queue
    uint8_t mask_out = MASK_RESET;

    // The intermediate byte for reading from the queue
    uint8_t intermediate_out = 0;

    const uint8_t* begin;
    const uint8_t* end;
    uint8_t* write_pos;

public:
    bit_writer(uint8_t* begin, uint8_t* end): begin(begin), end(end), write_pos(begin) {}
    bit_writer(uint8_t* begin, uint8_t* end, int padded_bits): begin(begin), end(end), write_pos(begin) {}

    void write_bit(bool bit) {
        // Write the bit to the intermediate byte
        if (bit) intermediate_out |= mask_out;

        // Move mask one position to the right
        mask_out >>= 1;

        // If we have traversed the whole mask
        if (!mask_out) {

            // Push intermediate byte to the queue
            *write_pos++ = intermediate_out;
        
            // Reset the intermediate byte
            intermediate_out = 0;

            // Reset the mask if we have traversed it completely
            mask_out = MASK_RESET;
        }
    }

    void write_byte(uint8_t byte) {
        const int mask_pos = (int) log2f(mask_out) + 1;

        intermediate_out |= byte >> (8 - mask_pos);

        *write_pos++ = intermediate_out;
        
        intermediate_out = byte << mask_pos;
    }

    size_t byte_space_left() {
        return end - write_pos;
    }

    size_t bytes_written() {
        return write_pos - begin;
    }

    /// @brief Sends all bits stored in the intermediate byte to the queue, making them available to read. 
    /// The unwritten bits in the resulting byte are set to 0.
    /// If no bits are written to the intermediate byte, no byte is written to the queue and the function returns 0.
    /// @return The number of bits added to send the intermediate byte to the queue.
    int flush_byte() {
        // The number of bits added to send the intermediate byte to the queue
        int result = ((int) log2f(mask_out) + 1) & 7;

        if (mask_out != MASK_RESET)
            *write_pos++ = intermediate_out;
        
        intermediate_out = 0;
        mask_out = MASK_RESET;

        return result;
    }
};



void encode_huffmann_tree(const tree_node* huffmann_tree, bit_stream& bits) {
    bits.write_bit(huffmann_tree->leaf);
    
    if (huffmann_tree->leaf)
        bits.write_byte(huffmann_tree->symbol);
    
    else {
        encode_huffmann_tree(huffmann_tree->n1, bits);
        encode_huffmann_tree(huffmann_tree->n2, bits);
    }
}



std::vector<dictionary_entry> decode_dictionary(bit_reader& reader) {
    std::vector<dictionary_entry> dictionary;

    decltype(dictionary_entry::code_length) depth = 0;
    decltype(dictionary_entry::code) bitcode = 0;

    do {
        // Find first leaf node
        while (!reader.read_bit()) {
            bitcode <<= 1;
            depth++;
        }

        // Store the bitcode and symbol
        dictionary.push_back({ reader.read_byte(), bitcode, depth });

        // Find the bit-shift size required to reach the first zero
        int move = 0;
        while ((bitcode >> move) & 1) move++;

        // Discard all 1-bits to the right of the discovered zero
        bitcode >>= move;

        // Set the LSB to 1
        bitcode |= 1;

        // Record the upward move
        depth -= move;
    } while (depth);

    return dictionary;
}

void set_leaf_node(tree_node* top, BITCODE code, uint8_t bit, const SYMBOL& s) {
    bool right = code & (1 << bit);

    tree_node** next_node = right ? &top->n2 : &top->n1;

    if (bit) {
        if (!*next_node)
            *next_node = new tree_node;
        set_leaf_node(*next_node, code, --bit, s);
    } else {
        *next_node = new tree_node(s);
        return;
    }

    
}

tree_node* decode_huffmann_tree(const std::vector<dictionary_entry>& dictionary) {
    tree_node* top = new tree_node;

    for (const auto& e : dictionary)
        set_leaf_node(top, e.code, e.code_length - 1, e.symbol);

    return top;
}






void write_compression(FILE* src, FILE* dst, const tree_node* huffmann_tree, const std::vector<dictionary_entry>& entries) {
    fseek(src, 0, SEEK_SET);
    fseek(dst, 0, SEEK_SET);

    static const size_t BUFFER_SIZE = 1 << 20;
    SYMBOL* buffer = new SYMBOL[BUFFER_SIZE];
    
    bit_stream bits;
    size_t bytes_read;

    encode_huffmann_tree(huffmann_tree, bits);

    do {
        bytes_read = fread(buffer, sizeof(buffer[0]), BUFFER_SIZE, src);
        
        for (int i = 0; i < bytes_read; i++) {
            SYMBOL s = buffer[i];
            const dictionary_entry& e = entries[s];
            for (int bit = e.code_length - 1; bit >= 0; bit--)
                bits.write_bit(e.code & (1 << bit));
        }
        
        size_t bytes_to_write = 0;

        while (bits.available_byte()) {
            buffer[bytes_to_write] = bits.read_byte();
            bytes_to_write++;
        }

        fwrite(buffer, sizeof(buffer[0]), bytes_to_write, dst);

    } while (bytes_read == BUFFER_SIZE);

    uint8_t padded_bits_count = bits.flush_byte();

    buffer[0] = bits.read_byte();
    buffer[1] = padded_bits_count;
    fwrite(buffer, sizeof(buffer[0]), 2, dst);

    delete[] buffer;
}



void huffman_compress(FILE* src, FILE* dst) {
    // Count occurrences of all symbols
    std::vector<char_count> occurrences = count_occurrences(src);

    // Create Huffmann tree
    tree_node* huffmann_tree = create_huffman_tree(occurrences);
    
    // Create dictionary from Huffmann tree
    std::vector<dictionary_entry> entries = create_dictionary(huffmann_tree);

    // Write stream through the tree
    write_compression(src, dst, huffmann_tree, entries);

    fflush(dst);
}





void huffman_decompress(FILE* src, FILE* dst) {
    // Read the end of file to discover the exact amount of bits to read
    uint8_t padded_bits_count;
    fseek(src, -1, SEEK_END);
    fread(&padded_bits_count, 1, 1, src);

    // The size of the file in number of bytes
    //fseek(src, 0, SEEK_END);
    const size_t byte_count = ftell(src);
    uint8_t* in_buffer  = new uint8_t[byte_count];
    uint8_t* out_buffer = new uint8_t[byte_count];
    
    // Read the whole file to memory
    fseek(src, 0, SEEK_SET);
    fread(in_buffer, sizeof(uint8_t), byte_count, src);

    // Setup bit handlers
    bit_reader reader(in_buffer, in_buffer + byte_count, padded_bits_count);
    bit_writer writer(out_buffer, out_buffer + byte_count);

    tree_node* huffmann_tree = decode_huffmann_tree(decode_dictionary(reader));

    // Convert bitcodes to symbols
    size_t bits_left = reader.available_bytes() * 8 - padded_bits_count;
    while (bits_left) {
        // Iterator for the huffmann tree
        tree_node* n = huffmann_tree;

        // Read bits until we reach a leaf
        while (bits_left && !n->leaf) {
            n = reader.read_bit() ? n->n2 : n->n1;
            bits_left--;
        }
        
        // Write the symbol from the leaf
        if (n->leaf) writer.write_byte(n->symbol);
        
        // Make sure we don't run out of space in our allocated buffer, by writing to the file multiple times.
        if (!writer.byte_space_left()) {
            fwrite(out_buffer, sizeof(uint8_t), writer.bytes_written(), dst);

            writer = bit_writer(out_buffer, out_buffer + byte_count);
        }
    }

    delete huffmann_tree;
    
    // Push the last bytes to file
    size_t bytes_left_to_write = writer.bytes_written();
    if (bytes_left_to_write)
        fwrite(out_buffer, sizeof(uint8_t), bytes_left_to_write, dst);

    fflush(dst);

    delete[] in_buffer;
    delete[] out_buffer;
}



std::string to_bit_string(unsigned char val) {
    char chars[64 + 64/4 + 1]{0};

    int i = 0;

    for (decltype(val) flag = 1 << (sizeof(val) * 8 - 1); flag > 0; flag >>= 1, ++i) {
        chars[i] = val & flag ? '1' : '0';

        if (i % 5 == 3)
            chars[++i] = ' ';
    }

    return std::string(chars);
}



int command(int argc, char *argv[]) {
    static const char SYNTAX_ERROR_MSG[] = "Forventet syntaks: [-c/-d] kildefil destinasjonsfil\nEksempel for    komprimering: Oving_8.exe -c enwik8.txt enwik8_komprimert.hzp\nEksempel for de-komprimering: Oving_8.exe -d enwik8_komprimert.hzp enwik8_dekomprimert.txt";

    if (argc < 3+1) {
        printf("%s\n", SYNTAX_ERROR_MSG);
        return 1;
    }

    int request = 0;
    char* src_path = 0;
    char* dst_path = 0;

    for (int i = 1; i < argc; i++) {
        char* c = argv[i];

        if (*c == '-') {
            switch (c[1]) {

            case 'c': request = 1; break;
            case 'd': request = 2; break;

            }
        } else {
            if (!src_path) src_path = c;
            else
            if (!dst_path) dst_path = c;
        }

    }
        
    if (!request | !src_path | !dst_path) {
        printf("%s\n", SYNTAX_ERROR_MSG);
        return 1;
    }

    

    // Open files

    FILE* src = fopen(src_path, "rb");
    FILE* dst = fopen(dst_path, "wb");

    if (!src) printf("Klarte ikke å åpne fil: %s", src_path);
    if (!dst) printf("Klarte ikke å åpne fil: %s", dst_path);

    char* failure = 0;
    if (!dst) failure = dst_path;
    if (!src) failure = src_path;

    if (failure) {
        printf("Klarte ikke å åpne fil: %s", failure);
        fclose(src);
        fclose(dst);
        return 2;
    }


    
    // Perform compression / decompression
    
    printf("%s: %s -> %s...", request == 1 ? "Compress" : "Decompress", src_path, dst_path);

    if (request == 1)
        huffman_compress(src, dst);
    else
        huffman_decompress(src, dst);

    fclose(src);
    fclose(dst);

    printf(" DONE\n");

    return 0;
}



int main(int argc, char *argv[]) {
    return command(argc, argv);
}