/*
 * System.cpp
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#include "thread.h"
#include "pcb.h"
#include "SCHEDULE.H"
#include "System.h"
#include <dos.h>
#include <stdio.h>
#include "kerSem.h"
#include "semaphor.h"

List<PCB*>* System::allThreads = new List<PCB*>;	//list of all active threads
PCB* System::runningThread = 0;		//currently running thread on processor
volatile int System::switchRequested = 0;	//if context switch has been requested
IntRoutine System::oldTimer = 0;	//keeps old timer routine
volatile int System::currentThreadTimer = 0;	//time that current thread is running
Thread* System::iddleThread = 0;	//iddle thread will run if there is no ready thread to be run
List<KernelSem*>* System::allSemaphores = new List<KernelSem*>;	//list of all semaphores

volatile int lockCount = 0;	//if it is 0,preemption is allowed
unsigned tsp; 			//temporary global variable used for dispatch() as SP Offset
unsigned tss;			//temporary global variable used for dispatch() as SP Segment
unsigned tbp; 			//temporary global variable used for dispatch() as BP Offset

void lock();
void unlock();
void tick();

void signal0();		//predefined function for signal 0 that kills thread violently and free all it's resources

void signalEndOfRun();	//when thread ends,it will process this signal

//dodatak krece

unsigned realOff;
unsigned realSeg;
unsigned wrapOff;
unsigned wrapSeg;
unsigned tmpPSW;
unsigned ticked = 1;
unsigned tmpAX;
unsigned tmpBX;
unsigned tmpCX;
unsigned tmpDX;
unsigned tmpES;
unsigned tmpDS;
unsigned tmpSI;
unsigned tmpDI;
unsigned tmpBP;

void wrap() {
	//lock();
	if (ticked) {
		ticked = 0;
		tick();
		lock();
		System::tickAllSemaphores();
		unlock();
	}

	//unlock();
}

//kraj dodatka

void System::init() {

	setNewTimerRoutine();
	runningThread = new PCB(0, 0, 7);		//we need to create PCB for our main thread,has id of 1
	addThread(runningThread);
	runningThread->setStatus(RUNNING);	//and set its status to RUNNING
	runningThread->registerHandler(0, signal0);	//register predefined function for signal 0 that kills thread violently and free all it's resources
	runningThread->registerHandler(2, signalEndOfRun);		//when our thread ends , we will process this signal
	currentThreadTimer = 0;

	iddleThread = new IddleThread();	//has id of 2
	//iddleThread->start();

}

void System::deInit() {

	restoreOldTimerRoutine();	//restores old timer routine which we manually moved to 60h

	delete iddleThread;

	delete runningThread;

	delete allSemaphores;

	delete allThreads;

	//delete iddleThread;
}

PCB* System::createThread(Thread* myThread, StackSize stackSize, Time timeSlice) {

	if (stackSize > maxStackSize)
		stackSize = maxStackSize;

	if (stackSize < minStackSize)
		stackSize = minStackSize;

	lock();
	PCB* newThread = new PCB(myThread, stackSize, timeSlice);
	addThread(newThread);
	unlock();
	return newThread;
	//enable preemption
}

void System::addThread(PCB* thread) {

	//lock();
	allThreads->insert(thread);
	//unlock();
}

void System::removeThread(PCB* thread) {
	//lock();
	allThreads->remove(thread);
	//unlock();
}

PCB* System::getRunningThread() {
	return runningThread;
}

PCB* System::getThreadById(ID id) {
	int i = 0;
	PCB* pcb = 0;

	do {
		pcb = allThreads->get(i++);
	} while (pcb && pcb->getId() != id);

	return pcb;
}

ID System::getThreadId(PCB* thread) {

	return allThreads->find(thread)->getId();
}

void System::deleteThread(PCB* pcb) {

	lock();
	removeThread(pcb);
	delete pcb;
	unlock();
}

void System::restoreOldTimerRoutine() {

#ifndef BCC_BLOCK_IGNORE
	asm cli;
	setvect(8, oldTimer);
	asm sti;
#endif
}

/*
void interrupt timer(...) {

	if (!System::switchRequested) {
#ifndef BCC_BLOCK_IGNORE			//if context switch was not requested
		asm int 60h;				//it means it was regular timer interrupt,so we must call it's routine
#endif
		tick();
		System::currentThreadTimer++;
		System::tickAllSemaphores();
	}

	if (((System::currentThreadTimer >= System::runningThread->getTimeSlice()) && (System::runningThread->getTimeSlice())) || (System::switchRequested)) {

		if (!lockCount) {

			System::switchRequested = 0;

			//saving context of running thread
#ifndef BCC_BLOCK_IGNORE
			asm {
				mov tss,ss
				mov tsp,sp
				mov tbp,bp
			}
#endif
			System::runningThread->sp = tsp;
			System::runningThread->ss = tss;
			System::runningThread->bp = tbp;

			//picking other thread to run
			if ((System::runningThread->getStatus() == RUNNING) && (System::runningThread != System::iddleThread->myPCB)) {
				System::runningThread->setStatus(READY);
				Scheduler::put(System::runningThread);
			}

			if (System::runningThread == System::iddleThread->myPCB)
				System::runningThread->setStatus(READY);

			//dodato za signale
			if (System::runningThread->getStatus() == ENDED) {
				delete[] System::runningThread->stack;
				System::runningThread->stack = 0;
			}

			System::runningThread = Scheduler::get();

			if (!System::runningThread)	 {		//if there is no ready thread,then we put our iddle thread to run
				System::runningThread = System::iddleThread->myPCB;
			}

			System::runningThread->setStatus(RUNNING);

			//restaurating context of a new running thread

			tss = System::runningThread->ss;
			tsp = System::runningThread->sp;
			tbp = System::runningThread->bp;

#ifndef BCC_BLOCK_IGNORE
			asm {
				mov ss,tss
				mov sp,tsp
				mov bp,tbp
			}
#endif

			System::currentThreadTimer = 0; //reset the timer of running thread

		}  else {	//end of context switch block

			System::switchRequested = 1;
		}
	}//end of if block

	System::runningThread->processSignals();
}
*/

