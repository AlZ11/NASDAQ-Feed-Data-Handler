#include "utils.h"
#include <iostream>

// Update the order map and book levels after an execution.
void executeOrder(std::unordered_map<uint64_t, Order>& orderMap,
                  std::map<uint32_t, uint64_t, std::greater<uint32_t>>& bids,
                  std::map<uint32_t, uint64_t>& asks,
                  uint64_t refNum,
                  uint32_t sharesExecuted) {
    auto it = orderMap.find(refNum);
    if (it == orderMap.end()) {
        // std::cerr << "Order reference number not found" << std::endl;
        return;
    }

    Order& order = it->second;

    auto reduceLevel = [&](auto& book) {
        auto level = book.find(order.price);
        if (level == book.end()) {
            return;
        }
        if (level->second <= sharesExecuted) {
            book.erase(level);
        } else {
            level->second -= sharesExecuted;
        }
    };

    if (order.side == 'B') {
        reduceLevel(bids);
    } else {
        reduceLevel(asks);
    }

    if (order.shares <= sharesExecuted) {
        orderMap.erase(it);
    } else {
        order.shares -= sharesExecuted;
    }
}

// Check if message length is valid
void checkMsg(int messageLength, size_t expectedSize) {
    if (messageLength - 1 < static_cast<int>(expectedSize)) {
        std::cerr << "Error: Message too short" << std::endl;
    }
}

// BlockTimer implementation
BlockTimer::BlockTimer(const std::string& processName) : name(processName) {
    start_time = std::chrono::high_resolution_clock::now();
}

void BlockTimer::stop() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "[" << name << "] took: " 
              << duration.count() << " microseconds (" 
              << duration.count() / 1000.0 << " ms)" << std::endl;
}

void BlockTimer::reset() {
    start_time = std::chrono::high_resolution_clock::now();
}
