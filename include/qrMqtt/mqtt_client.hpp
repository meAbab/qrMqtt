#ifndef QRMQTT_MQTT_CLIENT_HPP
#define QRMQTT_MQTT_CLIENT_HPP

#include <atomic>
#include <deque>
#include <mutex>
#include <string>

#include <mosquitto.h>

#include <qrMqtt/config.hpp>

namespace qrmqtt
{
class MqttClient
{
  public:
    MqttClient(const Config &config, const std::string &online_payload,
               const std::string &offline_payload);
    ~MqttClient();
    MqttClient(const MqttClient &) = delete;
    MqttClient &operator=(const MqttClient &) = delete;

    void start();
    void publish(const std::string &suffix, const std::string &payload, bool retained = false);
    bool connected() const;

  private:
    struct Message
    {
        std::string topic;
        std::string payload;
        bool retained;
    };

    Config config_;
    std::string online_payload_;
    std::string offline_payload_;
    struct mosquitto *client_;
    std::atomic<bool> connected_;
    bool loop_started_;
    mutable std::mutex queue_mutex_;
    std::deque<Message> queued_;

    std::string topic(const std::string &suffix) const;
    void configure();
    void loadSpool();
    void storeSpool() const;
    void enqueue(const Message &message);
    bool publishNow(const Message &message);
    void flushQueue();
    static void onConnect(struct mosquitto *, void *context, int rc);
    static void onDisconnect(struct mosquitto *, void *context, int rc);
};
}

#endif