void interrupt timer(...) {

	if (!System::switchRequested) {
#ifndef BCC_BLOCK_IGNORE			//if context switch was not requested
		asm int 60h;				//it means it was regular timer interrupt,so we must call it's routine
#endif
		System::currentThreadTimer++;
		ticked = 1;
	}

	if (((System::currentThreadTimer >= System::runningThread->getTimeSlice()) && (System::runningThread->getTimeSlice())) || (System::switchRequested)) {

		if (!lockCount) {

			System::switchRequested = 0;

			//saving context of running thread
#ifndef BCC_BLOCK_IGNORE
			asm{
				mov tss,ss
				mov tsp,sp
				mov tbp,bp
			}
#endif
			System::runningThread->sp = tsp;
			System::runningThread->ss = tss;
			System::runningThread->bp = tbp;

			//picking other thread to run
			if ((System::runningThread->getStatus() == RUNNING) && (System::runningThread != System::iddleThread->myPCB)) {
				System::runningThread->setStatus(READY);
				Scheduler::put(System::runningThread);
			}

			if (System::runningThread == System::iddleThread->myPCB)
				System::runningThread->setStatus(READY);

			//dodato za signale
			if (System::runningThread->getStatus() == ENDED) {
				delete[] System::runningThread->stack;
				System::runningThread->stack = 0;
			}

			System::runningThread = Scheduler::get();

			if (!System::runningThread) {		//if there is no ready thread,then we put our iddle thread to run
				System::runningThread = System::iddleThread->myPCB;
			}

			System::runningThread->setStatus(RUNNING);

			//restaurating context of a new running thread

			tss = System::runningThread->ss;
			tsp = System::runningThread->sp;
			tbp = System::runningThread->bp;

#ifndef BCC_BLOCK_IGNORE
			asm{
				mov ss,tss
				mov sp,tsp
				mov bp,tbp
			}
#endif

			System::currentThreadTimer = 0; //reset the timer of running thread

		}
		else {	//end of context switch block

			System::switchRequested = 1;
		}
	}//end of if block

	System::runningThread->processSignals();

#ifndef BCC_BLOCK_IGNORE

	wrapSeg = FP_SEG(wrap);
	wrapOff = FP_OFF(wrap);

	asm{
		pop bp
		mov tmpBP,bp
		pop bp
		mov tmpDI,bp
		pop bp
		mov tmpSI,bp
		pop bp
		mov tmpDS,bp
		pop bp
		mov tmpES,bp
		pop bp
		mov tmpDX,bp
		pop bp
		mov tmpCX,bp
		pop bp
		mov tmpBX,bp
		pop bp
		mov tmpAX,bp
		pop bp
		mov realOff,bp
		pop bp
		mov realSeg,bp
		pop bp
		mov tmpPSW,bp

		push realSeg
		push realOff
		push tmpPSW
		push wrapSeg
		push wrapOff
		push tmpAX
		push tmpBX
		push tmpCX
		push tmpDX
		push tmpES
		push tmpDS
		push tmpSI
		push tmpDI
		push tmpBP
	}
#endif
}

void System::setNewTimerRoutine() {

#ifndef BCC_BLOCK_IGNORE
	asm cli;
	oldTimer = getvect(0x8);	//getvect() returns address of a function on entry 8h in IVT
	setvect(0x8, timer);
	setvect(0x60, oldTimer);		//old timer routine is now set on entry 60h
	asm sti;
#endif
}

void lock() {
	lockCount++;
}

void unlock() {
	if (--lockCount < 0)
		lockCount = 0;
}

void System::dispatch() {
	lock();
	System::switchRequested = 1;
	unlock();
#ifndef BCC_BLOCK_IGNORE
	asm int 8h;				//we call timer routine which will do the context switch
#endif
}

KernelSem* System::createSemaphore(Semaphore* mySem, int val) {
	lock();
	KernelSem* sem = new KernelSem(mySem, val);
	allSemaphores->insert(sem);
	unlock();
	return sem;
}

void System::deleteSemaphore(KernelSem* semaphore) {
	lock();
	allSemaphores->remove(semaphore);
	delete semaphore;
	unlock();
}

void System::tickAllSemaphores() {

	for (int i = 0; i < allSemaphores->getSize(); i++) {
		allSemaphores->get(i)->reduceTime();
	}
}
