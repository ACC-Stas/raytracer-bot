#include <string>
#include "exceptions.h"

InvalidTokenError::InvalidTokenError(const std::string& token, const std::string& what)
    : runtime_error(what + " Your token " + token + " is invalid") {
    token_ = token;
}

std::string InvalidTokenError::Token() const {
    return token_;
}

HTTPError::HTTPError(int http_index, const std::string& what)
    : runtime_error(what) {
    http_index_ = http_index;
}

int HTTPError::HTTPIndex() const {
    return http_index_;
}
