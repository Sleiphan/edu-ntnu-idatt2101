#include <iostream>
#include <string>
#include <functional>
#include <exception>
#include <chrono>
#include <stdio.h>
#include <vector>



template <class T>
class ring_buffer {
private:
    typedef struct node {
        T val;
        node* next;

        node() {};
        node(T val) : val(val) {};
    } node;

    node* current;
    node* last;
    size_t size;

public:

    ring_buffer() : size(1) {
        current = new node();
        last = current;
        
        current->next = last;
        last->next = current;
    };

    ring_buffer(size_t size) : ring_buffer() {
        this->size = size;

        node* n = current;

        for (size_t i = 1; i < size; i++) {
            n->next = new node();
            n = n->next;
        }

        last = n;
        n->next = current;
    };

    ring_buffer(const ring_buffer& other) {
        this->size = other.size;
        current = new node(other.current->val);

        node* n = current;
        node* n_other = other.current;
        for (size_t i = 1; i < size; i++) {
            n->next = new node(n_other->next->val);

            n = n->next;
            n_other = n_other->next;
        }

        last = n;
        n->next = current;
    };

    ~ring_buffer() {
        node* n = current;

        do {
            node* next = n->next;
            delete n;
            n = next;
        } while (n != current);
    }

    void foreach(const std::function<void(T&)>& f) {
        node* n = current;

        do {
            f(n->val);
            n = n->next;
        } while (n != current);
    }

    T pop() {
        if (last == current)
            return current->val;
    
        // Make 'last' point to the next in the que
        last->next = current->next;

        // Save the value to be deleted
        T del_val = current->val;

        // Delete the current element
        delete current;

        // Set the current element to the next element
        current = last->next;

        size--;
    
        return del_val;
    }
    
    void insert(T val) {
        node* next = current->next;
        current->next = new node(val);
        current->next->next = next;
        size++;

        ++*this;
    }
    
    T& operator->() { return current->val; };
    T& operator*() { return current->val; };
    
    ring_buffer& operator++() {
        last = current;
        current = current->next;
        return *this;
    };
    
    ring_buffer operator++(int) {
        ring_buffer temp = *this;
        ++*this;
        
        return temp;
    };
    
    ring_buffer& operator+=(long long count) {
        if (count < 0)
            throw std::invalid_argument("Cannot walk backwards in a one-directional linked list");

        count %= size;

        for (; count > 1; count--)
            current = current->next;
        
        if (count > 0)
            ++*this;
        
        return *this;
    };
    
    bool operator==(const ring_buffer& other) { return current == other.current; };
    bool operator!=(const ring_buffer& other) { return current != other.current; };
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

    node* head;
    size_t _size;

public:
    stack() : _size(0), head(0) {}

    ~stack() {
        while (head) {
            node* n = head;
            head = head->previous;
            delete n;
        }
    }

