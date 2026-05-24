#ifndef QRMOSQ_H
#define QRMOSQ_H

#include <mosquitto.h>
#include <string>

class qrMqtt
{
  private:
   struct mosquitto *client;
   std::string host;
   std::string topic;
   int port;
   int keepalive;

   static void connect_callback(struct mosquitto *, void *context, int rc);
   static void disconnect_callback(struct mosquitto *, void *context, int rc);
   static void publish_callback(struct mosquitto *, void *context, int mid);
   void on_connect(int rc);
   void on_disconnect(int rc);
   void on_publish(int mid);

  public:
   qrMqtt(const char *id, const char * _topic, const char *host, int port);
   ~qrMqtt();
   qrMqtt(const qrMqtt &) = delete;
   qrMqtt &operator=(const qrMqtt &) = delete;
   bool send_msg(const char *message);
   std::string qrcam();

};

#endif
