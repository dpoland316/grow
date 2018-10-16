/*
 * SensorThreads.h
 *
 *  Created on: 15.10.2018
 *      Author: dpo
 */

#ifndef SENSORTHREADS_H_
#define SENSORTHREADS_H_
#include "ProcessScheduler.h"
#include "DHT.h"


DHT* _dht;

class TempAndHumidityProcess: public Process{
private:
	//Scheduler* manager;

public:
	// Call the Process constructor
	TempAndHumidityProcess(Scheduler &manager, ProcPriority pr,
			unsigned int period, DHT &dht);
	//TempAndHumidityProcess& setSensor(DHT& dht);

protected:

	virtual void service();

};






#endif /* SENSORTHREADS_H_ */
