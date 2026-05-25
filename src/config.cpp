#include <qrMqtt/config.hpp>

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace qrmqtt
{
namespace
{
std::string trim(const std::string &input)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t begin = input.find_first_not_of(whitespace);
    if (begin == std::string::npos)
    {
        return "";
    }
    const std::size_t end = input.find_last_not_of(whitespace);
    return input.substr(begin, end - begin + 1);
}

std::string valueOf(const std::unordered_map<std::string, std::string> &values,
                    const std::string &key, const std::string &fallback)
{
    const auto value = values.find(key);
    return value == values.end() ? fallback : value->second;
}

bool booleanOf(const std::unordered_map<std::string, std::string> &values,
               const std::string &key, bool fallback)
{
    const std::string value = valueOf(values, key, fallback ? "true" : "false");
    if (value == "true" || value == "yes" || value == "1")
    {
        return true;
    }
    if (value == "false" || value == "no" || value == "0")
    {
        return false;
    }
    throw std::runtime_error("Invalid boolean configuration value for " + key);
}

int integerOf(const std::unordered_map<std::string, std::string> &values,
              const std::string &key, int fallback)
{
    const std::string raw = valueOf(values, key, std::to_string(fallback));
    try
    {
        std::size_t read = 0;
        const int value = std::stoi(raw, &read);
        if (read != raw.size())
        {
            throw std::invalid_argument("suffix");
        }
        return value;
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("Invalid integer configuration value for " + key);
    }
}
}

Config Config::load(const std::string &path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot open configuration file: " + path);
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    std::size_t number = 0;
    while (std::getline(input, line))
    {
        ++number;
        line = trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }
        const std::size_t separator = line.find('=');
        if (separator == std::string::npos)
        {
            throw std::runtime_error("Invalid configuration line " + std::to_string(number));
        }
        std::string key = trim(line.substr(0, separator));
        std::string value = trim(line.substr(separator + 1));
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
        {
            value = value.substr(1, value.size() - 2);
        }
        values[key] = value;
    }

    Config config;
    config.device_id = valueOf(values, "device_id", config.device_id);
    config.mqtt.host = valueOf(values, "mqtt.host", config.mqtt.host);
    config.mqtt.port = integerOf(values, "mqtt.port", config.mqtt.port);
    config.mqtt.client_id = valueOf(values, "mqtt.client_id", config.mqtt.client_id);
    config.mqtt.topic_prefix = valueOf(values, "mqtt.topic_prefix", config.mqtt.topic_prefix);
    config.mqtt.username = valueOf(values, "mqtt.username", config.mqtt.username);
    config.mqtt.password = valueOf(values, "mqtt.password", config.mqtt.password);
    config.mqtt.tls_enabled = booleanOf(values, "mqtt.tls_enabled", config.mqtt.tls_enabled);
    config.mqtt.ca_file = valueOf(values, "mqtt.ca_file", config.mqtt.ca_file);
    config.mqtt.cert_file = valueOf(values, "mqtt.cert_file", config.mqtt.cert_file);
    config.mqtt.key_file = valueOf(values, "mqtt.key_file", config.mqtt.key_file);
    config.mqtt.qos = integerOf(values, "mqtt.qos", config.mqtt.qos);
    const int expiry_seconds = integerOf(values, "mqtt.message_expiry_seconds",
                                         static_cast<int>(config.mqtt.message_expiry_seconds));
    if (expiry_seconds < 0)
    {
        throw std::runtime_error("mqtt.message_expiry_seconds cannot be negative.");
    }
    config.mqtt.message_expiry_seconds = static_cast<unsigned int>(expiry_seconds);
    config.mqtt.spool_file = valueOf(values, "mqtt.spool_file", config.mqtt.spool_file);
    config.camera.device_index = integerOf(values, "camera.device_index", config.camera.device_index);
    config.camera.frame_delay_ms = integerOf(values, "camera.frame_delay_ms", config.camera.frame_delay_ms);
    config.camera.duplicate_window_seconds = integerOf(
        values, "camera.duplicate_window_seconds", config.camera.duplicate_window_seconds);
    config.security.require_signed_tokens = booleanOf(
        values, "security.require_signed_tokens", config.security.require_signed_tokens);
    config.security.hmac_secret = valueOf(values, "security.hmac_secret", "");
    config.security.nonce_file = valueOf(values, "security.nonce_file", config.security.nonce_file);

    const char *secret = std::getenv("QRMQTT_HMAC_SECRET");
    if (secret != nullptr && secret[0] != '\0')
    {
        config.security.hmac_secret = secret;
    }
    config.validate();
    return config;
}

void Config::validate() const
{
    if (device_id.empty() || mqtt.host.empty() || mqtt.client_id.empty() || mqtt.topic_prefix.empty())
    {
        throw std::runtime_error("Device ID and MQTT identity/topic/host cannot be empty.");
    }
    if (mqtt.port < 1 || mqtt.port > 65535 || mqtt.qos < 0 || mqtt.qos > 2)
    {
        throw std::runtime_error("MQTT port or QoS is outside its supported range.");
    }
    if (mqtt.tls_enabled && mqtt.ca_file.empty())
    {
        throw std::runtime_error("mqtt.ca_file is required when TLS is enabled.");
    }
    if ((!mqtt.cert_file.empty() && mqtt.key_file.empty())
        || (mqtt.cert_file.empty() && !mqtt.key_file.empty()))
    {
        throw std::runtime_error("Both mqtt.cert_file and mqtt.key_file are required for client TLS.");
    }
    if (camera.frame_delay_ms < 1 || camera.duplicate_window_seconds < 0)
    {
        throw std::runtime_error("Camera delay and duplicate window are invalid.");
    }
    if (security.require_signed_tokens && security.hmac_secret.empty())
    {
        throw std::runtime_error("Set QRMQTT_HMAC_SECRET when signed tokens are required.");
    }
}
}
