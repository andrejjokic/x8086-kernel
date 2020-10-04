/*
 * kernelEv.cpp
 *
 *  Created on: May 5, 2020
 *      Author: OS1
 */

#include "System.h"
#include "thread.h"
#include "pcb.h"
#include "kernelEv.h"
#include "SCHEDULE.H"

void lock();
void unlock();


KernelEv* KernelEv::allEvents[256] = { 0 };

KernelEv::KernelEv(IVTNo ivtNo) {
	//lock();
	this->ivtNo = ivtNo;
	this->myCreator = System::runningThread;
	this->blocked = 0;
	this->value = 0;
	allEvents[ivtNo] = this;
	//unlock();
}

KernelEv::~KernelEv() {
	//lock();
	allEvents[ivtNo] = 0;
	if (blocked) {
		blocked->setStatus(READY);
		Scheduler::put(blocked);
	}
	blocked = 0;
	myCreator = 0;
	//unlock();
}


void KernelEv::wait() {

	if (System::runningThread != this->myCreator) {
		return;
	}

	if (value == 1) {
		value = 0;
		return;
	}

	lock();
	this->blocked = System::runningThread;
	System::runningThread->setStatus(BLOCKED);
	unlock();
	dispatch();
}



void KernelEv::signal() {

	if (!blocked) {
		value = 1;
		return;
	}
	lock();
	blocked->setStatus(READY);
	Scheduler::put(blocked);
	blocked = 0;
	unlock();

	dispatch();	//maybe not needed, will see
}
