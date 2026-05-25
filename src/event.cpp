#include <qrMqtt/event.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace qrmqtt
{
std::string iso8601Now()
{
    const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm utc;
    gmtime_r(&now, &utc);
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

std::string jsonEscape(const std::string &value)
{
    std::ostringstream output;
    for (const unsigned char character : value)
    {
        switch (character)
        {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (character < 0x20)
            {
                output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                       << static_cast<int>(character) << std::dec;
            }
            else
            {
                output << character;
            }
        }
    }
    return output.str();
}

std::string statusJson(const std::string &device_id, const std::string &status)
{
    return "{\"device_id\":\"" + jsonEscape(device_id)
        + "\",\"status\":\"" + jsonEscape(status)
        + "\",\"timestamp\":\"" + iso8601Now() + "\",\"version\":\"2.0.0\"}";
}

std::string scanEventJson(const std::string &device_id, const ValidationResult &result)
{
    return "{\"device_id\":\"" + jsonEscape(device_id)
        + "\",\"event\":\"qr_scanned\",\"timestamp\":\"" + iso8601Now()
        + "\",\"code_id\":\"" + jsonEscape(result.code_id)
        + "\",\"result\":\"" + (result.approved ? "approved" : "rejected")
        + "\",\"reason\":\"" + jsonEscape(result.reason) + "\"}";
}
}
