/*
 * C8y_MQTT.cpp
 *
 *  Created on: 14.10.2018
 *      Author: dpo
 */
#include "C8y_MQTT.h"

C8y_MQTT::C8y_MQTT() {
}

C8y_MQTT::C8y_MQTT(PubSubClient& client) {
	setClient(client);
	setCmdCallback(NULL);
}

C8y_MQTT& C8y_MQTT::setClient(PubSubClient& client){
    this->_client = &client;
    return *this;
}
boolean C8y_MQTT::publish(const char* payload){
	return publish(SENDING_TEMPLATE_TOPIC, payload);
}
boolean C8y_MQTT::publish(int c8yTemplate, String payload){
	String payloadMsg = String(c8yTemplate) + "," + payload;
	return publish(SENDING_TEMPLATE_TOPIC, payloadMsg.c_str());
}
boolean C8y_MQTT::publish(int c8yTemplate, String c8yFragment, String payload) {
	String payloadMsg = String(c8yTemplate) + "," + c8yFragment + "," + payload;
	return publish(SENDING_TEMPLATE_TOPIC, payloadMsg.c_str());
}

boolean C8y_MQTT::publish(const char* topic, const char* payload) {
	if (c8yDebug){
		Serial.print("Sending ");
		Serial.print(payload);
		Serial.print(" to C8Y ");
		Serial.println(topic);
	}

	return _client->publish(SENDING_TEMPLATE_TOPIC, payload);
}

boolean C8y_MQTT::init(String deviceId) {

	boolean dcSent = false;
	boolean dhSent = false;
	boolean ceSent = false;
	boolean subscribed = false;

	String deviceCreationMsg = "100,Grow_" + deviceId + ",c8y_MQTTdevice";
	String deviceHardwareMsg = "110," + growModuleSerialNumber + "," + growModuleName + "," + growModuleRevision;
	dcSent = publish(deviceCreationMsg.c_str());
	dhSent = publish(deviceHardwareMsg.c_str());
	ceSent = publish(TEMPLATE_SEND_EVENT, "c8y_ConnectionEvent", "Device connected.");
	subscribed=registerSubscriptions();
	registerSupportedOperations();

	if (c8yDebug) {
		Serial.println("C8y Device creation sent: " + deviceCreationMsg + "..." + (dcSent ? "true" : "false"));
		Serial.println("C8y Device hardware sent: " + deviceHardwareMsg + "..." + (dhSent ? "true" : "false"));
		Serial.println("Connection Event sent: ..." + String(ceSent ? "true" : "false"));
	}

	if (dcSent && dhSent && subscribed) {
		return true;
	} else {
		return false;
	}

}

boolean C8y_MQTT::sendTemp(float temp) {
	String tempMsgString = "c8y_TemperatureSensor,T," + String(temp, 2) + ",C";
	publish(200, tempMsgString);
}

boolean C8y_MQTT::sendHumidity(float hum) {
	String humidityString = "c8y_HumiditySensor,h," + String(hum, 2) + ",%RH";
	publish(200, humidityString);
}

boolean C8y_MQTT::sendLight(int lux) {
	String lightString = "c8y_LightSensor,e," + String(lux) + ",lux";
	publish(200, lightString);
}

boolean C8y_MQTT::registerSubscriptions(){
	boolean rTopic = false;
	boolean eTopic = false;

	rTopic = _client->subscribe(RECEIVING_TEMPLATE_TOPIC);
	eTopic = _client->subscribe(RECEIVING_ERROR_TOPIC);

	if (c8yDebug){
		Serial.print("Subscribed to C8Y Topic ");
		Serial.print(RECEIVING_TEMPLATE_TOPIC);
		Serial.print("...");
		Serial.println(rTopic ? "true" : "false");
		Serial.print("Subscribed to C8Y Error Topic ");
		Serial.print(RECEIVING_ERROR_TOPIC);
		Serial.print("...");
		Serial.println(eTopic ? "true" : "false");
	}

	if (rTopic && eTopic){
		return true;
	} else {
		return false;
	}

}

