/*
 * thread.cpp
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#include "thread.h"
#include "System.h"
#include "pcb.h"
#include "SCHEDULE.H"

void lock();
void unlock();

Thread::~Thread() {

	System::deleteThread(myPCB);
}

Thread::Thread(StackSize stackSize, Time timeSlice) {

	myPCB = System::createThread(this, stackSize, timeSlice);
}

void Thread::start() {
	lock();
	if (myPCB->getStatus() == NEW) {
		myPCB->setStatus(READY);
		Scheduler::put(myPCB);
	}
	unlock();
}

ID Thread::getRunningId() {

	return System::getRunningThread()->getId();
}

ID Thread::getId() {

	return myPCB->getId();
}

Thread* Thread::getThreadById(ID id) {

	PCB* thr = System::getThreadById(id);

	if (thr)
		return thr->myThread;
	return 0;
}

void Thread::waitToComplete() {

	myPCB->waitToComplete();
}

void dispatch() {
	System::dispatch();
}

//signali
void Thread::signal(SignalId signal) {
	myPCB->signal(signal);
}

void Thread::registerHandler(SignalId signal, SignalHandler handler) {
	myPCB->registerHandler(signal, handler);
}

void Thread::unregisterAllHandlers(SignalId id) {
	myPCB->unregisterAllHandlers(id);
}

void Thread::swap(SignalId id, SignalHandler hand1, SignalHandler hand2) {
	myPCB->swap(id, hand1, hand2);
}

void Thread::blockSignal(SignalId signal) {
	myPCB->blockSignal(signal);
}

void Thread::unblockSignal(SignalId signal) {
	myPCB->unblockSignal(signal);
}

void Thread::blockSignalGlobally(SignalId signal) {
	PCB::blockSignalGlobally(signal);
}

void Thread::unblockSignalGlobally(SignalId signal) {
	PCB::unblockSignalGlobally(signal);
}
