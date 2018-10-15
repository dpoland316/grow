#include "Arduino.h"
#include "string.h"
#include "ProcessScheduler.h"
#include "DHT.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "C8y_MQTT.h"
#include "FS.h"

// 	Login Cerds
// -------------------
#include "logins.h"
// --------------------


const String mqttDeviceID = "1A2S3DFCX5432";

const char* temp_Q = "grow/Temperature";
const char* humidity_Q = "grow/Humidity";
const char* light_Q = "grow/Light";


boolean testTheFS=true;


// Wifi/MQTT/Cumulocity Clients
WiFiClient espClient;
PubSubClient client(espClient);
C8y_MQTT C8YClient(client);

// Define pins
#define YELLOW 	12
#define BLUE 	5
#define GREEN 	14
#define LDR 	0 		// A0 Light Dependant Resistor
#define DHTPIN 	13		// Temp and Humidity
#define DHTTYPE	DHT22

// Define DHT22 Sensor
DHT dht(DHTPIN, DHTTYPE);

// Define Async Processes
class BlinkProcess: public Process {
public:
	// Call the Process constructor
	BlinkProcess(Scheduler &manager, ProcPriority pr, unsigned int period,
			int pin) :
			Process(manager, pr, period) {
		_pinState = LOW; // Set the default state
		_pin = pin; // Store the pin number
	}

protected:
	//setup the pins
	virtual void setup() {
		pinMode(_pin, OUTPUT);
		_pinState = LOW;
		digitalWrite(_pin, _pinState);
	}

	//LEDs should be off when disabled
	virtual void onDisable() {
		_pinState = LOW;
		digitalWrite(_pin, _pinState);
	}

	//Start the LEDs on
	virtual void onEnable() {
		_pinState = HIGH;
		digitalWrite(_pin, _pinState);
	}

	// Create our service routine
	virtual void service() {
		// If pin is on turn it off, otherwise turn it on
		_pinState = !_pinState;
		digitalWrite(_pin, _pinState);
	}

private:
	bool _pinState; //the Current state of the pin
	int _pin; // The pin the LED is on
};
class TempAndHumidityProcess: public Process {
public:
	// Call the Process constructor
	TempAndHumidityProcess(Scheduler &manager, ProcPriority pr,
			unsigned int period) :
			Process(manager, pr, period) {
	}

protected:
	virtual void service() {
		//delay(2000);

		humidity = dht.readHumidity();
		temperature = dht.readTemperature();

		//Print temp and humidity values to serial monitor
		Serial.print("Humidity: ");
		Serial.print(humidity);
		Serial.print(" %, Temp: ");
		Serial.print(temperature);
		Serial.println(" Celsius");

		String tempMsgString = "c8y_TemperatureSensor,T," + String(temperature, 2) + ",C";
		String humidityString = "c8y_HumiditySensor,h," + String(humidity, 2) + ",%RH";

		C8YClient.publish(200, tempMsgString); flash();
		C8YClient.publish(200, humidityString); flash();
	}

private:
	float humidity;
	float temperature;
};
class LightProcess: public Process {
public:
	// Call the Process constructor
	LightProcess(Scheduler &manager, ProcPriority pr,
			unsigned int period, int pin) :
			Process(manager, pr, period) {
		_pin = pin; // Store the pin number
	}

protected:
	//setup the pins
	virtual void setup() {
		pinMode(_pin, INPUT);
	}

