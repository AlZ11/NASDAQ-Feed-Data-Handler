#include "utils.h"

// Update the order map and book levels after an execution.
void executeOrder(std::unordered_map<uint64_t, Order>& orderMap,
                  std::map<uint32_t, uint64_t, std::greater<uint32_t>>& bids,
                  std::map<uint32_t, uint64_t>& asks,
                  uint64_t refNum,
                  uint32_t sharesExecuted) {
    auto it = orderMap.find(refNum);
    if (it == orderMap.end()) {
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
