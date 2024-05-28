#include "ImageRedactor.h"

#include <string>
#include <string_view>
#include <memory>

#include "ImageException.h"
#include "BMPio.h"

void Interpret(size_t& dest, char** argv, size_t& i, size_t option, size_t expected_args) {
    ++i;
    size_t pos = 0;
    std::string_view view(argv[i]);
    try {
        dest = std::stoul(view.data(), &pos);
        if (pos < view.size()) {
            throw std::invalid_argument("not size_t");
        }
        if (view[0] == '-') {
            throw WrongType(view.data(), argv[option], "size_t");
        }
    } catch (const std::invalid_argument& e) {
        if (view[0] == '-') {
            throw TooFewArguments(argv[option], expected_args);
        } else {
            throw WrongType(view.data(), argv[option], "size_t");
        }
    } catch (const std::out_of_range& e) {
        throw ArgOutOfRange(view.data(), argv[option], "size_t");
    }
}

void Interpret(long double& dest, char** argv, size_t& i, size_t option, size_t expected_args) {
    ++i;
    size_t pos = 0;
    std::string_view view(argv[i]);
    try {
        dest = std::stold(view.data(), &pos);
        if (pos < view.size()) {
            throw std::invalid_argument("not long double");
        }
    } catch (const std::invalid_argument& e) {
        if (view[0] == '-') {
            throw TooFewArguments(argv[option], expected_args);
        } else {
            throw WrongType(view.data(), argv[option], "long double");
        }
    } catch (const std::out_of_range& e) {
        throw ArgOutOfRange(view.data(), argv[option], "long double");
    }
}

void ImageRedactor::ApplyFilter(Filter& filter) {
    try {
        filter(image_);
    } catch (FilterException& e) {
        e.SetFilter(filter.GetName());
        throw e;
    } catch (const std::exception& e) {
        throw BrokenFilter(filter.GetName());
    }
}

void ImageRedactor::Execute(size_t argc, char** argv) {
    std::vector<std::unique_ptr<Filter>> filters;
    size_t option = 0;
    for (size_t i = 0; i < argc; ++i) {
        std::string_view view(argv[i]);
        size_t last = option;
        option = i;
        if (view == "-crop") {
            if (i + 2 >= argc) {
                throw TooFewArguments(view.data(), 2);
            }
            size_t width = 0;
            size_t height = 0;
            Interpret(width, argv, i, option, 2);
            Interpret(height, argv, i, option, 2);
            filters.emplace_back(std::make_unique<CropFilter>(width, height));
        } else if (view == "-gs") {
            filters.emplace_back(std::make_unique<GrayscaleFilter>());
        } else if (view == "-neg") {
            filters.emplace_back(std::make_unique<NegativeFilter>());
        } else if (view == "-sharp") {
            filters.emplace_back(std::make_unique<SharpeningFilter>());
        } else if (view == "-edge") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            long double threshold = 0;
            Interpret(threshold, argv, i, option, 1);
            filters.emplace_back(std::make_unique<EdgeDetectionFilter>(threshold));
        } else if (view == "-blur") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            long double sigma = 0;
            Interpret(sigma, argv, i, option, 1);
            filters.emplace_back(std::make_unique<GaussianFilter>(sigma));
        } else if (view == "-burn") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            ++i;
            std::shared_ptr<Image> second(std::make_unique<Image>());
            ReadBMP(argv[i], *second);
            filters.emplace_back(std::make_unique<ColorBurnFilter>(second));
        } else if (view == "-dodge") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            ++i;
            std::shared_ptr<Image> second(std::make_unique<Image>());
            ReadBMP(argv[i], *second);
            filters.emplace_back(std::make_unique<ColorDodgeFilter>(second));
        } else if (view == "-chalk") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            long double sigma = 0;
            Interpret(sigma, argv, i, option, 1);
            filters.emplace_back(std::make_unique<ChalkFilter>(sigma));
        } else if (view == "-sketch") {
            if (i + 1 >= argc) {
                throw TooFewArguments(view.data(), 1);
            }
            long double sigma = 0;
            Interpret(sigma, argv, i, option, 1);
            filters.emplace_back(std::make_unique<SketchFilter>(sigma));
        } else if (view == "-") {
            throw NoOptionName();
        } else if (!view.empty()) {
            if (view[0] == '-' || i == 0) {
                throw UnknownOption(view.data());
            } else {
                throw TooManyArguments(argv[last], i - last - 1);
            }
        }
    }
    for (auto& filter : filters) {
        ApplyFilter(*filter);
    }
}
