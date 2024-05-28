#include <iostream>

#include "ImageException.h"
#include "Image.h"
#include "BMPio.h"
#include "ImageRedactor.h"

const std::string HELP = R"(Usage: image_processor <path to input image> <path to output image> [-crop <width> <height>]
                        [-gs] [-neg] [-sharp] [-edge <threshold>] [-blur <sigma>]
                        [-burn <path to image>] [-dodge <path to image>]
                        [-chalk <sigma>] [-sketch <sigma>]

Applies filters to the BMP image and saves the results to specified path.
If no arguments are given, shows this page.

Option                    Filter Name       Description
-crop <width> <height>    Crop              Crops the image to the given width and height. The top
                                            left part of the image is used
-gs                       Grayscale         Converts the image to grayscale
-neg                      Negative          Converts the image to negative
-sharp                    Sharpening        Sharpens the image
-edge <threshold>         Edge Detection    Detects edges of the image
-blur <sigma>             Gaussian Blur     Blurs the image by a Gaussian function with sigma as
                                            the standard deviation of the Gaussian distribution
-burn <path to image>     Color Burn        Blends the image with the image at specified path.
                                            Darkens the base color to reflect the blend color by
                                            increasing the contrast between the two. Blending with
                                            white produces no change
-dodge <path to image>    Color Dodge       Blends the image with the image at specified path.
                                            Brightens the base color to reflect the blend color by
                                            decreasing contrast between the two. Blending with
                                            black produces no change
-chalk <sigma>            Chalk Board       Makes the image look as if drawn on a chalk board
-sketch <sigma>           Sketch            Makes the image look as if drawn with a pencil
)";

int main(int argc, char** argv) {
    try {
        if (argc == 1) {
            std::cout << HELP;
        } else if (argc >= 3) {
            Image image;
            ReadBMP(argv[1], image);
            ImageRedactor redactor(image);
            redactor.Execute(argc - 3, argv + 3);

            // trying to write results
            bool write_success = false;
            std::string filename = argv[2];
            while (!write_success) {
                write_success = true;
                try {
                    WriteBMP(filename.c_str(), image);
                } catch (const FileException& e) {
                    write_success = false;
                    std::cout << e.what() << "\nPlease, enter the path to output file again:" << std::endl;
                    std::cin >> filename;
                }
            }
        } else {
            throw NoOutput();
        }
    } catch (const ImageException& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Somewhere, something went terribly wrong: " << e.what() << std::endl;
    }
    return 0;
}
