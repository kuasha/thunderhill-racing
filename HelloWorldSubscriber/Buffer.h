/*
 * Buffer.h
 *
 *
 *  Created on: 06.03.2016
 */

#ifndef BUFFER_H_
#define BUFFER_H_


#include "stdlib.h"
#include <pthread.h>
namespace std {

/*
	This class provides a thread safe RingBuffer for 
	storing and retrieveing elements using a FIFO rule.
*/
template<class T> class Buffer{

private:

	T * buffer;
	unsigned short maxSize;
	//pthread_mutex_t mutex;
	void increaseWritePosition(){

		writePosition ++;

		if (writePosition == maxSize) {
			writePosition = 0;
		}

		if (readPosition == writePosition) {
			increaseReadPosition();
		}


	}
	void increaseReadPosition(){
		readPosition ++;
		if (readPosition == maxSize) {
			readPosition = 0;
		}

	}

	void decreaseWritePosition(){

		if(readPosition != writePosition){

			readPosition --;
			if (readPosition < 0) {
				readPosition = maxSize -1;
			}
		}
	}

public:
	unsigned short readPosition;
	unsigned short writePosition;

	Buffer(unsigned short _maxSize){

		this->maxSize=_maxSize;

		this->buffer = new T[maxSize];

		this->writePosition = 0;
		this->readPosition = 0;

	}

	~Buffer(){
		delete this->buffer;
	}

	void push(const T & t){
		T newT = T(t);
		//pthread_mutex_lock(&mutex);
		buffer[writePosition] = newT;
		increaseWritePosition();
		//pthread_mutex_unlock(&mutex);

	}

	T pull(){
		//pthread_mutex_lock(&mutex);
		T output;
		if(hasData()){
			output = buffer[readPosition];
			increaseReadPosition();
		} else {
			output = T();
		}
		//pthread_mutex_unlock(&mutex);


		return output;
	}

	T pullYoungest(){
		//pthread_mutex_lock(&mutex);
		T output;
		if(hasData()){
			decreaseWritePosition();
			output = buffer[writePosition];
		}
		else output = T();
		//pthread_mutex_unlock(&mutex);
		return output;
	}

	void clear(){
		//pthread_mutex_lock(&mutex);
		readPosition = writePosition;
		//pthread_mutex_unlock(&mutex);
	}

	unsigned short getMaxSize(){

		unsigned short output = maxSize;

		return output;
	}

	unsigned short getSize(){
		//pthread_mutex_lock(&mutex);

		unsigned short output = 0;


		if(readPosition < writePosition){

			output = writePosition - readPosition;

		} else if(readPosition > writePosition){

			output = maxSize - (readPosition - writePosition);

		}
		//pthread_mutex_unlock(&mutex);


		return output;
	}

	bool hasData(){
		//pthread_mutex_lock(&mutex);

		bool output = true;


		if(readPosition == writePosition) output = false;
		//pthread_mutex_unlock(&mutex);


		return output;

	}


	unsigned short getContent(T * t){
		//pthread_mutex_lock(&mutex);

		unsigned short newIndex = 0;

		unsigned short ringBufferIndex = readPosition;
		while(ringBufferIndex != writePosition){

			t[newIndex] = buffer[ringBufferIndex];

			newIndex++;
			ringBufferIndex ++;

			if (ringBufferIndex == maxSize) {
				ringBufferIndex = 0;
			}
		}
		//pthread_mutex_unlock(&mutex);

		unsigned short size = getSize();


		return size;
	}


};

} /* namespace fs */

#endif /* BUFFER_H_ */
