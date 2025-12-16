#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <stdlib.h>
#include <map>
#include "ITCHv50.h"
#include "utils.h"


using namespace std;

int main() {
    string filename = "../data/itch50_data.bin";
    ifstream file(filename, ios::binary);

    file.seekg(0, ios::end);
    streamsize size = file.tellg();
    file.seekg(0, ios::beg);

    // Allocate memory
    std::vector<char> buffer(size);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return 1;
    }

    map<uint32_t, uint64_t> asks; 
    map<uint32_t, uint64_t, greater<uint32_t>> bids;
    unordered_map<uint64_t, Order> orderMap;
    int counter = 0;

    uint16_t messageLength;
    while (file.read(reinterpret_cast<char*>(&messageLength), sizeof(messageLength))) {
        messageLength = __builtin_bswap16(messageLength); 
        
        char messageType;
        if (!file.read(reinterpret_cast<char*>(&messageType), sizeof(messageType))) {
            cerr << "Error: Could not read message type." << endl;
            return 1;
        }

        switch (messageType) {
            case 'S': { // System event message
                SystemEventMessage msg;
                msg.messageType = messageType;
                // Read messageLength - 1 bytes from memory location + 1 and because we already read messageType (1 byte)
                if (!file.read(reinterpret_cast<char*>(&msg) + 1, messageLength - 1)) {
                    cerr << "Partial read on System Event Message." << endl;
                    return 1;
                }

                // Endianness swap
                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack  = __builtin_bswap16(msg.trackingNumber);

                cout << "[S] System Event | Code: " << msg.eventCode 
                     << " | Locate: " << fixedLocate << endl;
                break;
            }
            case 'A': { // Add order message
                AddOrderMessage msg;
                if (!readMessageBody(file, messageLength, messageType, msg, "Add Order Message")) {
                    return 1;
                }

                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg.trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg.orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg.shares);
                uint32_t fixedPrice  = __builtin_bswap32(msg.price);

                // Convert Price (Integer) to double (Divide by 10000)
                double finalPrice = fixedPrice / 10000.0;
                
                // Clean print of Stock Symbol (8 chars, not null-terminated)
                string symbol(msg.stock, 8); 
                if (symbol == "AAPL    ") {
                    msg.indicator == 'B' ? bids[fixedPrice] += fixedShares : asks[fixedPrice] += fixedShares;
                    Order o = {fixedPrice, fixedShares, msg.indicator};
                    orderMap[fixedRef] = o;
                }
                counter++;
                if (!(counter % 1000)) {
                    cout << "--- BOOK SNAPSHOT (AAPL) ---" << endl;
                    cout << "ASKS:" << endl;
                    int num = 0;
                    for (const auto& pair : asks) {
                        if (num == 5) {
                            break;
                        }
                        cout << "   $" << pair.first / 10000.0 << " : " << pair.second << " shares" << endl;
                        num++;
                    }
                    cout << "----------------------------" << endl;
                    cout << "BIDS:" << endl;
                    num = 0;
                    for (const auto& pair : bids) {
                        if (num == 5) {
                            break;
                        }
                        cout << "   $" << pair.first / 10000.0 << " : " << pair.second << " shares" << endl;
                        num++;
                    }
                }
                // cout << "[A] Add Order    | Stock: " << symbol << " | Side: " << msg.indicator << " | Shares: " << fixedShares << " | Price: " << fixedPrice << " ($" << fixedPrice << ")" << endl;
                break;
            }
            case 'X' : { // Order cancelled message
                OrderCancelMessage msg;
                if (!readMessageBody(file, messageLength, messageType, msg, "Order Cancel Message")) {
                    return 1;
                }
                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg.trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg.orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg.cancelledShares);

                if (orderMap.find(fixedRef) != orderMap.end()) {
                    if (orderMap[fixedRef].side == 'B') {
                        bids[orderMap[fixedRef].price] -= fixedShares;
                        if (bids[orderMap[fixedRef].price] == 0) bids.erase(orderMap[fixedRef].price);
                    } else {
                        asks[orderMap[fixedRef].price] -= fixedShares;
                        if (asks[orderMap[fixedRef].price] == 0) asks.erase(orderMap[fixedRef].price);
                    }
                    
                    orderMap[fixedRef].shares -= fixedShares;
                    if (orderMap[fixedRef].shares <= 0) orderMap.erase(fixedRef);
                } else {
                    cerr << "Order reference number not found" << endl;
                }
                break;
            }
            case 'D' : { // Order delete message
                OrderDeleteMessage msg;
                if (!readMessageBody(file, messageLength, messageType, msg, "Order Delete Message")) {
                    return 1;
                }
                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg.trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg.orderReferenceNumber);


            if (orderMap.find(fixedRef) != orderMap.end()) {
                uint32_t price = orderMap[fixedRef].price;
                uint32_t shares = orderMap[fixedRef].shares;

                if (orderMap[fixedRef].side == 'B') {
                    bids[price] -= shares; // Subtract only this order's volume
                    if (bids[price] == 0) bids.erase(price); // Clean up if empty
                } else {
                    asks[price] -= shares;
                    if (asks[price] == 0) asks.erase(price);
                }
                
                // Remove the order from the map after deletion
                orderMap.erase(fixedRef);
            } else {
                cerr << "Order reference number not found" << endl;
            }
                break;

            }

            case 'E' : { // Order executed message
                OrderExecutedMessage msg;
                if (!readMessageBody(file, messageLength, messageType, msg, "Order Executed Message")) {
                    return 1;
                }
                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg.trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg.orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg.executedShares);
                uint32_t fixedMatch  = __builtin_bswap32(msg.matchNumber);

                executeOrder(orderMap, bids, asks, fixedRef, fixedShares);
                break;
            }

            case 'C' : { // Order executed with price message
                OrderExecutedWithPriceMessage msg;
                if (!readMessageBody(file, messageLength, messageType, msg, "Order Executed With Price Message")) {
                    return 1;
                }
                uint16_t fixedLocate = __builtin_bswap16(msg.stockLocate);
                uint16_t fixedTrack = __builtin_bswap16(msg.trackingNumber);
                uint64_t fixedRef    = __builtin_bswap64(msg.orderReferenceNumber);
                uint32_t fixedShares = __builtin_bswap32(msg.executedShares);
                uint64_t fixedMatch  = __builtin_bswap64(msg.matchNumber);
                uint32_t fixedPrice  = __builtin_bswap32(msg.executionPrice);

                executeOrder(orderMap, bids, asks, fixedRef, fixedShares);
                break;
            }
            default:
                // If the type is unknown, the remaining bytes of this message must be skipped
                // So file pointer is positioned correctly for the NEXT message length.
                file.seekg(messageLength - 1, ios::cur);
                break;
        }
    }
    file.close();
    return 0;
}
