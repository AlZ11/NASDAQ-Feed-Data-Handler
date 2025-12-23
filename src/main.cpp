#include <iostream>
#include <cstdint>
#include <chrono>

#include "../include/ITCHv50.h"
#include "../include/OrderBook.h"
#include "../include/MessageHandler.h"
#include "../include/MMapReader.h"

// INITIALISE orderBook WITH false FOR MAX THROUGHPUT

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <TICKER>" << std::endl;
        return 1;
    }
    
    std::string targetSymbol = argv[1];
    std::cout << "Looking for [" << targetSymbol << "]" << std::endl;
    targetSymbol.resize(8, ' ');
    const char* target = targetSymbol.c_str();

    // Open and memory map the file
    MMapReader reader("../data/itch50_data.bin");
    if (!reader.isOpen()) {
        return 1;
    }

    // Initialize order book (set to false for max throughput)
    OrderBook orderBook(true);
    orderBook.reserve(10000000);
    
    // Initialize msg handler
    MessageHandler handler(orderBook, target);
    
    size_t messageCount = 0;
    char* cursor = reader.begin();
    char* end = reader.end();

    auto start = std::chrono::high_resolution_clock::now();

    // Process messages
    while (cursor < end) {
        // Enough bytes for msg length?
        if (end - cursor < 2) break;
        
        // Endianess swap for msg length
        uint16_t messageLength = __builtin_bswap16(*reinterpret_cast<uint16_t*>(cursor));

        // Keep reading rest of the msg
        char* messageStart = cursor + 2;
        // Safety check
        if (messageStart + messageLength > end) break;
        
        char messageType = *messageStart;
        messageCount++;
        
        // Process msg
        handler.processMessage(messageStart, messageType);

        // Next msg
        cursor += 2 + messageLength;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;

    // Display results
    std::cout << "\n================================================" << std::endl;
    std::cout << "Processing Complete!" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Time:            " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Messages:        " << static_cast<double>(messageCount) << std::endl;
    std::cout << "Throughput:      " << static_cast<long>(messageCount / elapsed.count()) << " msgs/sec" << std::endl;
    std::cout << "Active Orders:   " << orderBook.getOrderCount() << std::endl;
    std::cout << "================================================\n" << std::endl;
    
    // Disable book snapshot for maximum throughput
    // To enable, set OrderBook orderBook(true);
    orderBook.printSnapshot(targetSymbol, 10);

    // Cleanup happens automatically via RAII (MMapReader destructor)
    return 0;
}
