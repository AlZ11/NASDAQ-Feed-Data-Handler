#pragma once

#include <string>
#include <cstddef>

class MMapReader {
    private:
        int fd;
        char* mappedData;
        size_t fileSize;
        bool isValid;
        
    public:
        // Constructor opens and maps the file
        explicit MMapReader(const std::string& filename);
        
        // Destructor ensures cleanup
        ~MMapReader();
        
        // Delete copy operations
        MMapReader(const MMapReader&) = delete;
        MMapReader& operator=(const MMapReader&) = delete;
        
        // Move operations
        MMapReader(MMapReader&& other) noexcept;
        MMapReader& operator=(MMapReader&& other) noexcept;
        
        // Status checking
        bool isOpen() const { return isValid; }
        
        // Data access
        char* data() const { return mappedData; }
        size_t size() const { return fileSize; }
        
        // Iterator style access
        char* begin() const { return mappedData; }
        char* end() const { return mappedData + fileSize; }
};