boolean C8y_MQTT::callback(const char* topic, byte* payload,
		unsigned int length) {

	Serial.print("C8Y MQTT Message arrived in topic: "); Serial.println(topic);

	String msg;
	for (int i = 0; i < length; i++) {
		msg += ((char)payload[i]);
	}
	Serial.println("Message:" + msg);


	// Tokenize string and capture command elements
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	boost::char_separator<char> sep{","};
	tokenizer tok{msg, sep};

	std::vector<String> cmdElements;

	for (const auto &t : tok) {
		cmdElements.push_back(t.c_str());;
	}

	int c8yTemplate 		= cmdElements[0].toInt();
	String deviceIdentifer 	= cmdElements[1];
	String cmdToExecute		= cmdElements[2];

	if (deviceIdentifer == growDeviceID){
//		switch (c8yTemplate)
//		{
//			case TEMPLATE_EXEC_COMMAND:
//				executeOperation(cmdToExecute);
//				break;
//			case 2:
//				//
//			case default:
//				publish(TEMPLATE_CMD_FAILED, "Unsupported Operation.");
//		}
		if (c8yTemplate == TEMPLATE_EXEC_COMMAND) {
			executeOperation(cmdToExecute);
		} else {
			publish(TEMPLATE_CMD_FAILED, "Unsupported Operation.");
		}
	}

	return true;

}

boolean C8y_MQTT::registerSupportedOperations(){

	String operations = "c8y_Command,c8y_Configuration;c8y_Restart";
	publish(TEMPLATE_REGISTER_CMD, operations);

	return true;
}

C8y_MQTT& C8y_MQTT::setCmdCallback(GROW_CALLBACK_SIGNATURE) {
		this->cmdCallback = cmdCallback;
		return *this;
}

void C8y_MQTT::executeOperation(String cmd){
	String param="c8y_Command";
	publish(TEMPLATE_CMD_EXECUTING, param);
	Serial.print("Executing C8Y Operation: " + cmd);


	// Tokenize string and capture command elements
	typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
	boost::char_separator<char> sep{" "};
	tokenizer tok{cmd, sep};

	std::vector<String> cmdElements;
	std::vector<String> parameters;


	/***********************************************
	 *
	 *  Expected commands will follow the
	 *  <action> <target> <parameters> template.
	 *  i.e. <SET> <LED3> <OFF>
	 *
	 ***********************************************/

	for (const auto &t : tok) {
		cmdElements.push_back(t.c_str());;
	}

	switch (cmdElements.size()) {
		case 1: // single command
			cmdElements.push_back("<none>");
		case 2: // action on an object
			cmdElements.push_back("<none>");
		default: // action with multiple parameters
			Serial.println(" have enough parameters");
	}

	for (int i = 2; i < cmdElements.size(); i++){
		parameters.push_back(cmdElements[i]);
	}

	String action 			= cmdElements[0]; // i.e. SET
	String target 			= cmdElements[1]; // i.e. LED

	action.toUpperCase();
	target.toUpperCase();

	if (action.equals("SET")){
		if (target.equals("LED")){
			if (parameters[0]){
				String strColor = (parameters[0]).c_str();
				strColor.toLowerCase();
				Serial.println("Set LED Color: " + strColor);
				mc_color color = colorMap.find(strColor)->second;
				cmdCallback(color);
			}
		}
	} else if (action.equals("RESET")){
		Serial.println("executing reset");
		ESP.reset();
	} else if (action.equals("RESTART")){
		Serial.println("executing restart");
		ESP.restart();
	}

	String statusMsg = "The " + action + " command will be performed on the " + target + " with the parameters of";
	for (int i=0; i <= parameters.size(); i++) {
		statusMsg += " [" + String(i) + "] :" + parameters[i];
	}

	Serial.println(statusMsg);
	Serial.println("...done!!");
	publish(TEMPLATE_CMD_FINISHED, (param + "," + statusMsg));
}

String C8y_MQTT::getLastWillTopic() {
	return SENDING_TEMPLATE_TOPIC;
}

int C8y_MQTT::getLastWillQoS() {
	return MQTT_QoS;
}

int C8y_MQTT::getLastWillRetainFlag() {
	return MQTT_RETAIN;
}

String C8y_MQTT::getLastWillMsg() {
	return "400,c8y_ConnectionEvent,Device connection was lost.";
}

