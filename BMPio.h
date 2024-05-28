#pragma once

#include <fstream>

#include "Image.h"

class ReadBMP {
private:
    std::ifstream infile_;
    uint32_t file_size_;
    uint32_t offset_;
    int32_t width_;
    int32_t height_;
    int32_t hor_res_;
    int32_t ver_res_;
    void ReadBMPHeader();
    void ReadDIBHeader();

public:
    ReadBMP() = default;
    ReadBMP(const char* filename, Image& image);
    void operator()(const char* filename, Image& image);
    ~ReadBMP();
};

class WriteBMP {
private:
    std::ofstream outfile_;
    void WriteBMPHeader(const Image& image);
    void WriteDIBHeader(const Image& image);

public:
    WriteBMP() = default;
    WriteBMP(const char* filename, const Image& image);
    void operator()(const char* filename, const Image& image);
    ~WriteBMP();
};
