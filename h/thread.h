#pragma once
/*
 * thread.h
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#ifndef THREAD_H_
#define THREAD_H_

typedef unsigned long StackSize;
const StackSize defaultStackSize = 4096;
typedef unsigned int Time; // time, x 55ms
const Time defaultTimeSlice = 2; // default = 2*55ms
typedef int ID;

typedef void (*SignalHandler)();
typedef unsigned SignalId;

class PCB; //Kernel's implementation of a user's thread

class Thread {
public:
	void start();
	void waitToComplete();
	virtual ~Thread();

	ID getId();
	static ID getRunningId();
	static Thread* getThreadById(ID id);

	void signal(SignalId signal);
	void registerHandler(SignalId signal, SignalHandler handler);
	void unregisterAllHandlers(SignalId id);
	void swap(SignalId id, SignalHandler hand1, SignalHandler hand2);

	void blockSignal(SignalId signal);
	static void blockSignalGlobally(SignalId signal);
	void unblockSignal(SignalId signal);
	static void unblockSignalGlobally(SignalId signal);
protected:
	friend class PCB;
	friend void interrupt timer(...);
	friend class System;
	Thread(StackSize stackSize = defaultStackSize, Time timeSlice = defaultTimeSlice);
	virtual void run() {}

private:
	PCB* myPCB;
};

void dispatch();

#endif /* THREAD_H_ */
