/*
 * SensorThreads.cpp
 *
 *  Created on: 17.10.2018
 *      Author: dpo
 */
#include "SensorThreads.h"

void TempAndHumiditySensor::setup() {
	_dht->begin();				// execute objects on referenced object
}

TempAndHumiditySensor::TempAndHumiditySensor(uint8_t DHTPIN, uint8_t DHTTYPE, C8y_MQTT &C8YClient , int monitorPin) : _monitorPin(monitorPin) { // member initializer
	DHT dht(DHTPIN, DHTTYPE); 	// instantiate the object
	_dht = &dht; 				// assign the pointer to the value of the instantiated object
	_C8YClient = &C8YClient;
}

void TempAndHumiditySensor::querySensor() {
	Serial.println("executed querySensor service!");
	_humidity = 	_dht->readHumidity();
	_temperature = 	_dht->readTemperature();

	if (debug) { //  Print temp and humidity values to serial monitor
		Serial.print("Humidity: "); Serial.print(_humidity); Serial.print(" %, Temp: ");
		Serial.print(_temperature);	Serial.println(" Celsius");
	}

	String tempMsgString = "c8y_TemperatureSensor,T," + String(_temperature, 2) + ",C";
	String humidityString = "c8y_HumiditySensor,h," + String(_humidity, 2) + ",%RH";
	//
			_C8YClient->publish(200, tempMsgString); flash();
			_C8YClient->publish(200, humidityString); flash();
}
void TempAndHumiditySensor::flash(){
	// flash the pin to show activity
	digitalWrite(_monitorPin, LOW);
	digitalWrite(_monitorPin, HIGH);
	delay(100);
	digitalWrite(_monitorPin, LOW);
}



LightSensor::LightSensor(int sensorPin, C8y_MQTT &C8YClient, int monitorPin) : _pin (sensorPin), _monitorPin(monitorPin) { // member initializer
	_C8YClient = &C8YClient;
}

void LightSensor::setup() {
	pinMode(_pin, INPUT);
}


void LightSensor::querySensor() {
	int sensorValue = analogRead(_pin);   // read the input on analog pin

	// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
	float voltage = sensorValue * (3.3 / 1023.0);
	float percentage = (sensorValue / 1023.0) * 100.0;
	int lux = percentage + 1000;

	if (debug) { //  Print sensor values to serial monitor
		Serial.print("Light Measurement: "); Serial.print(sensorValue); Serial.print(", ");
		Serial.print(voltage); Serial.print(" volts"); Serial.print(", "); Serial.print(percentage);
		Serial.print("%");
	}

	String lightString = "c8y_LightSensor,e," + String(lux) + ",lux";
	_C8YClient->publish(200, lightString);
	flash();
}

void LightSensor::flash(){
	// flash the pin to show activity
	digitalWrite(_monitorPin, LOW);
	digitalWrite(_monitorPin, HIGH);
	delay(100);
	digitalWrite(_monitorPin, LOW);
}
