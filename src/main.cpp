/**
* @file main.cpp
* call camera, call publisher to send
* msg to subscriber, waits for 5 sec,
* and recall camera.
*/

#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

#include <qrMqtt/qrmosq.h>

using namespace std;

namespace
{
class MosquittoLibrary
{
  public:
    MosquittoLibrary()
    {
        int rc = mosquitto_lib_init();
        if (rc != MOSQ_ERR_SUCCESS)
        {
            throw runtime_error(mosquitto_strerror(rc));
        }
    }

    ~MosquittoLibrary()
    {
        mosquitto_lib_cleanup();
    }

    MosquittoLibrary(const MosquittoLibrary &) = delete;
    MosquittoLibrary &operator=(const MosquittoLibrary &) = delete;
};
}

int main(void)
{
    try
    {
        MosquittoLibrary mqtt;
        qrMqtt qr2sp("qr2sp", "pcktatDoor", "192.168.178.100", 1883);

        while (true)
        {
            /// call camera [qrcam()], read QR and send to publish()
            qr2sp.send_msg(qr2sp.qrcam().c_str());

            /**
            * will wait 5 sec, so camera light will be turned off
            * for 5 seconds. After 5 sec, camera will active again to read QR-code
            */
            usleep(5000000);
        }
    }
    catch (const exception &e)
    { /// if exception occurred in constructor. see class declaration.
        cerr << "Error on Network Connection: " << e.what() << "\n"
             << "Check mosquitto is running & IP/PORT\n";
        return 1;
    }
    return 0;
}

/*
Once a cake sent to us, bought back to the package-center and next 2 days,
we were not able to pick it.
However at the same day we were rounding near by package-center for few times.

While getting back to home, it was late and 2 days later we'd a cake with smell like feet.

Anyway, I guess a door-bell with QR-code reader that somehow
connected with a smart phone [mqtt i used here] would be helpful for package owner.

For future idea, a packet box can be build infront of the door,
and order id [that you made order in ebay, amazon, aliexpress etc],
description may supplied to MQTT broker. Once QR-code scanned with
the supplied order-id and description, the packet box door should open.
The postman just keep the packet inside the box and close it.
*/
