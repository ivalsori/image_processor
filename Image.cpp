#include "Image.h"
#include "ImageException.h"

Image::Pixel::Pixel(unsigned char r, unsigned char g, unsigned char b)
    : red(r / DEPTH), green(g / DEPTH), blue(b / DEPTH){};

Image::Pixel::Pixel() : red(0), green(0), blue(0){};

Image::Pixel* Image::Pixel::operator+=(const Pixel& other) {
    red += other.red;
    green += other.green;
    blue += other.blue;
    return this;
}

Image::Image(const std::vector<std::vector<Pixel>>& grid) : grid_(grid) {
    if (!grid_.empty()) {
        if (grid[0].empty()) {
            throw InvalidConstructor();
        }
        size_t size = grid[0].size();
        for (const auto& row : grid_) {
            if (row.size() != size) {
                throw InvalidConstructor();
            }
        }
    }
}

Image::Image(const std::vector<std::vector<Pixel>>& grid, int32_t hor_res, int32_t ver_res) : Image(grid) {
    hor_res_ = hor_res;
    ver_res_ = ver_res;
}

Image::Pixel& Image::At(size_t x, size_t y) {
    if (x < grid_.size() && y < grid_[x].size()) {
        return grid_[x][y];
    } else {
        throw OutOfBounds(x, y, GetHeight(), GetWidth());
    }
}

const Image::Pixel& Image::At(size_t x, size_t y) const {
    if (x < grid_.size() && y < grid_[x].size()) {
        return grid_[x][y];
    } else {
        throw OutOfBounds(x, y, GetHeight(), GetWidth());
    }
}

size_t Image::GetHeight() const {
    return grid_.size();
}

size_t Image::GetWidth() const {
    if (!grid_.empty()) {
        return grid_[0].size();
    } else {
        return 0;
    }
}

std::pair<int32_t, int32_t> Image::GetRes() const {
    return {hor_res_, ver_res_};
}

void Image::Resize(size_t new_height, size_t new_width) {
    grid_.resize(new_height);
    for (auto& row : grid_) {
        row.resize(new_width);
    }
}
