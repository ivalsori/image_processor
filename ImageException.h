#pragma once

#include <exception>
#include <string>

class ImageException : public std::exception {
private:
    inline static const std::string NAME = "Error";

protected:
    std::string message_;
    ImageException();

public:
    explicit ImageException(const std::string& message);
    const char* what() const noexcept final;
};

class OutOfBounds : public ImageException {
private:
    inline static const std::string MESSAGE = "Trying to access the pixel (";

public:
    OutOfBounds(size_t x, size_t y, size_t height, size_t width)
        : ImageException(MESSAGE + std::to_string(y) + ", " + std::to_string(x) + ") in an image of size " +
                         std::to_string(width) + 'x' + std::to_string(height)){};
};

class InvalidConstructor : public ImageException {
private:
    inline static const std::string MESSAGE = "Trying to construct an image with incorrect argument(s)";

public:
    InvalidConstructor() : ImageException(MESSAGE){};
};

class FileException : public ImageException {
private:
    inline static const std::string NAME = "File";

protected:
    FileException();

public:
    explicit FileException(const std::string& message);
    FileException(const std::string& message, const char* filename);
    void SetFile(const char* filename);
};

class OpenFileError : public FileException {
private:
    inline static const std::string MESSAGE = "Cannot open file";

public:
    OpenFileError() : FileException(MESSAGE){};
    explicit OpenFileError(const char* filename) : FileException(MESSAGE, filename){};
};

class ReadFileError : public FileException {
private:
    inline static const std::string MESSAGE = "Cannot read from file";

public:
    ReadFileError() : FileException(MESSAGE){};
    explicit ReadFileError(const char* filename) : FileException(MESSAGE, filename){};
};

class WriteFileError : public FileException {
private:
    inline static const std::string MESSAGE = "Cannot write to file";

public:
    WriteFileError() : FileException(MESSAGE){};
    explicit WriteFileError(const char* filename) : FileException(MESSAGE, filename){};
};

class WrongFileFormat : public FileException {
private:
    inline static const std::string MESSAGE = "File not in an acceptable format";

public:
    WrongFileFormat() : FileException(MESSAGE){};
    explicit WrongFileFormat(const char* filename) : FileException(MESSAGE, filename){};
};

class DamagedFile : public FileException {
private:
    inline static const std::string MESSAGE = "File is damaged";

public:
    DamagedFile() : FileException(MESSAGE){};
    explicit DamagedFile(const char* filename) : FileException(MESSAGE, filename){};
};

class FilterException : public ImageException {
private:
    inline static const std::string NAME = "Filter";

protected:
    FilterException();

public:
    explicit FilterException(const std::string& message);
    FilterException(const std::string& message, const std::string& filter);
    void SetFilter(const std::string& filter);
};

class BrokenFilter : public FilterException {
private:
    inline static const std::string MESSAGE = "Filter crashed unexpectedly";

public:
    BrokenFilter() : FilterException(MESSAGE){};
    explicit BrokenFilter(const std::string& filter) : FilterException(MESSAGE, filter){};
};

class ProhibitedValue : public FilterException {
public:
    ProhibitedValue(const std::string& value, const std::string& value_id)
        : FilterException("Value " + value + " is prohibited in argument " + value_id){};
    ProhibitedValue(const std::string& value, const std::string& value_id, const std::string& filter);
};

class InputException : public ImageException {
private:
    inline static const std::string NAME = "Input";

protected:
    InputException();

public:
    explicit InputException(const std::string& message);
};

class NoOutput : public InputException {
private:
    inline static const std::string MESSAGE = "Output file not specified";

public:
    NoOutput() : InputException(MESSAGE){};
};

class OptionException : public InputException {
private:
    inline static const std::string NAME = "Option";

protected:
    OptionException();

public:
    OptionException(const std::string& message, const char* option);
};

class NoOptionName : public OptionException {
private:
    inline static const std::string MESSAGE = "Expected option name";

public:
    NoOptionName() : OptionException(MESSAGE, "-"){};
};

class UnknownOption : public OptionException {
private:
    inline static const std::string MESSAGE = "Unknown option";

public:
    explicit UnknownOption(const char* option) : OptionException(MESSAGE, option){};
};

class TooManyArguments : public OptionException {
private:
    inline static const std::string MESSAGE = "Too many arguments given (expected ";

public:
    TooManyArguments(const char* option, size_t args) : OptionException(MESSAGE + std::to_string(args) + ')', option){};
};

class TooFewArguments : public OptionException {
private:
    inline static const std::string MESSAGE = "Too few arguments given (expected ";

public:
    TooFewArguments(const char* option, size_t args) : OptionException(MESSAGE + std::to_string(args) + ')', option){};
};

class ArgumentException : public OptionException {
private:
    inline static const std::string NAME = "Argument";

protected:
    ArgumentException();

public:
    ArgumentException(const char* argument, const char* in_option, const std::string& message);
};

class WrongType : public ArgumentException {
private:
    inline static const std::string MESSAGE = "is not of type ";

public:
    WrongType(const char* argument, const char* in_option, const std::string& type)
        : ArgumentException(argument, in_option, MESSAGE + type){};
};

class ArgOutOfRange : public ArgumentException {
private:
    inline static const std::string MESSAGE = "is out of range of type ";

public:
    ArgOutOfRange(const char* argument, const char* in_option, const std::string& type)
        : ArgumentException(argument, in_option, MESSAGE + type){};
};
