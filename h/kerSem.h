#pragma once
/*
 * kerSem.h
 *
 *  Created on: May 3, 2020
 *      Author: OS1
 */

#ifndef KERSEM_H_
#define KERSEM_H_

#include "Queue.h"
#include "pcb.h"
#include "thread.h"

class Semaphore;
struct blockedPCB;

class KernelSem {
public:
	KernelSem(Semaphore* mySemaphore, int init = 1);
	~KernelSem();
	int wait(Time maxTimeToWait);
	int signal(int n = 0);
	int val() const; // Returns the current value of the semaphore
	static void tickAllSemaphores();	//reduces waitingTime on all blocked semaphores(with limited time waiting)
	void reduceTime();

private:
	Semaphore* mySemaphore;
	Queue<blockedPCB*> blocked;
	int value;
};

struct blockedPCB {
	PCB* pcb;
	Time waitingTime;
	int unlimitedWait;
	blockedPCB(PCB* p, Time t) {
		pcb = p;
		waitingTime = t;
		unlimitedWait = t ? 0 : 1;
	}
};


#endif /* KERSEM_H_ */
