#include "Example_library/sum.hpp"

long sum(int* numbers, size_t number_count) {
    long sum = 0;
    
    int* end = numbers + number_count;

    while (numbers != end)
        sum += *(numbers++);
    
    return sum;
}