#include "../include/MessageHandler.h"

MessageHandler::MessageHandler(OrderBook& book, const char* symbol) 
    : orderBook(book), targetSymbol(symbol) {
}

void MessageHandler::processMessage(char* messageStart, char messageType) {
    switch (messageType) {
        case 'S':
            handleSystemEvent(messageStart);
            break;
        case 'A':
            handleAddOrder(messageStart);
            break;
        case 'X':
            handleOrderCancel(messageStart);
            break;
        case 'D':
            handleOrderDelete(messageStart);
            break;
        case 'E':
            handleOrderExecuted(messageStart);
            break;
        case 'C':
            handleOrderExecutedWithPrice(messageStart);
            break;
        default:
            // Unknown message type - ignore
            break;
    }
}

void MessageHandler::handleSystemEvent(char* messageStart) {
    // System events don't affect the order book
    // Add implementation later if I need to for certain reasons
}

// A
void MessageHandler::handleAddOrder(char* messageStart) {
    AddOrderMessage* msg = reinterpret_cast<AddOrderMessage*>(messageStart);
    
    // Only process orders for our target symbol
    if (strncmp(msg->stock, targetSymbol, 8) == 0) {
        uint64_t fixedRef = __builtin_bswap64(msg->orderReferenceNumber);
        uint32_t fixedShares = __builtin_bswap32(msg->shares);
        uint32_t fixedPrice = __builtin_bswap32(msg->price);
        
        orderBook.addOrder(fixedRef, fixedPrice, fixedShares, msg->indicator);
    }
}

// X
void MessageHandler::handleOrderCancel(char* messageStart) {
    OrderCancelMessage* msg = reinterpret_cast<OrderCancelMessage*>(messageStart);
    
    uint64_t fixedRef = __builtin_bswap64(msg->orderReferenceNumber);
    uint32_t fixedShares = __builtin_bswap32(msg->cancelledShares);
    
    orderBook.cancelOrder(fixedRef, fixedShares);
}

// D
void MessageHandler::handleOrderDelete(char* messageStart) {
    OrderDeleteMessage* msg = reinterpret_cast<OrderDeleteMessage*>(messageStart);
    
    uint64_t fixedRef = __builtin_bswap64(msg->orderReferenceNumber);
    
    orderBook.deleteOrder(fixedRef);
}

// E
void MessageHandler::handleOrderExecuted(char* messageStart) {
    OrderExecutedMessage* msg = reinterpret_cast<OrderExecutedMessage*>(messageStart);
    
    uint64_t fixedRef = __builtin_bswap64(msg->orderReferenceNumber);
    uint32_t fixedShares = __builtin_bswap32(msg->executedShares);
    
    orderBook.executeOrder(fixedRef, fixedShares);
}

// C
void MessageHandler::handleOrderExecutedWithPrice(char* messageStart) {
    OrderExecutedWithPriceMessage* msg = reinterpret_cast<OrderExecutedWithPriceMessage*>(messageStart);
    
    uint64_t fixedRef = __builtin_bswap64(msg->orderReferenceNumber);
    uint32_t fixedShares = __builtin_bswap32(msg->executedShares);
    
    orderBook.executeOrder(fixedRef, fixedShares);
}
