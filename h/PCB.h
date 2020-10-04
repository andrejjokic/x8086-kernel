#pragma once
/*
 * pcb.h
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#ifndef PCB_H_
#define PCB_H_

#include "thread.h"
#include "List.h"

typedef enum Status { NEW, READY, RUNNING, BLOCKED, ENDED };
const StackSize maxStackSize = 65536;
const StackSize minStackSize = 512;


class PCB {
public:
	PCB(Thread* myThread, StackSize stackSize, Time timeSlice);
	~PCB();
	static void wrapper();
	Status getStatus()const;
	void setStatus(Status status);
	ID getId()const;
	static ID getThreadId(Thread* thr);
	Time getTimeSlice()const;
	void waitToComplete();
	void notifyAllWaitingOnMe();

	//signali
	void signal(SignalId signal);
	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId id);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);
	void blockSignal(SignalId signal);
	void unblockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);
	void processSignals();
	PCB* getParent();

protected:
	friend class System;
	friend class Thread;
	friend void interrupt timer(...);
private:
	void initStack();
	void removeFromWaitingList(PCB* pcb);

	static ID ids;
	ID id;
	Thread* myThread;
	unsigned* stack;
	unsigned sp;	//offset part of stack pointer
	unsigned ss;	//segment part of stack pointer
	unsigned bp;
	StackSize stackSize;
	Time timeSlice;
	Status status;
	List<PCB*> waitingOnMe;

	PCB* parent;		//parent thread
	static int signalBlockedGlobally[16];			//if signal with that ordinal number is globally blocked
	int signalBlockedLocally[16];
	List<int> signalsReceived;		//list of signals that thread has received and not yet process
	List<SignalHandler> signalHandlers[16];	//array of signal handlers,assigned for each signal id
};



#endif /* PCB_H_ */
