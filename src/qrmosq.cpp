/** @file qrmosq.cpp
* description of qrmosq.h
* and send msg to subscriber
*/

#include <iostream>
#include <cstring>
#include <stdexcept>

#include <qrMqtt/qrmosq.h>

using namespace std;

qrMqtt::qrMqtt(const char *_id, const char *_topic, const char *_host, int _port)
  : client(nullptr), host(_host), topic(_topic), port(_port), keepalive(60)
{
  client = mosquitto_new(_id, true, this);
  if (client == nullptr)
  {
    throw runtime_error("Unable to create MQTT client.");
  }

  mosquitto_connect_callback_set(client, connect_callback);
  mosquitto_disconnect_callback_set(client, disconnect_callback);
  mosquitto_publish_callback_set(client, publish_callback);
  mosquitto_reconnect_delay_set(client, 1, 30, true);

  int rc = mosquitto_connect(client, host.c_str(), port, keepalive);
  if (rc != MOSQ_ERR_SUCCESS)
  {
    mosquitto_destroy(client);
    client = nullptr;
    throw runtime_error(mosquitto_strerror(rc));
  }

  rc = mosquitto_loop_start(client);
  if (rc != MOSQ_ERR_SUCCESS)
  {
    mosquitto_disconnect(client);
    mosquitto_destroy(client);
    client = nullptr;
    throw runtime_error(mosquitto_strerror(rc));
  }
}

qrMqtt::~qrMqtt()
{
  mosquitto_disconnect(client);
  mosquitto_loop_stop(client, false);
  mosquitto_destroy(client);
}

void qrMqtt::connect_callback(struct mosquitto *, void *context, int rc)
{
  static_cast<qrMqtt *>(context)->on_connect(rc);
}

void qrMqtt::disconnect_callback(struct mosquitto *, void *context, int rc)
{
  static_cast<qrMqtt *>(context)->on_disconnect(rc);
}

void qrMqtt::publish_callback(struct mosquitto *, void *context, int mid)
{
  static_cast<qrMqtt *>(context)->on_publish(mid);
}

void qrMqtt::on_connect(int rc)
{
  if (rc == 0)
  {
    cout << " ##-Connected with Broker-## " << std::endl;
  }
  else
  {
    cout << "##-Unable to Connect Broker-## " << std::endl;
  }
}

bool qrMqtt::send_msg(const char *message)
{
  int ret = mosquitto_publish(client, nullptr, topic.c_str(),
                              static_cast<int>(strlen(message)), message, 2, false);

  return (ret == MOSQ_ERR_SUCCESS);
}

void qrMqtt::on_disconnect(int rc)
{
  (void)rc;
  cout << " ##-Disconnected from Broker-## " << std::endl;
}

void qrMqtt::on_publish(int mid)
{
  (void)mid;
  cout << "## - Message published successfully" << endl;
}
