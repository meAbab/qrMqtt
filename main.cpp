/**
* @file main.cpp
* call camera, call publisher to send
* msg to subscriber, waits for 5 sec, 
* and recall camera.
*/

#include <iostream>
#include <string>
#include <unistd.h>

#include "qrmosq.h"

using namespace std;

int main(void)
{
	try {

	class qrMqtt *qr2sp;
	//int rc;
	mosqpp::lib_init();

	qr2sp = new qrMqtt ("qr2sp", "pcktatDoor", "192.168.178.100", 1883);

	while (1){

		/// call camera [qrcam()], read QR and send to publish()
		qr2sp->send_msg(qr2sp->qrcam().c_str());

		rc = qr2sp->loop();

		if (rc){
			qr2sp->reconnect();
		}
/** 
* will wait 5 sec, so camera light will be turned off
* for 5 seconds. After 5 sec, camera will active again to read QR-code
*/
		usleep(5000000);	
	}
	mosqpp::lib_cleanup();
	} catch (const exception& e){					/// if exception occured in constructor. see class declaration.
		cerr << "Error on Network Connection.\n" \
					<< "Check mosquitto is running & IP/PORT\n";
	}
	return 0;
}

/*
Once cake sent to us, bought back to package-center and next 2 days, we were not able to pick it. 
However at the same day we were rounding near by package-center for few times.

While get back home, it was late and 2 days later we'd a cake with feet-smell.

Anyway, I guess a door-bell with QR-code reader that somehow 
connected with smart phone [mqtt i used here] would be helpful for package owner.

For future idea, a packet box can build infront of door,
and order id [that you made order in ebay, amazon, aliexpress etc], 
description may supplied to MQTT broker, and once QR-code scanned with 
supplied order-id and description, the packet box door will open.
The postman will put the packet and close it.

*/