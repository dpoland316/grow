/*
 * SensorThreads.cpp
 *
 *  Created on: 15.10.2018
 *      Author: dpo
 */
#include "SensorThreads.h"

// Call the Process constructor
TempAndHumidityProcess::TempAndHumidityProcess(Scheduler& manager,
		ProcPriority pr, unsigned int period, DHT& dht) :
		Process(manager, pr, period) {
	//setSensor(dht);
}

/*
TempAndHumidityProcess& TempAndHumidityProcess::setSensor(DHT& dht){
    this->_dht = &dht;
    return *this;
}
*/

void service()
{
	float _humidity = _dht->readHumidity();
	float _temperature = _dht->readTemperature();

	//Print temp and humidity values to serial monitor
	Serial.print("Humidity: ");
	Serial.print(_humidity);
	Serial.print(" %, Temp: ");
	Serial.print(_temperature);
	Serial.println(" Celsius");

	String tempMsgString = "c8y_TemperatureSensor,T," + String(_temperature, 2) + ",C";
	String humidityString = "c8y_HumiditySensor,h," + String(_humidity, 2) + ",%RH";

//	C8YClient.publish(200, tempMsgString); //flash();
//	C8YClient.publish(200, humidityString); //flash();
}


