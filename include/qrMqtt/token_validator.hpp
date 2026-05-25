#ifndef QRMQTT_TOKEN_VALIDATOR_HPP
#define QRMQTT_TOKEN_VALIDATOR_HPP

#include <ctime>
#include <string>
#include <unordered_map>

#include <qrMqtt/config.hpp>
#include <qrMqtt/event.hpp>

namespace qrmqtt
{
class TokenValidator
{
  public:
    explicit TokenValidator(const Config::Security &config);
    ValidationResult validate(const std::string &payload, std::time_t now = std::time(nullptr));

  private:
    Config::Security config_;
    std::unordered_map<std::string, std::time_t> consumed_nonces_;
    void loadNonces(std::time_t now);
    void saveNonces() const;
};
}

#endif
