/*
 * pcb.cpp
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */
#include "pcb.h"
#include <dos.h>
#include "System.h"
#include "SCHEDULE.h"

void lock();
void unlock();

unsigned spp; 			//temporary global variable used for dispatch() as SP Offset
unsigned sss;			//temporary global variable used for dispatch() as SP Segment
unsigned bpp; 			//temporary global variable used for dispatch() as BP Offset

ID PCB::ids = 0;
int PCB::signalBlockedGlobally[16] = { 0 };			//if signal with that ordinal number is globally blocked

PCB::~PCB() {
	lock();
	delete[] stack;
	unlock();
}

PCB::PCB(Thread* myThread, StackSize stackSize, Time timeSlice) {

	lock();
	stackSize /= sizeof(unsigned);
	this->myThread = myThread;
	this->timeSlice = timeSlice;
	this->stackSize = stackSize;
	this->status = NEW;
	this->id = ++ids;
	this->stack = 0;

	//signali
	for (int i = 0; i < 16; i++) {
		signalBlockedLocally[i] = 0;
	}
	this->parent = System::runningThread;
	if (parent) {
		for (int j = 0; j < 16; j++) {
			this->signalHandlers[j] = System::runningThread->signalHandlers[j];
			this->signalBlockedLocally[j] = System::runningThread->signalBlockedLocally[j];
		}
	}

	initStack();
	unlock();
}

void PCB::initStack() {

	if (stackSize > 0) {

		stack = new unsigned[stackSize];

		stack[stackSize - 1] = 0x200;	//PSWI set to 1,so interrupts are enabled

#ifndef BCC_BLOCK_IGNORE
		stack[stackSize - 2] = FP_SEG(PCB::wrapper);	//Segment part of adress of wrapper function of run
		stack[stackSize - 3] = FP_OFF(PCB::wrapper);	//Offset part of adress of wrapper function of run
		ss = FP_SEG(stack + stackSize - 12);
		sp = FP_OFF(stack + stackSize - 12);
		bp = sp;
#endif
		/*
		 stack[stackSize - 4] = BP;
		 stack[stackSize - 5] = DS;
		 stack[stackSize - 6] = ES;
		 stack[stackSize - 7] = AX;
		 stack[stackSize - 8] = BX;
		 stack[stackSize - 9] = CX;
		 stack[stackSize - 10] = DX;
		 stack[stackSize - 11] = SI;
		 stack[stackSize - 12] = DI;
		 */
	}
	else {	//it is main thread

#ifndef BCC_BLOCK_IGNORE
		asm{
			mov sss,ss
			mov spp,sp
			mov bpp,bp
		}
#endif
		ss = sss;
		sp = spp;
		bp = bpp;
	}
}

void PCB::wrapper() {

	System::runningThread->myThread->run();			//run() method is a body of a thread

	System::runningThread->signal(0);					//thread has ended,so we can kill it now

	System::runningThread->processSignals();			//we process all the signals left for our thread

	//dispatch();
}

Status PCB::getStatus()const {
	return status;
}

void PCB::setStatus(Status status) {
	this->status = status;
}

ID PCB::getId()const {
	return id;
}

PCB* PCB::getParent() {
	return parent;
}

ID PCB::getThreadId(Thread* thr) {

	return System::getThreadId(thr->myPCB);
}

Time PCB::getTimeSlice()const {
	return timeSlice;
}

void PCB::waitToComplete() {

	if ((System::runningThread != this) && (this->getStatus() != ENDED) && (this != System::iddleThread->myPCB)) {
		lock();
		System::runningThread->setStatus(BLOCKED);
		waitingOnMe.insert(System::runningThread);
		unlock();
		dispatch();
	}
}

/*
void PCB::removeFromWaitingList(PCB* pcb) {

lock();
	PCB* removedOne = waitingOnMe.remove(pcb);

	if(!removedOne) {
unlock();
		return;
	}

	removedOne->setStatus(READY);
	Scheduler::put(removedOne);
unlock();
}*/

