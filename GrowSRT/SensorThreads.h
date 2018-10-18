/*
 * SensorThreads.h
 *
 *  Created on: 17.10.2018
 *      Author: dpo
 */

#ifndef SENSORTHREADS_H_
#define SENSORTHREADS_H_
#include "DHT.h"
#include "C8y_MQTT.h"

#define debug false
#define MONITORPIN 12

class TempAndHumiditySensor{
public:
	DHT * _dht; // declare pointer to DHT
	C8y_MQTT * _C8YClient;
	TempAndHumiditySensor(uint8_t DHTPIN, uint8_t DHTTYPE, C8y_MQTT &C8YClient, int monitoringPin=MONITORPIN);
	void setup();
	void querySensor();
private:
	float _humidity;
	float _temperature;
	int _monitorPin;
	void flash();
};

class LightSensor{
public:
	LightSensor(int sensorPin, C8y_MQTT &C8YClient, int monitoringPin=MONITORPIN);
	void setup();
	void querySensor();
	C8y_MQTT * _C8YClient;
private:
	int _pin;
	int _monitorPin;
	void flash();
};

void flash();

#endif /* SENSORTHREADS_H_ */
