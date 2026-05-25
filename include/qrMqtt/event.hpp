#ifndef QRMQTT_EVENT_HPP
#define QRMQTT_EVENT_HPP

#include <string>

namespace qrmqtt
{
struct ValidationResult
{
    bool approved = false;
    std::string code_id;
    std::string nonce;
    std::string reason;
};

std::string iso8601Now();
std::string jsonEscape(const std::string &value);
std::string statusJson(const std::string &device_id, const std::string &status);
std::string scanEventJson(const std::string &device_id, const ValidationResult &result);
}

#endif