void PCB::removeFromWaitingList(PCB* pcb) {

	PCB* removedOne = waitingOnMe.remove(pcb);

	if (!removedOne) {
		return;
	}

	removedOne->setStatus(READY);
	Scheduler::put(removedOne);
}

/*
void PCB::notifyAllWaitingOnMe() {
lock();
	while (waitingOnMe.getSize() > 0) {
		PCB* toBeRemoved = waitingOnMe.get(0);
		removeFromWaitingList(toBeRemoved);
	}
unlock();
}*/

void PCB::notifyAllWaitingOnMe() {

	while (waitingOnMe.getSize() > 0) {
		PCB* toBeRemoved = waitingOnMe.removeFromPosition(0);
		if (toBeRemoved) {
			lock();
			toBeRemoved->setStatus(READY);
			Scheduler::put(toBeRemoved);
			unlock();
		}
	}
}

//signali
void PCB::signal(SignalId signal) {
	if (signal < 0 || signal > 15)
		return;

	signalsReceived.insert(signal);
}

void PCB::registerHandler(SignalId signal, SignalHandler handler) {
	if (signal < 0 || signal > 15)
		return;

	signalHandlers[signal].insert(handler);
}

void PCB::unregisterAllHandlers(SignalId id) {
	if (id < 0 || id > 15)
		return;

	while (signalHandlers[id].getSize() > 0) {
		signalHandlers[id].removeFromPosition(0);
	}
}

void PCB::swap(SignalId id, SignalHandler hand1, SignalHandler hand2) {

	if (id < 0 || id > 15 || !signalHandlers[id].find(hand1) || !signalHandlers[id].find(hand2))		//there is no signal handler hand1 or hand2
		return;

	int hand1pos = signalHandlers[id].getPosition(hand1);
	int hand2pos = signalHandlers[id].getPosition(hand2);

	signalHandlers[id].remove(hand1);
	signalHandlers[id].remove(hand2);

	if (hand2pos < hand1pos) {
		signalHandlers[id].insertInPosition(hand1, hand2pos);
		signalHandlers[id].insertInPosition(hand2, hand1pos);
	}
	else {
		signalHandlers[id].insertInPosition(hand2, hand1pos);
		signalHandlers[id].insertInPosition(hand1, hand2pos);
	}
}


void PCB::blockSignal(SignalId signal) {
	if (signal < 0 || signal > 15)
		return;

	signalBlockedLocally[signal] = 1;
}

void PCB::unblockSignal(SignalId signal) {
	if (signal < 0 || signal > 15)
		return;

	signalBlockedLocally[signal] = 0;
}

void PCB::blockSignalGlobally(SignalId signal) {
	if (signal < 0 || signal > 15)
		return;

	signalBlockedGlobally[signal] = 1;
}
void PCB::unblockSignalGlobally(SignalId signal) {
	if (signal < 0 || signal > 15)
		return;

	signalBlockedGlobally[signal] = 0;
}

void PCB::processSignals() {
	int size = signalsReceived.getSize();

	for (int i = 0; i < size; i++) {
		SignalId sId = signalsReceived.removeFromPosition(0);

		if (signalBlockedGlobally[sId] || signalBlockedLocally[sId]) {		//if signal is blocked locally or globally,we don't process it
			signalsReceived.insert(sId);									//so it remains in signals received list
			continue;
		}

		for (int j = 0; j < signalHandlers[sId].getSize(); j++) {
			SignalHandler sh = signalHandlers[sId].get(j);
			(*sh)();
		}

	}
}

void signal0() {		//violently kills thread and free all it's resources
	lock();
	if (System::runningThread->getParent())
		System::runningThread->getParent()->signal(1);		 //inform parent , when thread finishes,we have to send signal to it's parent thread,it is signal 1

	System::runningThread->signal(2);						//inform me , when thread finishes,we have to send signal to that thread,it is signal 2

	System::runningThread->processSignals();				//if there are more signals after this one,we process them too

	System::runningThread->setStatus(ENDED);
	unlock();
	dispatch();
}

void signalEndOfRun() {
	lock();
	System::runningThread->notifyAllWaitingOnMe();
	unlock();
}