	// Create our service routine
	virtual void service() {
		int sensorValue = analogRead(_pin);   // read the input on analog pin

		float voltage = sensorValue * (3.3 / 1023.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
		Serial.print("Light Measurement: ");
		Serial.print(sensorValue);

		Serial.print(", ");
		Serial.print(voltage);
		Serial.print(" volts");

		// Sensor shows 525 when it's dark, so calibrate for that
		float percentage = (sensorValue / 1023.0) * 100.0;
		Serial.print(", ");
		Serial.print(percentage);
		Serial.print("%");

		if (percentage > 80.0) {
			Serial.println(" -- its bright!");
		} else if (percentage > 40.0) {
			Serial.println(" -- its manageable");
		} else {
			Serial.println(" -- it's dark! Turn on some lights!");
		}

		int lux = percentage + 1000;
		// "c8y_LightSensor, String(lux)

		String lightString = "c8y_LightSensor,e," + String(lux) + ",lux";
		C8YClient.publish(200, lightString);
		flash();
	}
private:
	int _pin; // The pin the LED is on
};


Scheduler sched; // Create a global Scheduler object

// Create our blink processes
//BlinkProcess blink1(sched, HIGH_PRIORITY, random(50,1000), YELLOW); // Blink every 250 ms
//BlinkProcess blink2(sched, HIGH_PRIORITY, random(50,1000), BLUE); // Blink every 500 ms
//BlinkProcess blink3(sched, HIGH_PRIORITY, random(50,1000), GREEN); // Blink every 1000 ms
TempAndHumidityProcess readTemp(sched, HIGH_PRIORITY, 2000);
LightProcess readLight(sched, HIGH_PRIORITY, 1000, LDR);



void setup()
{
	Serial.begin(115200);
	pinMode(BLUE, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(YELLOW,OUTPUT);
	digitalWrite(BLUE, LOW);
	digitalWrite(GREEN, LOW);
	digitalWrite(YELLOW, LOW);

	Serial.println();

	SPIFFS.begin();            // Start the SPI Flash Files System

	if (!SPIFFS.exists("/format.lck")) { // fs not formatted already
		Serial.print("Initializing file system...");
		SPIFFS.format();
		File formatted = SPIFFS.open("/format.lck", "w");
		formatted.print("This file is auto generated. Removing this file will reset the device.");
		formatted.close();
		Serial.println("done!");
	} else {
		Serial.println("File system already initialized...");
	}

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


	// Configure MQTT server/client and callback
	client.setServer(mqttServer.c_str(), mqttPort);
	client.setCallback(callback);

	//Establish connection to MQTT Server
	while (!client.connected()) {
	    Serial.println("Connecting to MQTT...");

	    if (client.connect(mqttDeviceID.c_str(), mqttUser, mqttPassword )) {
	      Serial.println("Connected to " + mqttServer);
	      digitalWrite(GREEN, HIGH);
	    } else {
	      Serial.print("failed with state ");
	      Serial.println(client.state());
	      delay(2000);
	    }
	}



	// Subscribe to C8Y topics
    if (C8YClient.init(mqttDeviceID)) {
      Serial.println("Subscribed to C8Y topics...");
    } else {
      Serial.print("Failed to subscribe to C8Y topics...");
      Serial.print(client.state());
      delay(2000);
    }


	client.subscribe("esp/commands");
	client.subscribe("esp/logger");
	// client.publish("s/us", "Hello from ESP8266");


	// Start DHT sensor
	dht.begin();


//	blink1.add(true);
//	blink2.add(true);
//	blink3.add(false);
	readTemp.add(true);
	readLight.add(true);
}

void callback(const char* topic, byte* payload, unsigned int length)
{
	C8YClient.callback(topic, payload, length);
}

// The loop function is called in an endless loop
void loop()
{

sched.run();
client.loop();

//if (testTheFS){
//	testFS();
//}

}

void flash(){
	digitalWrite(YELLOW, LOW);
	digitalWrite(YELLOW, HIGH);
	delay(100);
	digitalWrite(YELLOW, LOW);
}

void testFS(){
	  // open file for writing
	  File f = SPIFFS.open("/f.txt", "w");
	  if (!f) {
	      Serial.println("file open failed");
	  }
	  Serial.println("====== Writing to SPIFFS file =========");
	  // write 10 strings to file
	  for (int i=1; i<=10; i++){
	    f.print("Millis() : ");
	    f.println(millis());
	    Serial.println(millis());
	    delay(20);
	  }

	  f.close();

	  // open file for reading
	  File g = SPIFFS.open("/f.txt", "r");
	  if (!g) {
	      Serial.println("file open failed");
	  }  Serial.println("====== Reading from SPIFFS file =======");
	  // write 10 strings to file
	  for (int i=1; i<=10; i++){
	    String s=g.readStringUntil('\n');
	    Serial.print(i);
	    Serial.print(":");
	    Serial.println(s);
	  }

	  testTheFS=false;

}


