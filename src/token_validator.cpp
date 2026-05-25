#include <qrMqtt/token_validator.hpp>

#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace qrmqtt
{
namespace
{
std::vector<std::string> split(const std::string &payload)
{
    std::vector<std::string> fields;
    std::size_t begin = 0;
    while (true)
    {
        const std::size_t separator = payload.find(':', begin);
        fields.push_back(payload.substr(begin, separator - begin));
        if (separator == std::string::npos)
        {
            return fields;
        }
        begin = separator + 1;
    }
}

std::string hexadecimal(const unsigned char *data, unsigned int length)
{
    std::ostringstream output;
    for (unsigned int i = 0; i < length; ++i)
    {
        output << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<unsigned int>(data[i]);
    }
    return output.str();
}

std::string fingerprint(const std::string &value)
{
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char *>(value.data()), value.size(), digest);
    return hexadecimal(digest, SHA256_DIGEST_LENGTH).substr(0, 16);
}

std::string signature(const std::string &data, const std::string &secret)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    HMAC(EVP_sha256(), secret.data(), static_cast<int>(secret.size()),
         reinterpret_cast<const unsigned char *>(data.data()), data.size(), digest, &length);
    return hexadecimal(digest, length);
}

bool constantTimeEqual(const std::string &left, const std::string &right)
{
    return left.size() == right.size()
        && CRYPTO_memcmp(left.data(), right.data(), left.size()) == 0;
}

bool safeField(const std::string &value)
{
    if (value.empty() || value.size() > 128)
    {
        return false;
    }
    for (const unsigned char character : value)
    {
        if (!std::isalnum(character) && character != '-' && character != '_' && character != '.')
        {
            return false;
        }
    }
    return true;
}

bool hexadecimalSignature(const std::string &value)
{
    if (value.size() != 64)
    {
        return false;
    }
    for (const unsigned char character : value)
    {
        if (!std::isxdigit(character))
        {
            return false;
        }
    }
    return true;
}
}

TokenValidator::TokenValidator(const Config::Security &config)
  : config_(config)
{
    loadNonces(std::time(nullptr));
}

void TokenValidator::loadNonces(std::time_t now)
{
    if (config_.nonce_file.empty())
    {
        return;
    }
    std::ifstream input(config_.nonce_file);
    std::string nonce;
    std::time_t expiry = 0;
    while (input >> nonce >> expiry)
    {
        if (expiry > now)
        {
            consumed_nonces_[nonce] = expiry;
        }
    }
}

void TokenValidator::saveNonces() const
{
    if (config_.nonce_file.empty())
    {
        return;
    }
    std::ofstream output(config_.nonce_file, std::ios::trunc);
    if (!output)
    {
        throw std::runtime_error("Cannot write consumed nonce store: " + config_.nonce_file);
    }
    for (const auto &entry : consumed_nonces_)
    {
        output << entry.first << '\t' << entry.second << '\n';
    }
}

ValidationResult TokenValidator::validate(const std::string &payload, std::time_t now)
{
    ValidationResult result;
    result.code_id = fingerprint(payload);
    if (!config_.require_signed_tokens)
    {
        result.approved = true;
        result.reason = "unsigned_tokens_enabled";
        return result;
    }

    const std::vector<std::string> fields = split(payload);
    if (fields.size() != 6 || fields[0] != "qrmqtt" || fields[1] != "v1"
        || !safeField(fields[2]) || !safeField(fields[4])
        || !hexadecimalSignature(fields[5]))
    {
        result.reason = "invalid_format";
        return result;
    }

    result.code_id = fields[2];
    result.nonce = fields[4];
    std::time_t expires = 0;
    try
    {
        std::size_t read = 0;
        expires = static_cast<std::time_t>(std::stoll(fields[3], &read));
        if (read != fields[3].size())
        {
            throw std::invalid_argument("expiry");
        }
    }
    catch (const std::exception &)
    {
        result.reason = "invalid_expiry";
        return result;
    }

    if (expires <= now)
    {
        result.reason = "expired";
        return result;
    }
    if (consumed_nonces_.count(result.nonce) != 0)
    {
        result.reason = "already_used";
        return result;
    }

    const std::string signed_part = fields[0] + ":" + fields[1] + ":" + fields[2]
        + ":" + fields[3] + ":" + fields[4];
    if (!constantTimeEqual(signature(signed_part, config_.hmac_secret), fields[5]))
    {
        result.reason = "invalid_signature";
        return result;
    }

    consumed_nonces_[result.nonce] = expires;
    saveNonces();
    result.approved = true;
    result.reason = "verified";
    return result;
}
}
