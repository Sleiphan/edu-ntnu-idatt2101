#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>



std::pair<int, int> optimum_stock_plan_nSq(int* stock, int count) {
    if (count < 2)
        return std::pair<int, int>(-1, -1);

    int buy_idx  = -1;
    int sell_idx = -1;
    int current_profit = 0;

    for (int buy = 0; buy < count; buy++) {
        int temp_profit = 0;

        for (int sell = buy; sell < count; sell++) {
            temp_profit += stock[sell];

            if (current_profit < temp_profit) {
                current_profit = temp_profit;
                buy_idx = buy;
                sell_idx = sell;
            }
        }
    }

    std::pair<int, int> result(buy_idx, sell_idx);

    return result;
}



std::vector<long long> time_measurements() {
    std::vector<long long> measurements;

    int* data = new int[100000]; // Allocate a million random samples
    for (int i = 0; i < 100000; i++)
        data[i] = rand() % 19 - 9;

    

    std::chrono::time_point start_time = std::chrono::high_resolution_clock::now();
    auto [buy_1, sell_1] = optimum_stock_plan_nSq(data, 1000);
    std::chrono::time_point stop_time  = std::chrono::high_resolution_clock::now();
    measurements.push_back((stop_time - start_time).count());

    
    start_time = std::chrono::high_resolution_clock::now();
    auto [buy_2, sell_2] = optimum_stock_plan_nSq(data, 10000);
    stop_time  = std::chrono::high_resolution_clock::now();
    measurements.push_back((stop_time - start_time).count());

    
    start_time = std::chrono::high_resolution_clock::now();
    auto [buy_3, sell_3] = optimum_stock_plan_nSq(data, 100000);
    stop_time  = std::chrono::high_resolution_clock::now();
    measurements.push_back((stop_time - start_time).count());



    return measurements;
}



int main() {
    int stock_0[] = { -1, +3, -9, +2, +2, -1, +2, -1, -5, -1 };
    int stock_1[] = { -2, +5, -1, +3, -2, +9, -3, +1, +1, -9 };
    int stock_2[] = { -2, -5, -1, -3, -1, -2, -9, -1, -1, -1 };
    int stock_3[] = { -2, -5, -1, -3, -1, -2, -9, -1, -1, +1 };
    int stock_4[] = { +2, -5, -1, -3, -1, -2, -9, -1, -1, -1 };
    int stock_5[] = { -2, +5, -7, +2, -1, -2, -9, -1, -1, -1 };
    
    auto [buy_idx_0, sell_idx_0] = optimum_stock_plan_nSq(stock_0, 10);
    auto [buy_idx_1, sell_idx_1] = optimum_stock_plan_nSq(stock_1, 10);
    auto [buy_idx_2, sell_idx_2] = optimum_stock_plan_nSq(stock_2, 10);
    auto [buy_idx_3, sell_idx_3] = optimum_stock_plan_nSq(stock_3, 10);
    auto [buy_idx_4, sell_idx_4] = optimum_stock_plan_nSq(stock_4, 10);
    auto [buy_idx_5, sell_idx_5] = optimum_stock_plan_nSq(stock_5, 10);

    bool success =
        buy_idx_0 ==  3 && sell_idx_0 ==  6 &&
        buy_idx_1 ==  1 && sell_idx_1 ==  5 &&
        buy_idx_2 == -1 && sell_idx_2 == -1 &&
        buy_idx_3 ==  9 && sell_idx_3 ==  9 &&
        buy_idx_4 ==  0 && sell_idx_4 ==  0 &&
        buy_idx_5 ==  1 && sell_idx_5 ==  1;


    std::cout << "Algorithm test: " << (success ? "passed" : "FAILED" ) << std::endl;

    if (!success)
        return 1;

    std::cout << "Now running time measurements. Should take about 7 seconds..." << std::endl;
    
    std::vector<long long> measurements = time_measurements();

    for (auto it = measurements.begin(); it != measurements.end(); it++)
        std::cout << std::to_string((long double)(*it) / 1000000000) << "s" << std::endl;

    
    return 0;
}