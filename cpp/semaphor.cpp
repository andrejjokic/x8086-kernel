/*
 * semaphor.cpp
 *
 *  Created on: May 3, 2020
 *      Author: OS1
 */

#include "semaphor.h"
#include "System.h"
#include "kerSem.h"

Semaphore::Semaphore(int init) {

	this->myImpl = System::createSemaphore(this, init);
}
Semaphore::~Semaphore() {

	System::deleteSemaphore(myImpl);
}

int Semaphore::wait(Time maxTimeToWait) {

	return myImpl->wait(maxTimeToWait);
}

int Semaphore::signal(int n) {

	return myImpl->signal(n);
}

int Semaphore::val()const {

	return myImpl->val();
}
