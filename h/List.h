#pragma once
/*
 * List.h
 *
 *  Created on: Apr 12, 2020
 *      Author: OS1
 */

#ifndef LIST_H_
#define LIST_H_

template<class T>
class List {

public:

	List() : first(0), last(0), size(0) {}

	List(const List<T>& list2) {
		first = 0;
		last = 0;
		size = 0;
		elem* pom = list2.first;
		while (pom) {
			insert(pom->data);
			pom = pom->next;
		}
	}

	List<T>& operator=(const List<T> l2) {
		while (first) {
			T pod = first->data;
			remove(pod);
		}
		first = 0;
		last = 0;
		size = 0;
		elem* pom = l2.first;
		while (pom) {
			insert(pom->data);
			pom = pom->next;
		}
		return *this;
	}

	int getSize()const { return size; }

	void insert(T d) {

		last = first ? last->next : first = new elem(d, 0);
		size++;
	}

	void insertInPosition(T d, int i) {

		if (i < 0)
			return;

		if (i > size)
			i = size;

		if (i == 0) {
			elem* nov = new elem(d, first);
			first = nov;
			if (!last)
				last = nov;
			size++;
			return;
		}

		elem* prev = first;

		while (--i > 0)
			prev = prev->next;

		elem* nov = new elem(d, prev->next);
		prev->next = nov;

		if (prev == last)
			last = nov;

		size++;
	}

	int getPosition(T t) {

		elem* pom = first;
		int pos = 0;

		while (pom && pom->data != t) {
			pom = pom->next;
			pos++;
		}

		if (pom)
			return pos;
		else
			return -1;
	}

	T remove(T t) {

		if (!first)
			return 0;

		elem* pom = first;
		elem* prev = 0;

		while (pom && pom->data != t) {
			prev = pom;
			pom = pom->next;
		}

		if (!pom)
			return 0;

		T ret = 0;

		if (pom == first) {
			first = first->next;
			if (!first)
				last = 0;
			pom->next = 0;
			ret = pom->data;
			delete pom;

		}
		else {
			prev->next = pom->next;
			if (!pom->next)
				last = prev;
			pom->next = 0;
			ret = pom->data;
			delete pom;
		}

		size--;
		return ret;
	}

	T get(int i) {

		elem* pom = first;

		while (pom && i > 0) {
			pom = pom->next;
			i--;
		}

		if (pom)
			return pom->data;

		return 0;

	}

	T removeFromPosition(int i) {

		if (i >= size)
			return 0;

		T elem = get(i);

		return remove(elem);
	}

	T find(T data) {

		elem* pom = first;

		while (pom && pom->data != data)
			pom = pom->next;

		if (pom)
			return pom->data;

		return 0;
	}


	~List() {

		while (size > 0) {
			T pod = first->data;
			remove(pod);
		}
	}


private:
	struct elem {
		T data;
		elem* next;
		elem(T d, elem* n) {
			data = d;
			next = n;
		}
	};

	int size;
	elem* first, * last;
};



#endif /* LIST_H_ */
