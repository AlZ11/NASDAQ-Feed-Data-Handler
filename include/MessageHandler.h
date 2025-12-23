#pragma once

#include <cstdint>
#include <cstring>
#include "ITCHv50.h"
#include "OrderBook.h"

class MessageHandler {
    private:
        OrderBook& orderBook;
        const char* targetSymbol;  // 8 byte symbol with padding
        
        // Message type handlers
        void handleSystemEvent(char* messageStart);
        void handleAddOrder(char* messageStart);
        void handleOrderCancel(char* messageStart);
        void handleOrderDelete(char* messageStart);
        void handleOrderExecuted(char* messageStart);
        void handleOrderExecutedWithPrice(char* messageStart);
        
    public:
        MessageHandler(OrderBook& book, const char* symbol);
        
        // Process a single message (switch statements)
        void processMessage(char* messageStart, char messageType);
};
