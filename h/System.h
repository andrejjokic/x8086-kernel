#pragma once
/*
 * System.h
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include "List.h"
#include "thread.h"
#include <stdio.h>

class PCB;
class Semaphore;
class Thread;
class KernelSem;

typedef void interrupt(*IntRoutine) (...);

class IddleThread;

class System {
public:
	static PCB* createThread(Thread* myThread, StackSize stackSize, Time timeSlice);	//creates thread and adds it to the list of all threads
	static void deleteThread(PCB* pcb);			 									//deletes thread and removes it from the list of all threads
	static PCB* getRunningThread();
	static PCB* getThreadById(ID id);
	static ID getThreadId(PCB* thread);
	static void dispatch();
	static KernelSem* createSemaphore(Semaphore* mySem, int val);
	static void deleteSemaphore(KernelSem* sem);
	static void tickAllSemaphores();

	static void addThread(PCB* thread);
	static void removeThread(PCB* thread);
	static void setNewTimerRoutine();	//puts new timer routine address in place of Timer in IVT
	static void restoreOldTimerRoutine();	//restores old timer routine
	static void init();
	static void deInit();

	static List<PCB*>* allThreads;
	static List<KernelSem*>* allSemaphores;
	static PCB* runningThread;
	static volatile int switchRequested;
	static IntRoutine oldTimer;
	static volatile int currentThreadTimer;
	static Thread* iddleThread;	//if there is no thread to run(all blocked),then idle thread will run

};

class IddleThread : public Thread {
public:
	IddleThread() : Thread(512, 1) {}
protected:
	//friend class System;
	virtual void run() {
		while (1 < 2) {}
	}
};

extern int ret;
int userMain(int, char**);

class UserMain : public Thread {
public:
	UserMain(StackSize s, Time t, int argc, char** argv) : Thread(s, t) { this->argc = argc; this->argv = argv; }
	virtual ~UserMain() { waitToComplete(); }
protected:
	virtual void run() {
		ret = userMain(argc, argv);
	}
private:
	int argc;
	char** argv;
};

#endif /* SYSTEM_H_ */
