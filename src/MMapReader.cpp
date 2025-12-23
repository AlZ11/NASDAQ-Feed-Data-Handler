#include "../include/MMapReader.h"
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

MMapReader::MMapReader(const std::string& filename) 
    : fd(-1), mappedData(nullptr), fileSize(0), isValid(false) {
    
    // Open file
    fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return;
    }
    
    // Get file size
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Error: Could not get file size for '" << filename << "'" << std::endl;
        close(fd);
        fd = -1;
        return;
    }
    
    fileSize = sb.st_size;
    
    // Memory map the file (static cast since mmap returns a void* (no pointer arithmetic))
    mappedData = static_cast<char*>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
    
    if (mappedData == MAP_FAILED) {
        std::cerr << "Error: mmap failed for '" << filename << "'" << std::endl;
        mappedData = nullptr;
        close(fd);
        fd = -1;
        return;
    }
    
    isValid = true;
}

MMapReader::~MMapReader() {
    // Clean up memory mapping
    if (mappedData != nullptr && mappedData != MAP_FAILED) {
        munmap(mappedData, fileSize);
        mappedData = nullptr;
    }
    
    // Close file descriptor
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

// Move constructor
MMapReader::MMapReader(MMapReader&& other) noexcept
    : fd(other.fd),
      mappedData(other.mappedData),
      fileSize(other.fileSize),
      isValid(other.isValid) {
    
    // Take ownership from other
    other.fd = -1;
    other.mappedData = nullptr;
    other.fileSize = 0;
    other.isValid = false;
}

// Move assignment
MMapReader& MMapReader::operator=(MMapReader&& other) noexcept {
    if (this != &other) {
        // Clean up current resources
        if (mappedData != nullptr && mappedData != MAP_FAILED) {
            munmap(mappedData, fileSize);
        }
        if (fd != -1) {
            close(fd);
        }
        
        // Transfer ownership
        fd = other.fd;
        mappedData = other.mappedData;
        fileSize = other.fileSize;
        isValid = other.isValid;
        
        // Reset other
        other.fd = -1;
        other.mappedData = nullptr;
        other.fileSize = 0;
        other.isValid = false;
    }
    return *this;
}
