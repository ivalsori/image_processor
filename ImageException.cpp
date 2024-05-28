#include "ImageException.h"

const char* ImageException::what() const noexcept {
    return message_.c_str();
}

ImageException::ImageException() {
    message_ += NAME + '.';
}

ImageException::ImageException(const std::string& message) {
    message_ += NAME + ": " + message;
}

FileException::FileException() {
    message_ += NAME + '.';
}

FileException::FileException(const std::string& message) {
    message_ += NAME + ": " + message;
}

FileException::FileException(const std::string& message, const char* filename) : FileException(message) {
    SetFile(filename);
}

void FileException::SetFile(const char* filename) {
    (message_ += ": ") += filename;
}

FilterException::FilterException() {
    message_ += NAME + '.';
}

FilterException::FilterException(const std::string& message) {
    message_ += NAME + ": " + message;
}

FilterException::FilterException(const std::string& message, const std::string& filter) : FilterException(message) {
    SetFilter(filter);
}

void FilterException::SetFilter(const std::string& filter) {
    message_ += ": " + filter;
}

ProhibitedValue::ProhibitedValue(const std::string& value, const std::string& value_id, const std::string& filter)
    : ProhibitedValue(value, value_id) {
    SetFilter(filter);
}

InputException::InputException() {
    message_ += NAME + '.';
}

InputException::InputException(const std::string& message) {
    message_ += NAME + ": " + message;
}

OptionException::OptionException() {
    message_ += NAME + '.';
}

OptionException::OptionException(const std::string& message, const char* option) {
    message_ += NAME + ": " + message + ": '" + option + "'";
}

ArgumentException::ArgumentException() {
    message_ += NAME + '.';
}

ArgumentException::ArgumentException(const char* argument, const char* in_option, const std::string& message) {
    message_ += NAME + ": Argument '" + argument + "' in option '" + in_option + "' " + message;
}