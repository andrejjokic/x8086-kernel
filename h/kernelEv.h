#pragma once
/*
 * kernelEv.h
 *
 *  Created on: May 5, 2020
 *      Author: OS1
 */

#ifndef KERNELEV_H_
#define KERNELEV_H_

#include "event.h"

class PCB;

class KernelEv {
public:
	KernelEv(IVTNo ivtNo);
	~KernelEv();
	void wait();
protected:
	friend class IVTEntry;
	friend class Event;
	void signal();
private:
	int value;
	PCB* myCreator;
	PCB* blocked;
	IVTNo ivtNo;
	static KernelEv* allEvents[256];
};



#endif /* KERNELEV_H_ */
