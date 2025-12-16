#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>

#include "ITCHv50.h"
#include <iostream>
using namespace std;

// Apply an execution to an order: reduce order shares, adjust book levels, drop empty entries.
inline void executeOrder(
	unordered_map<uint64_t, Order>& orderMap, 
	map<uint32_t, uint64_t, greater<uint32_t>>& bids, 
	map<uint32_t, uint64_t>& asks,
	uint64_t refNum,
	uint32_t sharesExecuted) 
	{
	auto it = orderMap.find(refNum);
	if (it == orderMap.end()) {
		cerr << "Order reference number not found" << endl;
		return;
	}

	Order& o = it->second;

		if (o.side == 'B') {
			bids[o.price] -= sharesExecuted;
			if (bids[o.price] <= 0) {
				bids.erase(o.price);
			}
		} else {
			asks[o.price] -= sharesExecuted;
			if (asks[o.price] <= 0) {
				asks.erase(o.price);
			}
		}

		o.shares -= sharesExecuted;
		if (o.shares <= 0) {
			orderMap.erase(it);
		}
}

inline void checkMsg(int messageLength, size_t expectedSize) {
	if (messageLength - 1 < expectedSize) {
		cerr << "Error: Message too short" << endl;

    }
}

template <typename T>
inline bool readMessageBody(
	ifstream& file,
	uint16_t messageLength,
    char messageType,
	T& msg,
	const char* label) 
	{
		msg.messageType = messageType;
		const auto payloadSize = sizeof(T) - 1;
		checkMsg(messageLength, payloadSize);

		if (!file.read(reinterpret_cast<char*>(&msg) + 1, payloadSize)) {
			cerr << "Partial read on " << label << '\n';
			return false;
		}

		const auto bytesToSkip = static_cast<streamoff>((messageLength - 1) - payloadSize);
		if (bytesToSkip > 0) {
			file.seekg(bytesToSkip, ios::cur);
		}
		return true;
}
