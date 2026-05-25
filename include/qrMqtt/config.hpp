#ifndef QRMQTT_CONFIG_HPP
#define QRMQTT_CONFIG_HPP

#include <string>

namespace qrmqtt
{
struct Config
{
    struct Mqtt
    {
        std::string host = "localhost";
        int port = 8883;
        std::string client_id = "qrmqtt-front-door";
        std::string topic_prefix = "qrMqtt/devices/front-door";
        std::string username;
        std::string password;
        bool tls_enabled = true;
        std::string ca_file;
        std::string cert_file;
        std::string key_file;
        int qos = 1;
        unsigned int message_expiry_seconds = 60;
        std::string spool_file = "/var/lib/qrMqtt/outbox.tsv";
    } mqtt;

    struct Camera
    {
        int device_index = 0;
        int frame_delay_ms = 100;
        int duplicate_window_seconds = 10;
    } camera;

    struct Security
    {
        bool require_signed_tokens = true;
        std::string hmac_secret;
        std::string nonce_file = "/var/lib/qrMqtt/consumed-nonces.tsv";
    } security;

    std::string device_id = "front-door";

    static Config load(const std::string &path);
    void validate() const;
};
}

#endif
