#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <cstring>
#include <numeric>



template <class T>
void fill_with_test_data(T* data, size_t count) {
    srand((int) std::chrono::high_resolution_clock::now().time_since_epoch().count());

    T* end = data + count;

    for (T* it = data; it < end; it++)
        *it = (T) std::rand();
}

template <class T>
void fill_with_duplicate_test_data(T* data, size_t count) {
    srand((int) std::chrono::high_resolution_clock::now().time_since_epoch().count());

    T* end = data + count;
    T repeat = 42;

    for (T* it = data; it < end; it++) {
        if ((end - it) % 2)
            *it = repeat;
        else
            *it = std::rand();
    }
}

template <class T>
inline void bytt(T* data_1, T* data_2) {
    auto temp = *data_1;
    *data_1 = *data_2;
    *data_2 = temp;
}

template <class T>
T* splitt(T* data, size_t count) {
    T* last = data + count - 1;

    // Find a suitable pivot
    T* pivot = data + (count - 1) / 2;
    const T pivot_val = *pivot;

    // Define upper and lower iterators
    T* lower = data;
    T* upper = last - 1;

    // Store the pivot value in the last position
    bytt(pivot, last);
    
    // Perform the first seek for both iterators
    while (*lower < pivot_val && lower < last)
        lower++;
    while (*upper > pivot_val && upper > data)
        upper--;
    

    while (lower < upper) {
        bytt(lower, upper);
        while (*(++lower) < pivot_val);
        while (*(--upper) > pivot_val);
    }
    
    // Place the pivot value at a safe location
    bytt(lower, last);

    // Return that location
    return lower;
}

template <class T>
void custom_quick_sort(T* data, size_t count) {
    if (count < 1)
        return;

    T* end = data + count;
    T* last = end - 1;

    if (count > 2) {
        T* middle = splitt(data, count);
        custom_quick_sort(data, middle - data);
        middle++;
        custom_quick_sort(middle, end - middle);
    } else {
        if (*data > *last)
            bytt(data, last);
    }
}



template <class T>
void custom_quick_sort_improved_inner(T* data, size_t count, const T* bound_begin, const T* bound_end) {
    if (count < 1)
        return;

    T* end = data + count;
    T* last = end - 1;

    
    if (data != bound_begin && end != bound_end && count > 1) {
        // If the pivots bounding this sub-array are equal...
        if (*(data - 1) == *(last + 1))
            return; // ... we can skip processing this sub-array
    }

    if (count > 2) {
        T* middle = splitt(data, count);
        custom_quick_sort_improved_inner(data, middle - data, bound_begin, bound_end);
        middle++;
        custom_quick_sort_improved_inner(middle, end - middle, bound_begin, bound_end);
    } else {
        if (*data > *last)
            bytt(data, last);
    }
}



template <class T>
void custom_quick_sort_improved(T* data, size_t count) {
    custom_quick_sort_improved_inner(data, count, data, data + count);
}



