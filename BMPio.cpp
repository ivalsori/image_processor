#include "BMPio.h"

#include <bit>

#include "ImageException.h"

const uint16_t BITS_PER_PIXEL = 24;
const uint32_t OFFSET = 54;
const uint32_t BMP_HEADER_SIZE = 14;
const uint32_t DIB_HEADER_SIZE = 40;

template <typename INT>
INT ReadVar(std::ifstream& in) {
    union {
        INT i;
        char c[sizeof(INT)];
    } var;
    in.read(var.c, sizeof(INT));
    if constexpr (std::endian::native == std::endian::big) {
        std::reverse(var.c, var.c + sizeof(INT));
    }
    return var.i;
}

template <>
Image::Pixel ReadVar<Image::Pixel>(std::ifstream& in) {
    unsigned char bgr[3];
    in.read(reinterpret_cast<char*>(bgr), 3);
    return Image::Pixel(bgr[2], bgr[1], bgr[0]);
}

template <typename INT>
void WriteVar(std::ofstream& out, const INT& n) {
    union {
        INT i;
        char c[sizeof(INT)];
    } var;
    var.i = n;
    if constexpr (std::endian::native == std::endian::big) {
        std::reverse(var.c, var.c + sizeof(INT));
    }
    out.write(var.c, sizeof(INT));
}

template <>
void WriteVar<Image::Pixel>(std::ofstream& out, const Image::Pixel& pixel) {
    unsigned char bgr[3];
    bgr[0] = static_cast<unsigned char>(pixel.blue * Image::Pixel::DEPTH);
    bgr[1] = static_cast<unsigned char>(pixel.green * Image::Pixel::DEPTH);
    bgr[2] = static_cast<unsigned char>(pixel.red * Image::Pixel::DEPTH);
    out.write(reinterpret_cast<char*>(bgr), 3);
}

void ReadBMP::ReadBMPHeader() {
    infile_.seekg(0);
    unsigned char bm[2];
    infile_.read(reinterpret_cast<char*>(bm), 2);
    if (bm[0] != 'B' || bm[1] != 'M') {
        throw WrongFileFormat();
    }
    file_size_ = ReadVar<uint32_t>(infile_);
    infile_.seekg(4, std::ios_base::cur);
    offset_ = ReadVar<uint32_t>(infile_);
    if (!infile_.good()) {
        throw ReadFileError();
    }
}

void ReadBMP::ReadDIBHeader() {
    infile_.seekg(BMP_HEADER_SIZE);
    uint32_t header_size = ReadVar<uint32_t>(infile_);
    if (header_size != DIB_HEADER_SIZE) {
        throw WrongFileFormat();
    }
    width_ = ReadVar<int32_t>(infile_);
    height_ = -ReadVar<int32_t>(infile_);
    if ((width_ == 0) != (height_ == 0)) {
        throw DamagedFile();
    }
    uint16_t color_planes = ReadVar<uint16_t>(infile_);
    if (color_planes != 1) {
        throw WrongFileFormat();
    }
    uint16_t bits_per_pixel = ReadVar<uint16_t>(infile_);
    if (bits_per_pixel != BITS_PER_PIXEL) {
        throw WrongFileFormat();
    }
    uint32_t compression = ReadVar<uint32_t>(infile_);
    if (compression != 0) {
        throw WrongFileFormat();
    }
    uint32_t image_size = ReadVar<uint32_t>(infile_);
    if (image_size != 0 &&
        (image_size != static_cast<uint32_t>((3 * std::abs(width_) + 3) / 4 * 4) * std::abs(height_) ||
         image_size + offset_ != file_size_)) {
        throw DamagedFile();
    }
    hor_res_ = ReadVar<int32_t>(infile_);
    ver_res_ = ReadVar<int32_t>(infile_);
    if (!infile_.good()) {
        throw ReadFileError();
    }
}

void ReadBMP::operator()(const char* filename, Image& image) {
    infile_.open(filename, std::ios::binary | std::ios::in);

    if (!infile_.is_open()) {
        throw OpenFileError(filename);
    }
    try {
        ReadBMPHeader();
        ReadDIBHeader();
    } catch (FileException& e) {
        e.SetFile(filename);
        throw e;
    }
    std::vector<std::vector<Image::Pixel>> grid(std::abs(height_), std::vector<Image::Pixel>(std::abs(width_)));
    infile_.seekg(offset_);
    for (size_t i = std::abs(std::min(1, height_)); i > 0 && i <= std::abs(height_); i += (height_ > 0) ? 1 : -1) {
        for (size_t j = std::abs(std::min(1, width_)); j > 0 && j <= std::abs(width_); j += (width_ > 0) ? 1 : -1) {
            grid[i - 1][j - 1] = ReadVar<Image::Pixel>(infile_);
        }
        infile_.seekg((4 - (3 * std::abs(width_)) % 4) % 4, std::ios_base::cur);
    }
    if (!infile_.good()) {
        throw ReadFileError(filename);
    }
    image = Image(grid, hor_res_, ver_res_);
    infile_.close();
}

ReadBMP::ReadBMP(const char* filename, Image& image) {
    operator()(filename, image);
}

ReadBMP::~ReadBMP() {
    if (infile_.is_open()) {
        infile_.close();
    }
}

void WriteBMP::WriteBMPHeader(const Image& image) {
    outfile_.seekp(0);
    outfile_.write("BM", 2);
    uint32_t file_size = OFFSET + image.GetHeight() * ((image.GetWidth() * 3 + 3) / 4 * 4);
    WriteVar(outfile_, file_size);
    WriteVar<uint32_t>(outfile_, 0);  // reserved
    WriteVar<uint32_t>(outfile_, OFFSET);
    if (!outfile_.good()) {
        throw WriteFileError();
    }
}

void WriteBMP::WriteDIBHeader(const Image& image) {
    outfile_.seekp(BMP_HEADER_SIZE);
    WriteVar<uint32_t>(outfile_, DIB_HEADER_SIZE);
    WriteVar(outfile_, static_cast<int32_t>(image.GetWidth()));
    WriteVar(outfile_, static_cast<int32_t>(image.GetHeight()));
    WriteVar<uint16_t>(outfile_, 1);  // color planes
    WriteVar(outfile_, BITS_PER_PIXEL);
    WriteVar<uint64_t>(outfile_, 0);  // compression method and image size
    auto [hor, ver] = image.GetRes();
    WriteVar(outfile_, hor);
    WriteVar(outfile_, ver);
    WriteVar<uint64_t>(outfile_, 0);
    if (!outfile_.good()) {
        throw WriteFileError();
    }
}

void WriteBMP::operator()(const char* filename, const Image& image) {
    outfile_.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!outfile_.is_open()) {
        throw OpenFileError(filename);
    }
    try {
        WriteBMPHeader(image);
        WriteDIBHeader(image);
    } catch (FileException& e) {
        e.SetFile(filename);
        throw e;
    }
    outfile_.seekp(OFFSET);
    char align[4];
    for (size_t i = image.GetHeight(); i > 0; --i) {
        for (size_t j = 0; j < image.GetWidth(); ++j) {
            WriteVar(outfile_, image.At(i - 1, j));
        }
        outfile_.write(align, static_cast<int32_t>((4 - (3 * image.GetWidth()) % 4) % 4));
    }
    if (!outfile_.good()) {
        throw WriteFileError(filename);
    }
    outfile_.close();
}

WriteBMP::WriteBMP(const char* filename, const Image& image) {
    operator()(filename, image);
}

WriteBMP::~WriteBMP() {
    if (outfile_.is_open()) {
        outfile_.close();
    }
}
