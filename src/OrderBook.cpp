#include "../include/OrderBook.h"
#include <iostream>
#include <algorithm>

// Copying memory when map resizes is slow
// Allocate capacity at startup for efficiency
void OrderBook::reserve(size_t capacity) {
    orderMap.reserve(capacity);
}

void OrderBook::addOrder(uint64_t refNum, uint32_t price, uint32_t shares, char side) {
    // Create order in map
    Order order = {price, shares, side};
    orderMap[refNum] = order;
    
    // Only update book levels if enabled
    if (maintainBookLevels) {
        updateLevel(bids, price, shares, side == 'B');
    }
}

void OrderBook::cancelOrder(uint64_t refNum, uint32_t shares) {
    auto it = orderMap.find(refNum);
    if (it == orderMap.end()) {
        return;  // Order not found
    }
    
    // Reduce the book at this price level (if maintaining levels)
    if (maintainBookLevels) {
        bool isBid = (it->second.side == 'B');
        reduceLevel(isBid ? bids : asks, it->second.price, shares, isBid);
    }
    
    // Update or remove the order
    it->second.shares -= shares;
    if (it->second.shares <= 0) {
        orderMap.erase(it);
    }
}

void OrderBook::deleteOrder(uint64_t refNum) {
    auto it = orderMap.find(refNum);
    if (it == orderMap.end()) {
        return;  // Order not found
    }
    
    // Reduce the book by the full remaining shares (if maintaining levels)
    if (maintainBookLevels) {
        bool isBid = (it->second.side == 'B');
        reduceLevel(isBid ? bids : asks, it->second.price, it->second.shares, isBid);
    }
    
    // Remove the order completely
    orderMap.erase(it);
}

void OrderBook::executeOrder(uint64_t refNum, uint32_t shares) {
    auto it = orderMap.find(refNum);
    if (it == orderMap.end()) {
        return;  // Order not found
    }
    
    // Reduce the book at this price level (if maintaining levels)
    if (maintainBookLevels) {
        bool isBid = (it->second.side == 'B');
        reduceLevel(isBid ? bids : asks, it->second.price, shares, isBid);
    }
    
    // Update or remove the order
    it->second.shares -= shares;
    if (it->second.shares <= 0) {
        orderMap.erase(it);
    }
}

bool OrderBook::hasOrder(uint64_t refNum) const {
    return orderMap.find(refNum) != orderMap.end();
}

const Order* OrderBook::getOrder(uint64_t refNum) const {
    auto it = orderMap.find(refNum);
    return (it != orderMap.end()) ? &it->second : nullptr;
}

void OrderBook::printSnapshot(const std::string& symbol, int topLevels) const {
    std::cout << "--- BOOK SNAPSHOT (" << symbol << ") ---" << std::endl;
    
    std::cout << "ASKS:" << std::endl;
    int count = 0;
    for (const auto& level : asks) {
        if (count >= topLevels) break;
        std::cout << "   $" << (level.price / 10000.0) << " : " << level.shares << " shares" << std::endl;
        count++;
    }
    
    std::cout << "----------------------------" << std::endl;
    std::cout << "BIDS:" << std::endl;
    count = 0;
    for (const auto& level : bids) {
        if (count >= topLevels) break;
        std::cout << "   $" << (level.price / 10000.0) << " : " << level.shares << " shares" << std::endl;
        count++;
    }
    std::cout << "----------------------------" << std::endl;
}

// Update a price level (add shares or create new level)
void OrderBook::updateLevel(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid) {
    // Binary search to find where price is or should be
    auto it = std::lower_bound(book.begin(), book.end(), price, 
        [isBid](const PriceLevel& elem, uint32_t val) { // is_bid for sort descending, otherwise ascending
            return isBid ? (elem.price > val) : (elem.price < val);
        });

    // Check if price level exists
    if (it != book.end() && it->price == price) {
        it->shares += shares;  // Add to existing level
    } else {
        book.insert(it, {price, shares});  // Create new level
    }
}

// Reduce shares at a price level (or remove if depleted)
void OrderBook::reduceLevel(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid) {
    auto cmp = [isBid](const PriceLevel& elem, uint32_t val) {
        return isBid ? (elem.price > val) : (elem.price < val);
    };

    auto it = std::lower_bound(book.begin(), book.end(), price, cmp);

    if (it != book.end() && it->price == price) {
        if (it->shares <= shares) {
            book.erase(it);  // Remove level if liquidity is gone
        } else {
            it->shares -= shares;
        }
    }
}
