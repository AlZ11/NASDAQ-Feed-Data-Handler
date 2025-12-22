#include "utils.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include "ITCHv50.h"


// Update the order map and book levels after an execution.
void reduceBook(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid) {
    
    auto cmp = [isBid](const PriceLevel& elem, uint32_t val) {
        return isBid ? (elem.price > val) : (elem.price < val);
    };

    auto it = std::lower_bound(book.begin(), book.end(), price, cmp);

    if (it != book.end() && it->price == price) {
        if (it->shares <= shares) {
            book.erase(it); // Remove level if liquidity is gone
        } else {
            it->shares -= shares;
        }
    }
}

// Check if message length is valid
void checkMsg(int messageLength, size_t expectedSize) {
    if (messageLength - 1 < static_cast<int>(expectedSize)) {
        std::cerr << "Error: Message too short" << std::endl;
    }
}

void updateBook(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid) {
    // 1. Binary Search (O(log N)) - Finds where the price is or should be
    auto it = std::lower_bound(book.begin(), book.end(), price, 
        [isBid](const PriceLevel& elem, uint32_t val) {
            return isBid ? (elem.price > val) : (elem.price < val);
        });

    // 2. Check if price exists
    if (it != book.end() && it->price == price) {
        it->shares += shares;
    } else {
        book.insert(it, {price, shares});
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
