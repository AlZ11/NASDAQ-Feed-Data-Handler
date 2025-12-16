#include <cstdint>

#ifndef MESSAGES
#define MESSAGES

// Remove padding (memory alignment), We want the memory layout to match the wire protocol exactly.
#pragma pack(push, 1)

struct SystemEventMessage {
    char messageType;        // Offset 0, Length 1
    uint16_t stockLocate;    // Offset 1, Length 2
    uint16_t trackingNumber; // Offset 3, Length 2
    char timeStamp[6];       // Offset 5, Length 6
    char eventCode;          // Offset 11, Length 1
};

struct AddOrderMessage {
    char messageType;              // Offset 0, Length 1
    uint16_t stockLocate;          // Offset 1, Length 2
    uint16_t trackingNumber;       // Offset 3, Length 2
    char timeStamp[6];             // Offset 5, Length 6
    uint64_t orderReferenceNumber; // Offset 11, Length 8
    char indicator;                // Offset 19, Length 1
    uint32_t shares;               // Offset 20, Length 4
    char stock[8];                 // Offset 24, Length 8
    uint32_t price;                // Offset 32, Length 4
};

struct OrderExecutedMessage {
    char messageType;              // Offset 0, Length 1
    uint16_t stockLocate;          // Offset 1, Length 2
    uint16_t trackingNumber;       // Offset 3, Length 2
    char timeStamp[6];             // Offset 5, Length 6
    uint64_t orderReferenceNumber; // Offset 11, Length 8
    uint32_t executedShares;       // Offset 19, Length 4
    uint64_t matchNumber;          // Offset 23, Length 8
};

struct OrderExecutedWithPriceMessage {
    char messageType;              // Offset 0, Length 1
    uint16_t stockLocate;          // Offset 1, Length 2
    uint16_t trackingNumber;       // Offset 3, Length 2
    char timeStamp[6];             // Offset 5, Length 6
    uint64_t orderReferenceNumber; // Offset 11, Length 8
    uint32_t executedShares;       // Offset 19, Length 4
    uint64_t matchNumber;          // Offset 23, Length 8
    char printable;                // Offset 31, Length 1
    uint32_t executionPrice;       // Offeset 32, Length 4
};

struct OrderCancelMessage {
    char messageType;              // Offset 0, Length 1
    uint16_t stockLocate;          // Offset 1, Length 2
    uint16_t trackingNumber;       // Offset 3, Length 2
    char timeStamp[6];             // Offset 5, Length 6
    uint64_t orderReferenceNumber; // Offset 11, Length 8
    uint32_t cancelledShares;      // Offset 19, Length 4
};

struct OrderDeleteMessage {
    char messageType;              // Offset 0, Length 1
    uint16_t stockLocate;          // Offset 1, Length 2
    uint16_t trackingNumber;       // Offset 3, Length 2
    char timeStamp[6];             // Offset 5, Length 6
    uint64_t orderReferenceNumber; // Offset 11, Length 8
};

struct Order {
    uint32_t price;
    uint32_t shares;
    char side;
};

#pragma pack(pop)

#endif
