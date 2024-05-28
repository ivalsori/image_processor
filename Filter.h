#pragma once

#include <array>
#include <vector>
#include <memory>

#include "Image.h"

class Filter {
private:
    inline static const std::string NAME = "Filter";

public:
    virtual const std::string& GetName() const {
        return NAME;
    };
    virtual void operator()(Image& image) = 0;
    virtual ~Filter() = default;
};

class CropFilter : public Filter {
private:
    inline static const std::string NAME = "CropFilter";
    size_t new_height_, new_width_;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    CropFilter(size_t width, size_t height) : new_height_(height), new_width_(width){};
    void operator()(Image& image) override;
};

class ByPixelFilter : public Filter {
private:
    inline static const std::string NAME = "ByPixelFilter";
    virtual void ComputePixel(Image& image, size_t x, size_t y) = 0;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    void operator()(Image& image) override;
};

class GrayscaleFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "GrayscaleFilter";
    inline static const long double RED_MULT = 0.299;
    inline static const long double GREEN_MULT = 0.587;
    inline static const long double BLUE_MULT = 0.114;
    void ComputePixel(Image& image, size_t x, size_t y) override;

public:
    const std::string& GetName() const override {
        return NAME;
    }
};

class NegativeFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "NegativeFilter";
    void ComputePixel(Image& image, size_t x, size_t y) override;

public:
    const std::string& GetName() const override {
        return NAME;
    }
};

template <typename T>
class MatrixFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "MatrixFilter";
    std::array<std::array<T, 3>, 3> matrix_;
    std::vector<Image::Pixel> prev_line_, cur_line_;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit MatrixFilter(const std::array<std::array<T, 3>, 3>& matrix) : matrix_(matrix){};
    void ComputePixel(Image& image, size_t x, size_t y) override {
        if (image.GetHeight() == 0) {
            return;
        }
        Image::Pixel pixel;
        if (y == 0) {
            std::swap(prev_line_, cur_line_);
            if (x < 2) {
                cur_line_.resize(image.GetWidth());
            }
        }
        cur_line_[y] = image.At(x, y);
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                size_t coord_x = (x + i > 0) ? x + i : 1;
                coord_x = (coord_x <= image.GetHeight()) ? coord_x : image.GetHeight();
                size_t coord_y = (y + j > 0) ? y + j : 1;
                coord_y = (coord_y <= image.GetWidth()) ? coord_y : image.GetWidth();
                --coord_x;
                --coord_y;
                if (coord_x < x) {
                    pixel += prev_line_[coord_y] * matrix_[i][j];
                } else if (coord_x > x || coord_y > y) {
                    pixel += image.At(coord_x, coord_y) * matrix_[i][j];
                } else {
                    pixel += cur_line_[coord_y] * matrix_[i][j];
                }
            }
        }

        if (pixel.red < 0) {
            pixel.red = 0;
        }
        if (pixel.red > 1) {
            pixel.red = 1;
        }
        if (pixel.green < 0) {
            pixel.green = 0;
        }
        if (pixel.green > 1) {
            pixel.green = 1;
        }
        if (pixel.blue < 0) {
            pixel.blue = 0;
        }
        if (pixel.blue > 1) {
            pixel.blue = 1;
        }

        image.At(x, y) = pixel;
    }
};

class SharpeningFilter : public MatrixFilter<char> {
private:
    inline static const std::string NAME = "SharpeningFilter";
    inline static const std::array<std::array<char, 3>, 3> SHARP_MATRIX = {{{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}}};

public:
    const std::string& GetName() const override {
        return NAME;
    }
    SharpeningFilter() : MatrixFilter<char>(SHARP_MATRIX){};
};

template <typename... Args>
class QueueFilter;

template <typename THead, typename... TTail>
class QueueFilter<THead, TTail...> : public QueueFilter<TTail...> {
private:
    THead first_filter_;

public:
    explicit QueueFilter(const THead& last_filter, const TTail&... next_filters)
        : QueueFilter<TTail...>(next_filters...), first_filter_(last_filter){};
    void operator()(Image& image) override {
        first_filter_(image);
        QueueFilter<TTail...>::operator()(image);
    }
};

template <>
class QueueFilter<> : public Filter {
private:
    inline static const std::string NAME = "QueueFilter";

public:
    const std::string& GetName() const override {
        return NAME;
    }
    void operator()(Image& image) override{};
};

class ThresholdFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "ThresholdFilter";
    long double threshold_;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit ThresholdFilter(long double threshold) : threshold_(threshold){};
    void ComputePixel(Image& image, size_t x, size_t y) override;
};

class EdgeDetectionFilter : public QueueFilter<GrayscaleFilter, MatrixFilter<char>, ThresholdFilter> {
private:
    inline static const std::string NAME = "EdgeDetectionFilter";
    inline static const std::array<std::array<char, 3>, 3> EDGE_MATRIX = {{{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}}};

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit EdgeDetectionFilter(long double threshold)
        : QueueFilter<GrayscaleFilter, MatrixFilter<char>, ThresholdFilter>(
              GrayscaleFilter(), MatrixFilter<char>(EDGE_MATRIX), ThresholdFilter(threshold)){};
};

class GaussianFilter : public Filter {
private:
    inline static const std::string NAME = "ByLineFilter";
    std::vector<long double> gauss_;
    std::vector<Image::Pixel> prevs_;
    long double sigma_;
    ssize_t size_;

    void ComputeLine(Image& image, size_t line, size_t y);
    void ComputeRow(Image& image, size_t x, size_t row);

public:
    const std::string& GetName() const override {
        return NAME;
    }
    void operator()(Image& image) override;
    explicit GaussianFilter(const long double& sigma) : sigma_(std::abs(sigma)){};
};

class ColorDodgeFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "ColorDodgeFilter";
    std::shared_ptr<Image> second_;
    void ComputePixel(Image& image, size_t x, size_t y) override;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit ColorDodgeFilter(std::shared_ptr<Image> second) : second_(second){};
};

class ColorBurnFilter : public ByPixelFilter {
private:
    inline static const std::string NAME = "ColorBurnFilter";
    std::shared_ptr<Image> second_;
    void ComputePixel(Image& image, size_t x, size_t y) override;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit ColorBurnFilter(std::shared_ptr<Image> second) : second_(second){};
};

class SketchFilter : public Filter {
private:
    inline static const std::string NAME = "SketchFilter";
    long double sigma_;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit SketchFilter(const long double& sigma) : sigma_(sigma){};
    void operator()(Image& image) override;
};

class ChalkFilter : public Filter {
private:
    inline static const std::string NAME = "ChalkFilter";
    long double sigma_;

public:
    const std::string& GetName() const override {
        return NAME;
    }
    explicit ChalkFilter(const long double& sigma) : sigma_(sigma){};
    void operator()(Image& image) override;
};
