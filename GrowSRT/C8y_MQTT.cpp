/*
 * C8y_MQTT.cpp
 *
 *  Created on: 14.10.2018
 *      Author: dpo
 */
#include "C8y_MQTT.h"

char* sendingTemplateTopic = "s/us";
char* receivingTemplateTopic = "s/ds";
char* receivingErrorTopic = "s/e";



C8y_MQTT::C8y_MQTT() {

}

C8y_MQTT::C8y_MQTT(PubSubClient& client) {
	setClient(client);
}

C8y_MQTT& C8y_MQTT::setClient(PubSubClient& client){
    this->_client = &client;
    return *this;
}
boolean C8y_MQTT::publish(const char* payload){
	return publish(sendingTemplateTopic, payload);
}
boolean C8y_MQTT::publish(int c8yTemplate, String payload){
	String payloadMsg = String(c8yTemplate) + "," + payload;
	return publish(sendingTemplateTopic, payloadMsg.c_str());
}

boolean C8y_MQTT::publish(const char* topic, const char* payload) {
	Serial.print("Sending ");
	Serial.print(payload);
	Serial.print("to C8Y ");
	Serial.println(topic);

	return _client->publish(sendingTemplateTopic, payload);
}

boolean C8y_MQTT::init(String deviceId) {

	boolean dcSent = false;
	boolean dhSent = false;
	boolean subscribed = false;

	String deviceCreationMsg = "100,Grow_" + deviceId + ",c8y_MQTTdevice";
	String deviceHardwareMsg = "110,1234554321,Grow Module v0.5,0.0.1";
	dcSent = publish(deviceCreationMsg.c_str());
	dhSent = publish(deviceHardwareMsg.c_str());
	subscribed=registerSubscriptions();

	Serial.println("C8y Device creation sent: " + deviceCreationMsg + "..." + (dcSent ? "true" : "false"));
	Serial.println("C8y Device hardware sent: " + deviceHardwareMsg + "..." + (dhSent ? "true" : "false"));

	if (dcSent && dhSent && subscribed) {
		return true;
	} else {
		return false;
	}

}

boolean C8y_MQTT::registerSubscriptions(){
	boolean rTopic = false;
	boolean eTopic = false;

	rTopic = _client->subscribe(receivingTemplateTopic);
	eTopic = _client->subscribe(receivingErrorTopic);

	Serial.print("Subscribed to C8Y Topic ");
	Serial.print(receivingTemplateTopic);
	Serial.print("...");
	Serial.println(rTopic ? "true" : "false");
	Serial.print("Subscribed to C8Y Error Topic ");
	Serial.print(receivingErrorTopic);
	Serial.print("...");
	Serial.println(eTopic ? "true" : "false");

	if (rTopic && eTopic){
		return true;
	} else {
		return false;
	}

}

boolean C8y_MQTT::callback(const char* topic, byte* payload,
		unsigned int length) {
	/* MQTT callback service
	 * Placed before setup loop so it can be registered at initialization time.
	 */

	Serial.print("C8Y MQTT Message arrived in topic: ");
	Serial.println(topic);

	Serial.print("Message:");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}

	Serial.println();
	Serial.println("-----------------------");
	return true;
}
