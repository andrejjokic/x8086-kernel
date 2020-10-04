/*
 * main.cpp
 *
 *  Created on: May 2, 2020
 *      Author: OS1
 */

#include "System.h"
#include "thread.h"
#include <stdio.h>
#include "kerSem.h"
#include "semaphor.h"

int userMain(int, char**);
void lock();
void unlock();

int ret = 0;

class UserMain;

int main(int argc, char** argv) {

	System::init();

	lock();
	Thread* user = new UserMain(512, 0, argc, argv);
	unlock();
	user->start();

	printf("\nlele\n");

	delete user;

	System::deInit();

	return ret;

}
