/*
 * C8y_MQTT.h
 *
 *  Created on: 14.10.2018
 *      Author: dpo
 */

#ifndef C8Y_MQTT_H_
#define C8Y_MQTT_H_
#include "PubSubClient.h"
#include "string.h"


// MQTT credentials
//const char* mqttServer = "m20.cloudmqtt.com";
//const int 	mqttPort = 14273;
//const char* mqttUser = "david.poland@softwareag.com";
//const char* mqttPassword = "2wsx3edc!";
//techUser/techUser123!

class C8y_MQTT{
public:
	C8y_MQTT();
	C8y_MQTT(PubSubClient& client); // Establish the WiFi link
	boolean init(String deviceId);
	boolean publish(const char* payload);
	boolean publish(const char* topic, const char* payload);
	boolean publish(int c8yTemplate, String payload);
	boolean callback(const char* topic, byte* payload, unsigned int length);

	boolean sendTemp(float temp);
	boolean sendHumidity(float hum);
	boolean sendLight(int lux);

	C8y_MQTT& setClient(PubSubClient& client);
private:
	PubSubClient* _client;
	boolean registerSubscriptions();
};


#endif /* C8Y_MQTT_H_ */