template <class T, typename sort_function>
double test_sort_function(sort_function sort, T* data, size_t count) {
    // Store the checksum
    long double checksum = std::accumulate(data, data + count, 0);
    
    // Start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Perform sorting
    sort(data, count);

    // Stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the elapsed time in seconds
    double elapsed = (double) ((long double) (end_time - start_time).count() / 1'000'000'000);

    // Validate the sorting
    bool sorting_ok  = std::is_sorted(data, data + count);
    bool checksum_ok = std::accumulate(data, data + count, 0) == checksum;

    // Return error code if any validation failed
    if (!sorting_ok)
        elapsed = -1;
    else if (!checksum_ok)
        elapsed = -2;

    return elapsed;
}



template <class T, typename sort_function>
void print_measurements(sort_function sort, T* data, size_t count, const size_t first_num_elements, const size_t interval) {
    std::vector<T> arr(count);

    for (size_t i = first_num_elements; i <= count; i += interval) {
        memcpy(arr.data(), data, count * sizeof(T));
        double measurement = test_sort_function(sort, arr.data(), i);
        std::cout << std::to_string(i) + " : " + std::to_string(measurement) + "\n";
    }
}



template <class T, typename sort_function>
void print_complexity_measurements(sort_function sort, T* data, size_t count, const size_t first_num_elements) {
    std::vector<T> arr(count);

    for (size_t i = first_num_elements; i <= count; i *= 10) {
        memcpy(arr.data(), data, count * sizeof(T));
        double measurement = test_sort_function(sort, arr.data(), i);
        std::cout << std::to_string(i) + " : " + std::to_string(measurement) + "\n";
    }
}



template <class T, typename sort_function>
void demo(sort_function sort, std::string algorithm_name) {
    // The container for our test data, 
    std::vector<T> arr(10'000'000);

    // First, test the sorting algorithm with random data
    fill_with_test_data(arr.data(), arr.size());
    double elapsed = test_sort_function(sort, arr.data(), arr.size());
    std::cout << "### " + algorithm_name + " on random values ###\n" + std::to_string(elapsed) + "s\n\n";

    // Second, test the sorting algorithm with many duplicates
    fill_with_duplicate_test_data(arr.data(), arr.size());
    elapsed = test_sort_function(sort, arr.data(), arr.size());
    std::cout << "### " + algorithm_name + " on random values with many duplicates ###\n" + std::to_string(elapsed) + "s\n\n";

    // Third, test the sorting algorithm on a sorted data set
    fill_with_test_data(arr.data(), arr.size());
    std::sort(arr.begin(), arr.end());
    elapsed = test_sort_function(sort, arr.data(), arr.size());
    std::cout << "### " + algorithm_name + " on sorted data ###\n" + std::to_string(elapsed) + "s\n\n";

    // Fourth, test the sorting algorithm on a sorted data set (with many duplicates)
    fill_with_duplicate_test_data(arr.data(), arr.size());
    std::sort(arr.begin(), arr.end());
    elapsed = test_sort_function(sort, arr.data(), arr.size());
    std::cout << "### " + algorithm_name + " on sorted data with many duplicates ###\n" + std::to_string(elapsed) + "s\n\n";

    // Lastly, print complexity output
    std::cout << "### Complexity output of " + algorithm_name + " ###\n";
    print_complexity_measurements(sort, arr.data(), arr.size(), 10);

    std::cout << "\n\n\n";
}



int main() {
    demo<int>(custom_quick_sort<int>, "Quick-sort");
    demo<int>(custom_quick_sort_improved<int>, "Improved quick-sort");

    std::vector<int> arr(10'000'000);
    fill_with_test_data(arr.data(), (int) arr.size());

    std::cout << "### Quick-sort measurements [3 million, 10 million] ###\n";
    print_measurements(custom_quick_sort<int>, arr.data(), arr.size(), 3'000'000, 250'000);

    std::cout << "### Improved quick-sort measurements [3 million, 10 million] ###\n";
    print_measurements(custom_quick_sort_improved<int>, arr.data(), arr.size(), 3'000'000, 250'000);

    return 0;
}



// Unused stuff

/*
template <class T, typename sort_function>
double measure_sort_function(sort_function sort, T* data, size_t count) {
    // The least amount of elements to be sorted during time measurement
    // No matter the number of elements in the input array (defined by the
    // the parameter iterators), the sorting function will be called until this
    // amount of elements has been sorted.
    static const int ELEMENT_COUNT_THRESHOLD = 1'000;

    // The number of times to perform the sorting to reach the threshold.
    const long repeats = (int) std::ceil((long double) ELEMENT_COUNT_THRESHOLD / count);

    // The buffer we are sorting
    std::vector<T> buffer(repeats * count);

    // Store the checksum of that data
    const long double checksum = std::accumulate(data, data + count, 0);

    // Copy that data to the rest of the buffer
    T* it = buffer.data();
    for (long i = 0; i < repeats; i++, it+=count)
        memcpy(it, data, count * sizeof(T));

    // Start time measurement
    auto start_time = std::chrono::high_resolution_clock::now();

    // Perform sorting
    it = buffer.data();
    for (int i = 0; i < repeats; i++, it+=count)
        sort(it, count);

    // Stop time measurement
    auto end_time = std::chrono::high_resolution_clock::now();

    // Calculate the average time for each sort
    double time_per_sort = (double) ((long double) (end_time - start_time).count() / 1'000'000'000) / repeats;
    
    // Validate each sorted segment
    bool sorted_ok = true;
    bool checksum_ok = true;
    T* it1 = buffer.data();
    T* it2 = it1 + count;
    for (int i = 0; i < repeats; i++, it1+=count, it2+=count) {
        sorted_ok   &= std::is_sorted(it1, it2);
        checksum_ok &= std::accumulate(it1, it2, 0) == checksum;
    }

    // Return averga time per sort, or -1 if the sorting is incorrect, and -2 if the checksum check failed
    return time_per_sort * checksum_ok * sorted_ok + (-1) * !sorted_ok + (-2) * !checksum_ok;
}

template <class T>
void fill_with_decending(T* data, const T count) {
    for (T i = count - 1; i >= 0; i--, data++)
        *data = i;
}


*/