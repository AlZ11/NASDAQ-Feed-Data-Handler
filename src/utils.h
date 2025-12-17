#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>

#include "ITCHv50.h"

// Forward declarations for function declarations
void executeOrder(
	std::unordered_map<uint64_t, Order>& orderMap, 
	std::map<uint32_t, uint64_t, std::greater<uint32_t>>& bids, 
	std::map<uint32_t, uint64_t>& asks,
	uint64_t refNum,
	uint32_t sharesExecuted);

void checkMsg(int messageLength, size_t expectedSize);

// Template function - must remain in header
template <typename T>
inline bool readMessageBody(
	std::ifstream& file,
	uint16_t messageLength,
    char messageType,
	T& msg,
	const char* label) 
	{
		msg.messageType = messageType;
		const auto payloadSize = sizeof(T) - 1;
		checkMsg(messageLength, payloadSize);

		if (!file.read(reinterpret_cast<char*>(&msg) + 1, payloadSize)) {
			std::cerr << "Partial read on " << label << '\n';
			return false;
		}

		const auto bytesToSkip = static_cast<std::streamoff>((messageLength - 1) - payloadSize);
		if (bytesToSkip > 0) {
			file.seekg(bytesToSkip, std::ios::cur);
		}
		return true;
}

struct BlockTimer {
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time;

    BlockTimer(const std::string& processName);
    void stop();
    void reset();
};
