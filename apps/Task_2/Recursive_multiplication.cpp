#include <iostream>
#include <chrono>
#include <string>





double mul_rec_1(long n, double x) {
    if (n <= 1)
        return x;
    
    return mul_rec_1(n - 1, x) + x;
}

double mul_rec_2(long n, double x) {
    if (n <= 1)
        return x;

    if (n & 1)
        return x + mul_rec_2((n-1)/2, x + x);
    
    return mul_rec_2(n/2, x + x);
}





long double time_test_m1(long calls, long n, double x) {
    std::cout << "Running " + std::to_string(calls) + " calls of function 1 with parameters n=" + std::to_string(n) + " x=" + std::to_string(x) + " ... ";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < calls; i++)
        mul_rec_1(n, x);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "complete\n";
    return (long double) (end - start).count() / 1000000;
}

long double time_test_m2(long calls, long n, double x) {
    std::cout << "Running " + std::to_string(calls) + " calls of function 2 with parameters n=" + std::to_string(n) + " x=" + std::to_string(x) + " ... ";
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < calls; i++)
        mul_rec_2(n, x);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "complete\n";
    return (long double) (end - start).count() / 1000000;
}





typedef struct data_point {
    long n; // Integer factor
    double x; // Floating point factor
    double r; // Expected result
} data_point;

// Test data til å sjekke 
data_point data_set[] = {
    { 1, 1, 1 },
    { 1, 2, 2 },
    { 2, 1, 2 },
    { 2, 2, 4 },
    { 31, 15, 465 },
    { 9, 1.1, 9.9 },
    { 37, 7643.2597, 282800.6089 }
};





int main() {

    const double tolerance = 0.000001;

    // Test method 1
    std::cout << "Method 1 test ... ";
    bool success = true;

    for (data_point d : data_set) {
        double result = mul_rec_1(d.n, d.x);

        if (result < d.r - tolerance || result > d.r + tolerance)
            success = false;
    }

    if (success)
        std::cout << "success\n";
    else
        std::cout << "FAILED\n";



    // Test method 2
    std::cout << "Method 2 test ... ";
    success = true;

    for (data_point d : data_set) {
        double result = mul_rec_2(d.n, d.x);

        if (result < d.r - tolerance || result > d.r + tolerance)
            success = false;
    }

    if (success)
        std::cout << "success\n";
    else
        std::cout << "FAILED\n";

    
    // Run time-analysis

    const long num_calls = 100000;
    const long highest_n = 10000; // BEMERK! Kan forårsake stack overflow; høyere enn 5 tusen
    const long num_tests = 4;

    long double timings_inner_m1[num_tests] = {
        time_test_m1(num_calls, highest_n / 1000,  1.1),
        time_test_m1(num_calls, highest_n / 100,   1.1),
        time_test_m1(num_calls, highest_n / 10,    1.1),
        time_test_m1(num_calls, highest_n,         1.1)
    };

    long double timings_inner_m2[num_tests] = {
        time_test_m2(num_calls, highest_n / 1000,  1.1),
        time_test_m2(num_calls, highest_n / 100,   1.1),
        time_test_m2(num_calls, highest_n / 10,    1.1),
        time_test_m2(num_calls, highest_n,         1.1)
    };
    
    std::cout << "### Method 1 ###\n";
    for (int i = 0; i < num_tests; i++)
        std::cout << std::to_string(timings_inner_m1[i]) + "\n";
    
    std::cout << "### Method 2 ###\n";
    for (int i = 0; i < num_tests; i++)
        std::cout << std::to_string(timings_inner_m2[i]) + "\n";
}
