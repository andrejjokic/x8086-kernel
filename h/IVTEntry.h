#pragma once
/*
 * IVTEntry.h
 *
 *  Created on: May 5, 2020
 *      Author: OS1
 */

#ifndef IVTENTRY_H_
#define IVTENTRY_H_

#include "System.h"
#include "kernelEv.h"

#define PREPAREENTRY(num,old)\
	void interrupt newRoutine##num(...);\
	IVTEntry ivtEntry##num = IVTEntry(num,newRoutine##num);\
	void interrupt newRoutine##num(...){\
		ivtEntry##num.signal();\
		if (old == 1) ivtEntry##num.callOld();\
	}

class IVTEntry {

public:
	IVTEntry(IVTNo ivtNo, IntRoutine intRoutine);
	~IVTEntry();
	static IVTEntry* getEntry(IVTNo ivtNo);
	void signal();
	void callOld();
private:
	static IVTEntry* allEntries[256];
	IntRoutine oldRoutine;
	IVTNo ivtNo;
};


#endif /* IVTENTRY_H_ */
