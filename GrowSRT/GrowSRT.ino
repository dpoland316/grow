#include "Arduino.h"
#include "string.h"
#include "DHT.h"
#include "ESP8266WiFi.h"
#include "time.h"
#include "PubSubClient.h"
#include "C8y_MQTT.h"
#include "FS.h"
#include "Sensors.h"
#include "TaskScheduler.h"


/***************************************************
 *
 * Login credentials and pin/server configuration
 *
 ***************************************************/
#include "logins.h"
#include "growConfig.h"


// Wifi / MQTT / Cumulocity Clients
WiFiClient espClient;
PubSubClient client(espClient);
C8y_MQTT C8YClient(client);


void signalLEDCallback(mc_color color){
Serial.print("Callback executed, color: "); Serial.println(color);
	switch (color) {
		case red:
			digitalWrite(MC_RED, HIGH);
			digitalWrite(MC_BLUE, LOW);
			digitalWrite(MC_GREEN, LOW);
			break;
		case green:
			digitalWrite(MC_RED, LOW);
			digitalWrite(MC_BLUE, LOW);
			digitalWrite(MC_GREEN, HIGH);
			break;
		case blue:
			digitalWrite(MC_RED, LOW);
			digitalWrite(MC_BLUE, HIGH);
			digitalWrite(MC_GREEN, LOW);
			break;
		case yellow:
			digitalWrite(MC_RED, HIGH);
			digitalWrite(MC_BLUE, LOW);
			digitalWrite(MC_GREEN, HIGH);
			break;
		case purple:
			digitalWrite(MC_RED, HIGH);
			digitalWrite(MC_BLUE, HIGH);
			digitalWrite(MC_GREEN, LOW);
			break;
		case aqua:
			digitalWrite(MC_RED, LOW);
			digitalWrite(MC_BLUE, HIGH);
			digitalWrite(MC_GREEN, HIGH);
			break;
	}
}

void initFS(){
	if (!SPIFFS.exists("/format.lck")) { // fs not formatlck already
		Serial.print("Initializing file system...");
		SPIFFS.format();
		File formatlck = SPIFFS.open("/format.lck", "w");
		formatlck.print("This file is auto generated. Removing this file will reset the device.");
		formatlck.close();
		Serial.println("done!");
	} else {
		Serial.println("File system already initialized...");
	}
}

void initPins(){
	pinMode(BLUE, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(YELLOW,OUTPUT);
	pinMode(MC_RED,OUTPUT);
	pinMode(MC_BLUE,OUTPUT);
	pinMode(MC_RED,OUTPUT);

	digitalWrite(BLUE, LOW);
	digitalWrite(GREEN, LOW);
	digitalWrite(YELLOW, LOW);
	digitalWrite(MC_RED,LOW);
	digitalWrite(MC_BLUE,LOW);
	digitalWrite(MC_RED,LOW);
}

void initWiFi(){
	// Connect to WiFi
	WiFi.begin(ssid, password);
	Serial.print("Connecting to WiFi..");

	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
	delay(500);
	}

	digitalWrite(BLUE, HIGH);
	Serial.println();
	Serial.println("Connected to the WiFi network");

//	//set date time
//	configTime(0, 0, "pool.ntp.org", "time.nist.gov");
//	Serial.print("\nWaiting for time");
//	while (!time(nullptr)) {
//	  Serial.print(".");
//	  delay(1000);
//	}
//	Serial.print("\r\nTime has been acquired from internet time service\r\nCurrent GMT: ");
//
//	time_t now = time(nullptr);
//	Serial.println(ctime(&now));


}

