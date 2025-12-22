#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "ITCHv50.h"

// Forward declarations for function declarations
void checkMsg(int messageLength, size_t expectedSize);

void updateBook(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid);

void reduceBook(std::vector<PriceLevel>& book, uint32_t price, uint32_t shares, bool isBid);

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
