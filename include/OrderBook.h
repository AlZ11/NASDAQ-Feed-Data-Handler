#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include "ITCHv50.h"

class OrderBook {
    private:
        std::vector<PriceLevel> bids;  // Sorted descending (high to low)
        std::vector<PriceLevel> asks;  // Sorted ascending (low to high)
        std::unordered_map<uint64_t, Order> orderMap;
        bool maintainBookLevels;  // Performance flag
        
        // Internal helper methods
        void updateLevel(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid);
        void reduceLevel(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid);
        
    public:
        OrderBook(bool maintainLevels = false) : maintainBookLevels(maintainLevels) {}
        
        // Pre allocate order map capacity for performance
        void reserve(size_t capacity);
        
        // Order lifecycle operations
        void addOrder(uint64_t refNum, uint32_t price, uint32_t shares, char side);
        void cancelOrder(uint64_t refNum, uint32_t shares);
        void deleteOrder(uint64_t refNum);
        void executeOrder(uint64_t refNum, uint32_t shares);
        
        // Query operations
        bool hasOrder(uint64_t refNum) const;
        const Order* getOrder(uint64_t refNum) const;
        
        // Access to book levels (for analysis)
        const std::vector<PriceLevel>& getBids() const { return bids; }
        const std::vector<PriceLevel>& getAsks() const { return asks; }
        size_t getOrderCount() const { return orderMap.size(); }
        
        // Display
        void printSnapshot(const std::string& symbol, int topLevels = 5) const;
};
