/*
 * kerSem.cpp
 *
 *  Created on: May 3, 2020
 *      Author: OS1
 */

#include "kerSem.h"
#include "SCHEDULE.H"
#include "semaphor.h"
#include "System.h"

void lock();
void unlock();


KernelSem::KernelSem(Semaphore* mySemaphore, int init) {

	this->value = init;
	this->mySemaphore = mySemaphore;
}

KernelSem::~KernelSem() {

	while (blocked.getSize() > 0) {
		blockedPCB* blkPCB = blocked.pop();
		blkPCB->pcb->setStatus(READY);
		Scheduler::put(blkPCB->pcb);
	}

}


int KernelSem::wait(Time maxTimeToWait) {

	int ret = 1;

	lock();
	if (--value < 0) {
		blockedPCB* blkPCB = new blockedPCB(System::runningThread, maxTimeToWait);
		blocked.push(blkPCB);
		System::runningThread->setStatus(BLOCKED);
		unlock();
		dispatch();

		if (!blkPCB->unlimitedWait && blkPCB->waitingTime == 0) {
			ret = 0;
		}
		lock();
		delete blkPCB;
	}
	unlock();

	return ret;
}

int KernelSem::signal(int num) {

	if (num < 0)
		return num;

	int ret;

	if (num == 0) {
		ret = 0;
		lock();
		if (value++ < 0) {	//there was blocked thread
			blockedPCB* unblocked = blocked.pop();
			unblocked->pcb->setStatus(READY);
			Scheduler::put(unblocked->pcb);
		}
		unlock();

	}
	else {
		int i = 0;
		while ((blocked.getSize() > 0) && (i < num)) {
			lock();
			blockedPCB* unblocked = blocked.pop();
			unblocked->pcb->setStatus(READY);
			Scheduler::put(unblocked->pcb);
			unlock();
			i++;
		}
		lock();
		value += num;
		unlock();
		ret = i;
	}

	return ret;
}


int KernelSem::val() const {
	return value;
}

void KernelSem::reduceTime() {

	int size = blocked.getSize();

	for (int i = 0; i < size; i++) {
		blockedPCB* blk = blocked.pop();
		if ((--blk->waitingTime <= 0) && (!blk->unlimitedWait)) {
			blk->pcb->setStatus(READY);
			Scheduler::put(blk->pcb);
		}
		else {
			blocked.push(blk);
		}
	}
}

void KernelSem::tickAllSemaphores() {
	System::tickAllSemaphores();
}
