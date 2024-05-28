#include "Filter.h"

#include <cmath>

#include "ImageException.h"

void CropFilter::operator()(Image& image) {
    if (new_width_ == 0) {
        throw ProhibitedValue("0", "<width>");
    }
    if (new_height_ == 0) {
        throw ProhibitedValue("0", "<height>");
    }
    image.Resize(std::min(new_height_, image.GetHeight()), std::min(new_width_, image.GetWidth()));
}

void ByPixelFilter::operator()(Image& image) {
    for (size_t i = 0; i < image.GetHeight(); ++i) {
        for (size_t j = 0; j < image.GetWidth(); ++j) {
            ComputePixel(image, i, j);
        }
    }
}

void GrayscaleFilter::ComputePixel(Image& image, size_t x, size_t y) {
    Image::Pixel& pixel = image.At(x, y);
    long double gray_color = RED_MULT * pixel.red + GREEN_MULT * pixel.green + BLUE_MULT * pixel.blue;
    pixel.red = gray_color;
    pixel.green = gray_color;
    pixel.blue = gray_color;
}

void NegativeFilter::ComputePixel(Image& image, size_t x, size_t y) {
    Image::Pixel& pixel = image.At(x, y);
    pixel.red = 1 - pixel.red;
    pixel.green = 1 - pixel.green;
    pixel.blue = 1 - pixel.blue;
}

void ThresholdFilter::ComputePixel(Image& image, size_t x, size_t y) {
    Image::Pixel& pixel = image.At(x, y);
    bool steps_over = (pixel.red > threshold_) && (pixel.green > threshold_) && (pixel.blue > threshold_);
    pixel.red = steps_over;
    pixel.green = steps_over;
    pixel.blue = steps_over;
}

void GaussianFilter::ComputeLine(Image& image, size_t line, size_t y) {
    if (y == 0) {
        prevs_.resize(image.GetWidth());
        if (sigma_ == 0) {
            throw ProhibitedValue(std::to_string(sigma_), "<sigma>");
        }
        size_ = static_cast<ssize_t>(image.GetWidth());
        if (4 * sigma_ < size_ + 1) {
            size_ = static_cast<ssize_t>(std::ceil(3 * sigma_));
        }
        gauss_.resize(2 * size_ + 1);
        long double g_1 = std::pow(std::numbers::e_v<long double>, -1 / (2 * sigma_ * sigma_));
        for (ssize_t i = -size_; i < size_ + 1; ++i) {
            gauss_[i + size_] = std::pow(g_1, i * i);
        }
    }
    Image::Pixel pixel;
    prevs_[y] = image.At(line, y);
    long double sum = 0;
    for (ssize_t j = static_cast<ssize_t>(y) - size_; j < static_cast<ssize_t>(y) + size_ + 1; ++j) {
        if (j >= 0 && static_cast<size_t>(j) < image.GetWidth()) {
            if (static_cast<size_t>(j) <= y) {
                pixel += prevs_[j] * gauss_[j + size_ - y];
            } else {
                pixel += image.At(line, j) * gauss_[j + size_ - y];
            }
            sum += gauss_[j + size_ - y];
        }
    }
    pixel = pixel / sum;
    image.At(line, y) = pixel;
}

void GaussianFilter::ComputeRow(Image& image, size_t x, size_t row) {
    if (x == 0) {
        prevs_.resize(image.GetHeight());
        size_ = static_cast<ssize_t>(image.GetHeight());
        if (4 * sigma_ < size_ + 1) {
            size_ = static_cast<ssize_t>(std::ceil(3 * sigma_));
        }
        gauss_.resize(2 * size_ + 1);
        long double g_1 = std::pow(std::numbers::e_v<long double>, -1 / (2 * sigma_ * sigma_));
        for (ssize_t i = -size_; i < size_ + 1; ++i) {
            gauss_[i + size_] = std::pow(g_1, i * i);
        }
    }
    Image::Pixel pixel;
    prevs_[x] = image.At(x, row);
    long double sum = 0;
    for (ssize_t i = static_cast<ssize_t>(x) - size_; i < static_cast<ssize_t>(x) + size_ + 1; ++i) {
        if (i >= 0 && static_cast<size_t>(i) < image.GetHeight()) {
            if (static_cast<size_t>(i) <= x) {
                pixel += prevs_[i] * gauss_[i + size_ - x];
            } else {
                pixel += image.At(i, row) * gauss_[i + size_ - x];
            }
            sum += gauss_[i + size_ - x];
        }
    }
    pixel = pixel / sum;
    image.At(x, row) = pixel;
}

void GaussianFilter::operator()(Image& image) {
    for (size_t i = 0; i < image.GetHeight(); ++i) {
        for (size_t j = 0; j < image.GetWidth(); ++j) {
            ComputeLine(image, i, j);
        }
    }
    for (size_t j = 0; j < image.GetWidth(); ++j) {
        for (size_t i = 0; i < image.GetHeight(); ++i) {
            ComputeRow(image, i, j);
        }
    }
}

void Dodge(long double& bg, long double& fg) {
    if (fg >= 1) {
        bg = 1;
    } else {
        if (bg / (1 - fg) > 1) {
            bg = 1;
        } else {
            bg = bg / (1 - fg);
        }
    }
}

void ColorDodgeFilter::ComputePixel(Image& image, size_t x, size_t y) {
    if (x >= second_->GetHeight() || y >= second_->GetWidth()) {
        return;
    };
    Image::Pixel base = image.At(x, y);
    Image::Pixel added = second_->At(x, y);
    Dodge(base.red, added.red);
    Dodge(base.green, added.green);
    Dodge(base.blue, added.blue);
    image.At(x, y) = base;
}

void Burn(long double& bg, long double& fg) {
    if (fg <= 0) {
        bg = 0;
    } else {
        if ((1 - bg) / fg > 1) {
            bg = 0;
        } else {
            bg = 1 - (1 - bg) / fg;
        }
    }
}

void ColorBurnFilter::ComputePixel(Image& image, size_t x, size_t y) {
    if (x >= second_->GetHeight() || y >= second_->GetWidth()) {
        return;
    };
    Image::Pixel base = image.At(x, y);
    Image::Pixel added = second_->At(x, y);
    Burn(base.red, added.red);
    Burn(base.green, added.green);
    Burn(base.blue, added.blue);
    image.At(x, y) = base;
}

void SketchFilter::operator()(Image& image) {
    GrayscaleFilter{}(image);
    std::shared_ptr<Image> second(std::make_shared<Image>(image));
    NegativeFilter{}(*second);
    GaussianFilter{sigma_}(*second);
    ColorDodgeFilter{second}(image);
}

void ChalkFilter::operator()(Image& image) {
    GrayscaleFilter{}(image);
    std::shared_ptr<Image> second(std::make_shared<Image>(image));
    NegativeFilter{}(*second);
    GaussianFilter{sigma_}(*second);
    ColorBurnFilter{second}(image);
}