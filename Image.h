#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

class Image {
public:
    struct Pixel {
        inline static const long double DEPTH = 255;
        long double red, green, blue;
        Pixel();
        Pixel(unsigned char r, unsigned char g, unsigned char b);
        Pixel(long double r, long double g, long double b) : red(r), green(g), blue(b){};
        Pixel* operator+=(const Pixel& other);
        template <typename T>
        Pixel operator*(const T& n) {
            return Pixel(red * n, green * n, blue * n);
        }
        template <typename T>
        Pixel operator/(const T& n) {
            return Pixel(red / n, green / n, blue / n);
        }
    };

private:
    std::vector<std::vector<Pixel>> grid_;
    int32_t hor_res_ = 1;
    int32_t ver_res_ = 1;

public:
    Image() = default;
    explicit Image(const std::vector<std::vector<Pixel>>& grid);
    Image(const std::vector<std::vector<Pixel>>& grid, int32_t hor_res, int32_t ver_res);

    Pixel& At(size_t x, size_t y);

    const Pixel& At(size_t x, size_t y) const;

    size_t GetHeight() const;

    size_t GetWidth() const;

    std::pair<int32_t, int32_t> GetRes() const;

    void Resize(size_t new_height, size_t new_width);
};
