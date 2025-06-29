#include <iostream>
#include "Example_library/sum.hpp"

int main() {
    int arr[] = { 5, 6, 7 };
    
    printf("Hello World!\nThe sum of %d, %d, and %d is %zd\n", arr[0], arr[1], arr[2], sum(arr, 3));

    return 0;
}
