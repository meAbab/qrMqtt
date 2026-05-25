#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <mosquitto.h>

#include <qrMqtt/config.hpp>
#include <qrMqtt/duplicate_filter.hpp>
#include <qrMqtt/event.hpp>
#include <qrMqtt/mqtt_client.hpp>
#include <qrMqtt/qr_scanner.hpp>
#include <qrMqtt/token_validator.hpp>

namespace
{
std::atomic<bool> running(true);

void stop(int)
{
    running.store(false);
}

class MosquittoLibrary
{
  public:
    MosquittoLibrary()
    {
        const int rc = mosquitto_lib_init();
        if (rc != MOSQ_ERR_SUCCESS)
        {
            throw std::runtime_error(mosquitto_strerror(rc));
        }
    }

    ~MosquittoLibrary()
    {
        mosquitto_lib_cleanup();
    }
};

std::string configPath(int argc, char **argv)
{
    if (argc == 1)
    {
        return "config/qrMqtt.conf";
    }
    if (argc == 3 && std::string(argv[1]) == "--config")
    {
        return argv[2];
    }
    throw std::runtime_error("Usage: qrMqtt [--config PATH]");
}
}

int main(int argc, char **argv)
{
    try
    {
        std::signal(SIGINT, stop);
        std::signal(SIGTERM, stop);

        const qrmqtt::Config config = qrmqtt::Config::load(configPath(argc, argv));
        MosquittoLibrary mqtt_library;
        qrmqtt::MqttClient mqtt(config,
                                 qrmqtt::statusJson(config.device_id, "online"),
                                 qrmqtt::statusJson(config.device_id, "offline"));
        mqtt.start();

        qrmqtt::TokenValidator validator(config.security);
        qrmqtt::DuplicateFilter duplicate_filter(config.camera.duplicate_window_seconds);
        qrmqtt::QrScanner scanner(config.camera);
        mqtt.publish("status", qrmqtt::statusJson(config.device_id, "camera_ready"), true);

        while (running.load())
        {
            const std::string payload = scanner.next(running);
            if (payload.empty() || !duplicate_filter.accept(payload))
            {
                continue;
            }

            const qrmqtt::ValidationResult result = validator.validate(payload);
            mqtt.publish("events/scan", qrmqtt::scanEventJson(config.device_id, result));
            mqtt.publish("status", qrmqtt::statusJson(
                config.device_id, result.approved ? "last_scan_approved" : "last_scan_rejected"), true);
            std::cout << "QR event " << result.code_id << ": " << result.reason << '\n';
        }
        mqtt.publish("status", qrmqtt::statusJson(config.device_id, "stopping"), true);
    }
    catch (const std::exception &error)
    {
        std::cerr << "qrMqtt error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
