#include <qrMqtt/mqtt_client.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <mqtt_protocol.h>
#include <openssl/ssl.h>

namespace qrmqtt
{
namespace
{
void requireSuccess(int rc, const std::string &operation)
{
    if (rc != MOSQ_ERR_SUCCESS)
    {
        throw std::runtime_error(operation + ": " + mosquitto_strerror(rc));
    }
}

const char *nullable(const std::string &value)
{
    return value.empty() ? nullptr : value.c_str();
}
}

MqttClient::MqttClient(const Config &config, const std::string &online_payload,
                       const std::string &offline_payload)
  : config_(config), online_payload_(online_payload), offline_payload_(offline_payload),
    client_(nullptr), connected_(false), loop_started_(false)
{
    client_ = mosquitto_new(config_.mqtt.client_id.c_str(), true, this);
    if (client_ == nullptr)
    {
        throw std::runtime_error("Unable to allocate the MQTT client.");
    }
    try
    {
        configure();
        loadSpool();
    }
    catch (...)
    {
        mosquitto_destroy(client_);
        throw;
    }
}

MqttClient::~MqttClient()
{
    if (client_ == nullptr)
    {
        return;
    }
    if (connected_.load())
    {
        publishNow({topic("availability"), offline_payload_, true});
    }
    mosquitto_disconnect_v5(client_, MQTT_RC_NORMAL_DISCONNECTION, nullptr);
    if (loop_started_)
    {
        mosquitto_loop_stop(client_, false);
    }
    mosquitto_destroy(client_);
}

void MqttClient::configure()
{
    requireSuccess(mosquitto_int_option(client_, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5),
                   "Unable to select MQTT v5");
    mosquitto_connect_callback_set(client_, onConnect);
    mosquitto_disconnect_callback_set(client_, onDisconnect);
    requireSuccess(mosquitto_reconnect_delay_set(client_, 1, 30, true),
                   "Unable to configure reconnection");

    if (!config_.mqtt.username.empty())
    {
        requireSuccess(mosquitto_username_pw_set(client_, config_.mqtt.username.c_str(),
                                                 nullable(config_.mqtt.password)),
                       "Unable to configure MQTT credentials");
    }
    if (config_.mqtt.tls_enabled)
    {
        requireSuccess(mosquitto_tls_set(client_, config_.mqtt.ca_file.c_str(), nullptr,
                                         nullable(config_.mqtt.cert_file),
                                         nullable(config_.mqtt.key_file), nullptr),
                       "Unable to configure TLS certificates");
        requireSuccess(mosquitto_tls_opts_set(client_, SSL_VERIFY_PEER, "tlsv1.2", nullptr),
                       "Unable to require TLS verification");
        requireSuccess(mosquitto_tls_insecure_set(client_, false),
                       "Unable to require broker hostname verification");
    }
    requireSuccess(mosquitto_will_set(client_, topic("availability").c_str(),
                                      static_cast<int>(offline_payload_.size()),
                                      offline_payload_.data(), config_.mqtt.qos, true),
                   "Unable to configure MQTT availability will");
}

void MqttClient::start()
{
    requireSuccess(mosquitto_connect_async(client_, config_.mqtt.host.c_str(),
                                           config_.mqtt.port, 60),
                   "Unable to begin MQTT connection");
    requireSuccess(mosquitto_loop_start(client_), "Unable to start MQTT network loop");
    loop_started_ = true;
}

bool MqttClient::connected() const
{
    return connected_.load();
}

std::string MqttClient::topic(const std::string &suffix) const
{
    return config_.mqtt.topic_prefix + "/" + suffix;
}

void MqttClient::publish(const std::string &suffix, const std::string &payload, bool retained)
{
    const Message message{topic(suffix), payload, retained};
    if (!connected_.load() || !publishNow(message))
    {
        enqueue(message);
    }
}

bool MqttClient::publishNow(const Message &message)
{
    mosquitto_property *properties = nullptr;
    mosquitto_property_add_string(&properties, MQTT_PROP_CONTENT_TYPE, "application/json");
    if (!message.retained && config_.mqtt.message_expiry_seconds > 0)
    {
        mosquitto_property_add_int32(&properties, MQTT_PROP_MESSAGE_EXPIRY_INTERVAL,
                                     config_.mqtt.message_expiry_seconds);
    }
    const int rc = mosquitto_publish_v5(client_, nullptr, message.topic.c_str(),
                                        static_cast<int>(message.payload.size()),
                                        message.payload.data(), config_.mqtt.qos,
                                        message.retained, properties);
    mosquitto_property_free_all(&properties);
    return rc == MOSQ_ERR_SUCCESS;
}

void MqttClient::enqueue(const Message &message)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queued_.push_back(message);
    storeSpool();
}

void MqttClient::loadSpool()
{
    std::ifstream input(config_.mqtt.spool_file);
    std::string line;
    while (std::getline(input, line))
    {
        const std::size_t first = line.find('\t');
        const std::size_t second = first == std::string::npos ? first : line.find('\t', first + 1);
        if (first != std::string::npos && second != std::string::npos)
        {
            queued_.push_back({line.substr(0, first), line.substr(second + 1),
                               line.substr(first + 1, second - first - 1) == "1"});
        }
    }
}

void MqttClient::storeSpool() const
{
    std::ofstream output(config_.mqtt.spool_file, std::ios::trunc);
    if (!output)
    {
        std::cerr << "Unable to write MQTT outbox: " << config_.mqtt.spool_file << '\n';
        return;
    }
    for (const Message &message : queued_)
    {
        output << message.topic << '\t' << (message.retained ? "1" : "0") << '\t'
               << message.payload << '\n';
    }
}

void MqttClient::flushQueue()
{
    std::deque<Message> queued;
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        queued.swap(queued_);
        storeSpool();
    }
    for (const Message &message : queued)
    {
        if (!publishNow(message))
        {
            enqueue(message);
        }
    }
}

void MqttClient::onConnect(struct mosquitto *, void *context, int rc)
{
    MqttClient *self = static_cast<MqttClient *>(context);
    if (rc != 0)
    {
        std::cerr << "MQTT broker rejected connection: " << rc << '\n';
        return;
    }
    self->connected_.store(true);
    self->publishNow({self->topic("availability"), self->online_payload_, true});
    self->flushQueue();
    std::cout << "MQTT connected to " << self->config_.mqtt.host << '\n';
}

void MqttClient::onDisconnect(struct mosquitto *, void *context, int rc)
{
    MqttClient *self = static_cast<MqttClient *>(context);
    self->connected_.store(false);
    if (rc != 0)
    {
        std::cerr << "MQTT connection lost; events will be queued for retry.\n";
    }
}
}
