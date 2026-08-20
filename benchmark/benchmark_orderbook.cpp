#include "../src/types.h"
#include "../src/order.h"
#include "../src/orderbook.h"
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include <vector>
#include <iostream>
using namespace std;

static int timeStamp = 1;

void populateBook(OrderBook& book, mt19937& generator) {
    normal_distribution<double> W;
    uniform_int_distribution<int> quantityGenerator;
    double sigma = 0.005; // arbitrary value - chosen for a tighter price distribution to stress test
    // there is a 'mu' value too, but it adds unnecessary variance to price generation, so I have ignored it for now
    
    // generating buy and sell prices at the below intervals to avoid matching while populating order book
    int buyPrice = 14750; // ranges from 14500 <= x < 15000 - equal to $145.00 <= x < $150.00
    int sellPrice = 15200; // ranges from 15000 <= x < 15500 - equal to $150.00 <= x < $155.00

    int orderQuantity = 15;

    while (book.getOrderCount() < 100000) {
        Side side = book.getOrderCount() % 2 == 0 ? BUY : SELL;
        int price = 15000;

        double exponent = (-(sigma * sigma) / 2) + sigma * W(generator);
        if (side == BUY) {
            price = buyPrice * exp(exponent);
            price = max(price, 14500);
            price = min(price, 14999);
        }
        else {
            price = sellPrice * exp(exponent);
            price = max(price, 15000);
            price = min(price, 15499);
        }
        int quantity = quantityGenerator(generator);

        book.addOrder(side, price, quantity, timeStamp++);
    }
}

void reportLatencies(std::vector<int>& latencies) {
    std::sort(latencies.begin(), latencies.end());

    size_t n = latencies.size();
    auto percentile = [&](double p) {
        size_t idx = static_cast<size_t>(p * (n - 1));
        return latencies[idx];
    };

    long long sum = std::accumulate(latencies.begin(), latencies.end(), 0LL);
    double mean = static_cast<double>(sum) / n;

    cout << "n       = " << n << endl;
    cout << "min     = " << latencies.front() << endl;
    cout << "max     = " << latencies.back() << endl;
    cout << "mean    = " << mean << endl;
    cout << "p50     = " << percentile(0.50) << endl;
    cout << "p90     = " << percentile(0.90) << endl;
    cout << "p99     = " << percentile(0.99) << endl;
    cout << "p99.9   = " << percentile(0.999) << endl;
    cout << "p99.99  = " << percentile(0.9999) << endl;
}

int getDistributionType(int i, mt19937& generator) {
    uniform_int_distribution<int> uniformPrice(14500, 15500);
    exponential_distribution<double> expPrice(1.0 / 300.0);
    normal_distribution<double> normalPrice(15000, 150.0);
    lognormal_distribution<double> logNormalPrice(std::log(15000), 0.01);
    student_t_distribution<double> studentTPrice(3.0);
    
    int p = 15000;
    switch (i) {
        case 0: p = uniformPrice(generator); break;
        case 1: p = 14500 + min(expPrice(generator), 1000.00); break;
        case 2: p = normalPrice(generator); break;
        case 3: p = logNormalPrice(generator); break;
        case 4: p = 15000 + studentTPrice(generator) * 50; break;
    }
    
    p = max(p, 14500);
    p = min(p, 15500);
    
    return p;
}

void measurePerOrderLatency() {
    mt19937 generator(30);
    vector<string> distributions = {"Uniform" , "Exponential", "Normal", "Log Normal", "Student T"};
    
    for (int i = 0; i < distributions.size(); i++) { // loop for going through all 5 distributions
        OrderBook book;
        cout << "Populating book" << endl; 
        populateBook(book, generator);
        cout << "Populated book" << endl; 
        
        vector<int> latencies;
        vector<U64> activeOrders;
        activeOrders.reserve(1000000);
        latencies.reserve(1000000);
        cout << "Beginning benchmark #" << i + 1 << " for " << distributions[i] << "." << endl;
        for (int j = 0; j < 1000000; j++) {            
            // adding order
            int price = getDistributionType(i, generator);
            Side side = j % 2 == 0 ? BUY : SELL;
            int quantity = 15 + (j % 10);
            int timestamp = 100000 + j;
            
            const auto t1 = chrono::steady_clock::now();
            auto newOrderInfo = book.addOrder(side, price, quantity, timestamp);
            const auto t2 = chrono::steady_clock::now();
            
            if (newOrderInfo.first) activeOrders.push_back(newOrderInfo.second);
            
            // calculate time taken
            const chrono::duration<int, nano> elapsed = t2 - t1;
            latencies.push_back(elapsed.count() > 0 ? elapsed.count() : 0);
        }

        cout << distributions[i] << " statistics:" << endl;
        reportLatencies(latencies);
    }
}

int main() {
    cout << "Measuring Per Order Latencies: " << endl;
    measurePerOrderLatency();
}