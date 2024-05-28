#pragma once

#include "Image.h"
#include "Filter.h"

class ImageRedactor {
private:
    Image& image_;

public:
    explicit ImageRedactor(Image& source) : image_(source){};

    void Execute(size_t argc, char** argv);

    void ApplyFilter(Filter& filter);
};
