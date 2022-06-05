#pragma once
#include <stdexcept>

class InvalidTokenError : public std::runtime_error {
public:
    InvalidTokenError(const std::string& token, const std::string& what);
    std::string Token() const;

private:
    std::string token_;
};

class HTTPError : public std::runtime_error {
public:
    HTTPError(int http_index, const std::string& what);
    int HTTPIndex() const;

private:
    int http_index_;
};