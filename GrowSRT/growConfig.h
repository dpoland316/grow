/*
 * growConfig.h
 *
 *  Created on: 18.10.2018
 *      Author: dpo
 */

#ifndef GROWCONFIG_H_
#define GROWCONFIG_H_


const String mqttDeviceID = "1A2S3DFCX5432";

// Define LED pins
#define YELLOW 	12
#define BLUE 	5
#define GREEN 	14

// Define Sensor Pins
#define LDR 	0 		// A0 Light Sensor (Light Dependent Resistor)
#define DHTPIN 	13		// Temp and Humidity
#define DHTTYPE	DHT22
#define SOIL	N/A

// Define indicators
const int MONITORPIN	=YELLOW;
const int WIFIPIN		=BLUE;
const int MQTTPIN		=GREEN;

#endif /* GROWCONFIG_H_ */
