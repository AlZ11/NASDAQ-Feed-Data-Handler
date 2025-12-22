#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <stdlib.h>
#include <map>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <cstdio> 

#include "ITCHv50.h"
#include "utils.h"


using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <TICKER>" << endl;
        return 1;
    }
    string targetSymbol = argv[1];
    cout << "Listening for symbol: [" << targetSymbol << "]" << std::endl;
    targetSymbol.resize(8, ' ');
    const char* target = targetSymbol.c_str();

    const char* filename = "../data/itch50_data.bin";

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        cerr << "Error: Could not open file" << endl;
    }
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        cerr << "Error: Could not get file size" << endl;
        close(fd);
        return 1;
    }

    size_t fileSize = sb.st_size;

    char* mappedData = static_cast<char*>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));

    if (mappedData == MAP_FAILED) {
        cerr << "Error: mmap failed" << endl;
        close(fd);
        return 1;
    }

    vector<PriceLevel> asks; 
    vector<PriceLevel> bids;
    unordered_map<uint64_t, Order> orderMap;
    int counter = 0;
    char* cursor = mappedData;
    char* end = mappedData + fileSize;

    uint16_t messageLength;

    // BlockTimer timer("Many System Calls (1 Byte Read)");
    orderMap.reserve(10000000);
    auto start = std::chrono::high_resolution_clock::now();

    while (cursor < end) {
        if (end - cursor < 2) break; 
        
        uint16_t messageLength = *reinterpret_cast<uint16_t*>(cursor);
        messageLength = __builtin_bswap16(messageLength); 

        char* messageStart = cursor + 2; 

        if (messageStart + messageLength > end) {
            // cerr << "Error: Incomplete message at end of file" << endl;
            break;
        }
        
        char messageType = *messageStart;
        counter++;
        switch (messageType) {
            case 'S': { // System event message
                SystemEventMessage* msg = reinterpret_cast<SystemEventMessage*>(messageStart);

                // Endianness swap
                uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                uint16_t fixedTrack  = __builtin_bswap16(msg->trackingNumber);
                break;
            }
            case 'A': { // Add order message
                AddOrderMessage* msg = reinterpret_cast<AddOrderMessage*>(messageStart);
                
                if (strncmp(msg->stock, target, 8) == 0) {
                    uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                    uint16_t fixedTrack = __builtin_bswap16(msg->trackingNumber);
                    uint64_t fixedRef    = __builtin_bswap64(msg->orderReferenceNumber);
                    uint32_t fixedShares = __builtin_bswap32(msg->shares);
                    uint32_t fixedPrice  = __builtin_bswap32(msg->price);

                    // Convert Price (Integer) to double (Divide by 10000)
                    double finalPrice = fixedPrice / 10000.0;

                    // Clean print of Stock Symbol (8 chars, not null-terminated)
                    string symbol(msg->stock, 8);

                    Order o = {fixedPrice, fixedShares, msg->indicator};
                    orderMap[fixedRef] = o;
                    
                }
                break;
            }
            case 'X' : { // Order cancelled message
                OrderCancelMessage* msg = reinterpret_cast<OrderCancelMessage*>(messageStart);

                uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg->trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg->orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg->cancelledShares);
                
                // Direct map lookup (No endian swap needed for checking existence)
                auto it = orderMap.find(fixedRef);
                if (it != orderMap.end()) {
                    uint32_t shares = __builtin_bswap32(msg->cancelledShares);
                    
                    // Reduce Vector Book
                    reduceBook(it->second.side == 'B' ? bids : asks, it->second.price, shares, it->second.side == 'B');

                    it->second.shares -= shares;
                    if (it->second.shares <= 0) orderMap.erase(it);
                } else {
                    // cerr << "Order reference number not found" << endl;
                }
                break;
            }
            case 'D' : { // Order delete message
                OrderDeleteMessage* msg = reinterpret_cast<OrderDeleteMessage*>(messageStart);


                uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg->trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg->orderReferenceNumber);

                auto it = orderMap.find(fixedRef);
                if (it != orderMap.end()) {
                    // Reduce Vector Book by TOTAL remaining shares
                    reduceBook(it->second.side == 'B' ? bids : asks, it->second.price, it->second.shares, it->second.side == 'B');
                    orderMap.erase(it);
                } else {
                    // cerr << "Order reference number not found" << endl;
                }
                break;
            }

            case 'E' : { // Order executed message
                OrderExecutedMessage* msg = reinterpret_cast<OrderExecutedMessage*>(messageStart);
                uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg->trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg->orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg->executedShares);
                uint32_t fixedMatch  = __builtin_bswap32(msg->matchNumber);
                
                auto it = orderMap.find(fixedRef);
                if (it != orderMap.end()) {
                    uint32_t shares = __builtin_bswap32(msg->executedShares);
                    reduceBook(it->second.side == 'B' ? bids : asks, it->second.price, shares, it->second.side == 'B');

                    it->second.shares -= shares;
                    if (it->second.shares <= 0) orderMap.erase(it);
                }
                break;
            }

            case 'C' : { // Order executed with price message
                OrderExecutedWithPriceMessage* msg = reinterpret_cast<OrderExecutedWithPriceMessage*>(messageStart);

                uint16_t fixedLocate = __builtin_bswap16(msg->stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg->trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg->orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg->executedShares);
                uint64_t fixedMatch  = __builtin_bswap64(msg->matchNumber);
                uint32_t fixedPrice  = __builtin_bswap32(msg->executionPrice);

                auto it = orderMap.find(fixedRef);
                if (it != orderMap.end()) {
                    uint32_t shares = __builtin_bswap32(msg->executedShares);
                    // IMPORTANT: We reduce the book at the ORIGINAL price (it->second.price),
                    // NOT the execution price in the 'C' message.
                    reduceBook(it->second.side == 'B' ? bids : asks, it->second.price, shares, it->second.side == 'B');

                    it->second.shares -= shares;
                    if (it->second.shares <= 0) orderMap.erase(it);
                }
                break;
            }
            default:
                break;
        }
        cursor += 2 + messageLength;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = finish - start;
    double seconds = diff.count();
    
    // 'counter' is your total messages processed
    // Note: You need to increment 'counter' for EVERY message, not just 'A'
    double msgsPerSec = counter / seconds;

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << "Time: " << seconds << " seconds" << std::endl;
    std::cout << "Total Messages: " << counter << std::endl;
    std::cout << "Throughput: " << (long)msgsPerSec << " msgs/sec" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    // timer.stop();
    if (munmap(mappedData, fileSize) == -1) {
        // cerr << "Error: munmap failed" << endl;
    }
    close(fd);
    return 0;
}
