/*
 * growConfig.h
 *
 *  Created on: 18.10.2018
 *      Author: dpo
 */

#ifndef GROWCONFIG_H_
#define GROWCONFIG_H_

#include "boost/assign/list_of.hpp"
#include "map"

const String growDeviceID = "1A2S3DFCX5433";
const String growModuleName = "Grow Module v0.5,0.0.2";
const String growModuleSerialNumber = "1234554321";
const String growModuleRevision = "0.0.1";

// Define LED pins
#define YELLOW 		12
#define BLUE 		5
#define GREEN 		14
#define MC_RED 		10 //15
#define MC_GREEN	4
#define MC_BLUE		15 //10

enum mc_color {red, green, blue, yellow, purple, aqua};

const std::map<String, mc_color> colorMap = boost::assign::map_list_of	("red", red)
																	("green", green)
																	("blue", blue)
																	("yellow", yellow)
																	("purple", purple)
																	("aqua", aqua);

// Define Sensor Pins
#define LDR 	0 		// A0 Light Sensor (Light Dependent Resistor)
#define DHTPIN 	13		// Temp and Humidity
#define DHTTYPE	DHT22
#define SOIL	N/A

// Define indicators
const int MONITORPIN	=YELLOW;
const int WIFIPIN		=BLUE;
const int MQTTPIN		=GREEN;

const boolean sensorDebug 	= false;
const boolean c8yDebug 		= false;

#endif /* GROWCONFIG_H_ */
