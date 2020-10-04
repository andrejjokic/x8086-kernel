/*
 * IVTEntry.cpp
 *
 *  Created on: May 5, 2020
 *      Author: OS1
 */

#include "IVTEntry.h"
#include "kernelEv.h"
#include "System.h"
#include "event.h"
#include <dos.h>

IVTEntry* IVTEntry::allEntries[256] = { 0 };

IVTEntry::IVTEntry(IVTNo ivtNo, IntRoutine intRoutine) : oldRoutine(0) {

#ifndef BCC_BLOCK_IGNORE
	asm cli;
	oldRoutine = getvect(ivtNo);
	setvect(ivtNo, intRoutine);
	asm sti;
#endif

	this->ivtNo = ivtNo;
	allEntries[ivtNo] = this;;

}

IVTEntry::~IVTEntry() {

#ifndef BCC_BLOCK_IGNORE
	asm cli;
	setvect(ivtNo, oldRoutine);
	asm sti;
#endif

	allEntries[ivtNo] = 0;

}

IVTEntry* IVTEntry::getEntry(IVTNo ivtNo) {
	return allEntries[ivtNo];
}

void IVTEntry::signal() {

	if (KernelEv::allEvents[ivtNo]) {
		KernelEv::allEvents[ivtNo]->signal();
	}
}

void IVTEntry::callOld() {

	if (oldRoutine) {
		(*oldRoutine)();
	}
}