void initMQTT(){
	// Configure MQTT server/client and callback
	client.setServer(mqttServer.c_str(), mqttPort);
	client.setCallback(mqttCallback);

	//Establish connection to MQTT Server
	while (!client.connected()) {
	    Serial.println("Connecting to MQTT...");

// 	Problem with the below - it core dumps the chip upon receiving an event

//	    if (client.connect(growDeviceID.c_str(),
//	    					mqttUser,
//							mqttPassword,
//							C8YClient.getLastWillTopic().c_str(),
//							C8YClient.getLastWillQoS(),
//							C8YClient.getLastWillRetainFlag(),
//							C8YClient.getLastWillMsg().c_str()
//		)){
	    if (client.connect(growDeviceID.c_str(), mqttUser, mqttPassword )) {
	      Serial.println("Connected to " + mqttServer);
	      digitalWrite(GREEN, HIGH);

	      C8YClient.setCmdCallback(&signalLEDCallback);

	    } else {
	      Serial.print("failed with state ");
	      Serial.println(client.state());
	      delay(2000);
	    }
	}

	// Subscribe to C8Y topics
    if (C8YClient.init(growDeviceID)) {
      Serial.println("Subscribed to C8Y topics...");
    } else {
      Serial.print("Failed to subscribe to C8Y topics...");
      Serial.print(client.state());
      delay(2000);
    }


}


void flash(){
	// flash the pin to show activity
	digitalWrite(MONITORPIN, LOW);
	digitalWrite(MONITORPIN, HIGH);
	delay(100);
	digitalWrite(MONITORPIN, LOW);
}


/************************************************************
 *
 *  Define Schedulers, Tasks and task callbacks, so that
 *  sensor readings can be gathered and sent asynchronously
 *
 ************************************************************/

Scheduler scheud;


void tempAndHumidityCallback() 	{
	float temp, hum;
	std::tie(temp, hum) = TempAndHumiditySensor::querySensor();

	C8YClient.sendTemp(temp);
	flash(); 	// sent temp

	C8YClient.sendHumidity(hum);
	flash(); 	//sent humidity
}
void lightCallback() 	{
	C8YClient.sendLight(LightSensor::querySensor());
	flash();	// sent lux
}
void mqttCallback(const char* topic, byte* payload, unsigned int length){
	C8YClient.callback(topic, payload, length);
}

// Create task definitions and specify task interval
Task TempAndHumidityTask(2000, TASK_FOREVER, &tempAndHumidityCallback);
Task LightTask(1000, TASK_FOREVER, &lightCallback);


void setup()
{
	Serial.begin(115200);
	Serial.println();

	// Set output mode
	initPins();

	// Start the SPI Flash Files System
	SPIFFS.begin();
	initFS();

	// Connect to WiFi
	initWiFi();

	// Connect to MQTT
	initMQTT();

	// Prepare sensors
	TempAndHumiditySensor::sensorSetup();
	LightSensor::sensorSetup();

	// Initialize scheduler and sensor tasks
	scheud.init();
	scheud.addTask(TempAndHumidityTask);
	scheud.addTask(LightTask);
	TempAndHumidityTask.enable();
	LightTask.enable();

}


void loop()
{
	client.loop(); 		// MQTT monitor
	scheud.execute();	// Sensor loop


	/*****************************************************************
	 *
	 * Keep indicator pins updated with current connection status
	 * WiFi will automatically reconnect, MQTT needs to be manually
	 * retriggered.
	 *
	 *****************************************************************/
	if (!WiFi.isConnected()){
		digitalWrite(WIFIPIN, LOW);
	} else {
		digitalWrite(WIFIPIN, HIGH);
	}

	if (!client.connected()){
		digitalWrite(MQTTPIN, LOW);
		if (WiFi.isConnected()){
			initMQTT();
		}
	} else {
		digitalWrite(MQTTPIN, HIGH);
	}
}




//void testFS(){
//	  // open file for writing
//	  File f = SPIFFS.open("/f.txt", "w");
//	  if (!f) {
//	      Serial.println("file open failed");
//	  }
//	  Serial.println("====== Writing to SPIFFS file =========");
//	  // write 10 strings to file
//	  for (int i=1; i<=10; i++){
//	    f.print("Millis() : ");
//	    f.println(millis());
//	    Serial.println(millis());
//	    delay(20);
//	  }
//
//	  f.close();
//
//	  // open file for reading
//	  File g = SPIFFS.open("/f.txt", "r");
//	  if (!g) {
//	      Serial.println("file open failed");
//	  }  Serial.println("====== Reading from SPIFFS file =======");
//	  // write 10 strings to file
//	  for (int i=1; i<=10; i++){
//	    String s=g.readStringUntil('\n');
//	    Serial.print(i);
//	    Serial.print(":");
//	    Serial.println(s);
//	  }
//
//	  testTheFS=false;
//
//}