    void push(T val) {
        // Copy the pinter to the current head node
        node* prev = head;

        // Create a new node that points back to the previous head
        head = new node(val, prev);

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



static const short O4_FILE_NOT_FOUND = 2;
static const short O4_INVALID_SYNTAX = 1;
static const short O4_OK = 0;

int check_brk(const char* file_path) {
    FILE* f = fopen(file_path, "r");

    if (!f)
        return O4_FILE_NOT_FOUND;
    
    stack<char> stk;

    bool syntax_valid = true;
    int c;
    while (syntax_valid && (c = std::fgetc(f)) != EOF) {

        int end_bracket = 0;

        switch (c) {
            case '(':
            case '[':
            case '{':
                stk.push(c); break;
            case '}':
                end_bracket = '{'; break;
            case ']':
                end_bracket = '['; break;
            case ')':
                end_bracket = '('; break;
        }

        if (end_bracket != 0) {
            if (stk.size() == 0)
                syntax_valid = false;
            else
                syntax_valid &= stk.pop() == end_bracket;
        }
    }

    fclose(f);

    syntax_valid &= stk.size() == 0;

    return syntax_valid * O4_OK + !syntax_valid * O4_INVALID_SYNTAX;
}



int josephus_problem(int soldier_count, int interval) {
    if (soldier_count < 1)
        throw std::invalid_argument("Josephus problem require at least one solider. Negative values not supported.");
    
    if (soldier_count < 0)
        throw std::invalid_argument("A positive interval is required");

    // The circular list that keeps track of all the soldiers
    ring_buffer<int> list(soldier_count);

    // Fill it with the index of each soldier
    int i = 1;
    list.foreach([&i](int& val){ val = i++; });

    // Some fixing of the parameters to suit the behaviour of my 'ring_buffer' class
    soldier_count--; // We don't want to kill the last soldier
    interval--; // 'ring_buffer' puts the play head at the next node after a pop-operation
    interval = std::max(interval, 0); // Deal with the special case where the caller inputs interval=0

    // Let the soldiers kill themselves
    for (int i = 0; i < soldier_count; i++) {
        list += interval;
        list.pop(); 
    }

    // Return the last man standing
    return *list;
}



double measure_function_time(const std::function<void()>& func, int cycles) {
    // Start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Perform task
    for (int i = 0; i < cycles; i++)
        func();

    // Stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time in seconds
    double elapsed = (double) ((long double) (end_time - start_time).count() / 1'000'000'000);
    elapsed /= cycles;

    return elapsed;
}



void run_josephus_problem() {
    int soldier_count = 10;
    int interval = 4;

    std::cout << "Running Josephus Problem with " + std::to_string(soldier_count) + " soldiers and an interval of " + std::to_string(interval) + "...\n";
    int survivour = josephus_problem(soldier_count, interval);
    std::cout << "Soldier " + std::to_string(survivour) + " is the last man standing\n";
    
    soldier_count = 40;
    interval = 3;

    std::cout << "Running Josephus Problem with " + std::to_string(soldier_count) + " soldiers and an interval of " + std::to_string(interval) + "...\n";
    survivour = josephus_problem(soldier_count, interval);
    std::cout << "Soldier " + std::to_string(survivour) + " is the last man standing\n";
}



void print_complexity_josephus_problem() {
    int interval = 100;
    int soldiers = 100;

    int cycles = 10;
    
    for (soldiers = 10; soldiers < 1'000'000; soldiers *= 10) {
        double time_in_seconds = measure_function_time([&soldiers, interval](){ josephus_problem(soldiers, interval); }, cycles);
        printf("Soldiers= %-9d Interval= %-9d : %2.8fs\n", soldiers, interval, time_in_seconds);
    }
    std::cout << "\n";
    
    cycles = 10;
    for (soldiers = 10, interval = 10; soldiers < 1'00'000; soldiers *= 10, interval *= 10) {
        if (soldiers == 100'000)
            cycles = 1;

        double time_in_seconds = measure_function_time([&soldiers, interval](){ josephus_problem(soldiers, interval); }, cycles);
        printf("Soldiers= %-9d Interval= %-9d : %2.8fs\n", soldiers, interval, time_in_seconds);
    }
    std::cout << "\n";



    interval = 1'000;
    soldiers = 100;
    cycles = 10'000;
    
    for (interval = 100; interval < 1'000'000'000; interval *= 10) {
        double time_in_seconds = measure_function_time([&soldiers, interval](){ josephus_problem(soldiers, interval); }, cycles);
        printf("Soldiers= %-9d Interval= %-9d : %2.8fs\n", soldiers, interval, time_in_seconds);
    }
    std::cout << "\n";


    cycles = 1;
    soldiers = 10'000;

    for (interval = 10; interval < 1'000'000'000; interval *= 10) {
        double time_in_seconds = measure_function_time([&soldiers, interval](){ josephus_problem(soldiers, interval); }, cycles);
        printf("Soldiers= %-9d Interval= %-9d : %2.8fs\n", soldiers, interval, time_in_seconds);
    }
    std::cout << "\n";
}



void run_syntax_check() {
    std::string files[] = {
        "Correct_syntax_1.txt",
        "Correct_syntax_2.txt",
        "Wrong_syntax_1.txt",
        "Wrong_syntax_2.txt",
        "Wrong_syntax_3.txt",
        "Wrong_syntax_4.txt"
    };

    for (int i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
        int check = check_brk(files[i].c_str());

        if (check == O4_FILE_NOT_FOUND) {
            std::cout << "Failed to find the file '" + files[i] + "'\n";
            continue;
        }

        if (check != O4_OK && check != O4_INVALID_SYNTAX) {
            std::cout << "Unknown error opening the file '" + files[i] + "'\n";
            continue;
        }
             

        std::string state = check == O4_OK ? "correct" : "faulty ";
        std::cout << "The file '" + files[i] + "' contains " + state + " syntax\n";
    }
}



void complete_demo() {
    std::cout << "### Josephus Problem tests ###\n\n";
    run_josephus_problem();
    std::cout << "\n\n\n\n";

    
    std::cout << "### Josephus Problem complexity ###\n\n";
    print_complexity_josephus_problem();
    std::cout << "\n\n\n\n";

    
    std::cout << "### Syntax Validation tests ###\n\n";
    run_syntax_check();
    std::cout << "\n\n\n\n";
}



int main() {
    complete_demo();


    return 0;
}
