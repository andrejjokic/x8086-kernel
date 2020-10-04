#pragma once
/*
 * Queue.h
 *
 *  Created on: Apr 12, 2020
 *      Author: OS1
 */

#ifndef QUEUE_H_
#define QUEUE_H_

template<class T>
class Queue {
private:
	struct Node {
		T data;
		Node* next;
		Node(T d, Node* n) {
			data = d;
			next = n;
		}
	};
public:
	Queue() : front(0), rear(0), size(0) {}

	T peek() {
		if (front)
			return front->data;
		else
			return 0;
	}

	void push(T data) {

		rear = front ? rear->next : front = new Node(data, 0);
		size++;
	}

	T pop() {

		T ret = 0;

		if (front) {
			ret = front->data;
			Node* old = front;
			front = front->next;
			old->next = 0;
			if (!front)
				rear = 0;

			size--;
			delete old;
		}

		return ret;
	}

	void remove(T t) {

		if (isEmpty())
			return;

		Node* pom = front;
		Node* prev = 0;

		while (pom && pom->data != t) {
			prev = pom;
			pom = pom->next;
		}

		if (!pom)
			return;

		if (pom == front) {
			front = front->next;
			if (!front)
				rear = 0;
			pom->next = 0;
			delete pom;
		}
		else {
			prev->next = pom->next;
			if (!pom->next)
				rear = prev;
			pom->next = 0;
			delete pom;
		}

		size--;
	}

	int isEmpty()const {
		return size == 0;
	}

	int getSize()const {
		return size;
	}

	T get(int i) {

		Node* pom = front;

		while (pom && i > 0) {
			pom = pom->next;
			i--;
		}

		if (pom)
			return pom->data;

		return 0;
	}

	~Queue() {

		while (front) {
			pop();
		}
	}
private:
	Node* front, * rear;
	int size;
};




#endif /* QUEUE_H_ */
