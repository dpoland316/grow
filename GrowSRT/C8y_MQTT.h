/*
 * C8y_MQTT.h
 *
 *  Created on: 14.10.2018
 *      Author: dpo
 */

#ifndef C8Y_MQTT_H_
#define C8Y_MQTT_H_

#include "PubSubClient.h"
#include "growConfig.h"
#include "string.h"
#include "boost/tokenizer.hpp"

// C8Y Static MQTT Templates
#define TEMPLATE_REGISTER_CMD 	114
#define TEMPLATE_SEND_EVENT		400
#define TEMPLATE_CMD_EXECUTING 	501
#define TEMPLATE_CMD_FAILED	 	502
#define TEMPLATE_CMD_FINISHED 	503
#define TEMPLATE_EXEC_COMMAND	511
#define TEMPLATE_EXEC_CONFIG	513

// C8Y MQTT topics
#define SENDING_TEMPLATE_TOPIC 		"s/us"
#define RECEIVING_TEMPLATE_TOPIC 	"s/ds"
#define RECEIVING_ERROR_TOPIC 		"s/e"

#define MQTT_QoS 	1
#define MQTT_RETAIN 0

#define GROW_CALLBACK_SIGNATURE std::function<void(mc_color color)> cmdCallback

class C8y_MQTT{
private:
	PubSubClient* _client;
	GROW_CALLBACK_SIGNATURE;
	boolean registerSubscriptions();
	boolean registerSupportedOperations();
	void executeOperation(String cmd);
public:
	C8y_MQTT();
	C8y_MQTT(PubSubClient& client); // Establish the WiFi link
	boolean init(String deviceId);
	boolean publish(const char* payload);
	boolean publish(const char* topic, const char* payload);
	boolean publish(int c8yTemplate, String payload);
	boolean publish(int c8yTemplate, String c8yFragment, String payload);
	boolean callback(const char* topic, byte* payload, unsigned int length);

	boolean sendTemp(float temp);
	boolean sendHumidity(float hum);
	boolean sendLight(int lux);
	String getLastWillTopic();
	int getLastWillQoS();
	int getLastWillRetainFlag();
	String getLastWillMsg();

	C8y_MQTT& setClient(PubSubClient& client);

	C8y_MQTT& setCmdCallback(GROW_CALLBACK_SIGNATURE);
};


#endif /* C8Y_MQTT_H_ */
