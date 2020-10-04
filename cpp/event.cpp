/*
 * event.cpp
 *
 *  Created on: May 5, 2020
 *      Author: OS1
 */

#include "event.h"
#include "kernelEv.h"

void lock();
void unlock();

Event::Event(IVTNo ivtNo) {
	lock();
	myImpl = new KernelEv(ivtNo);
	unlock();
}

Event::~Event() {
	lock();
	delete myImpl;
	unlock();
}

void Event::wait() {
	myImpl->wait();
}

void Event::signal() {
	myImpl->signal();
}

