/*
 * Buffer.h
 *
 *
 *  Created on: 06.03.2016
 */

#ifndef BUFFER_H_
#define BUFFER_H_


#include "stdlib.h"
#include <mutex>
namespace std {

/*
	This class provides a thread safe RingBuffer for 
	storing and retrieveing elements using a FIFO rule.
*/
template<class T> class Buffer{

private:

	T * buffer;
	unsigned short maxSize;
	mutex mtx;
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
		mtx.lock();
		buffer[writePosition] = newT;
		increaseWritePosition();
		mtx.unlock();

	}

	T pull(){
		T output;
		if(hasData()){
			mtx.lock();
			output = buffer[readPosition];
			increaseReadPosition();
			mtx.unlock();
		} else {
			output = T();
		}


		return output;
	}

	T pullYoungest(){
		T output;
		if(hasData()){
			mtx.lock();
			decreaseWritePosition();
			output = buffer[writePosition];
			mtx.unlock();
		}
		else output = T();
		return output;
	}

	void clear(){
		mtx.lock();
		readPosition = writePosition;
		mtx.unlock();
	}

	unsigned short getMaxSize(){

		unsigned short output = maxSize;

		return output;
	}

	unsigned short getSize(){

		unsigned short output = 0;
		mtx.lock();


		if(readPosition < writePosition){

			output = writePosition - readPosition;

		} else if(readPosition > writePosition){

			output = maxSize - (readPosition - writePosition);

		}
		mtx.unlock();


		return output;
	}

	bool hasData(){

		bool output = true;
		mtx.lock();


		if(readPosition == writePosition) output = false;
		mtx.unlock();


		return output;

	}


	unsigned short getContent(T * t){
		unsigned short newIndex = 0;
		mtx.lock();


		unsigned short ringBufferIndex = readPosition;
		while(ringBufferIndex != writePosition){

			t[newIndex] = buffer[ringBufferIndex];

			newIndex++;
			ringBufferIndex ++;

			if (ringBufferIndex == maxSize) {
				ringBufferIndex = 0;
			}
		}
		mtx.unlock();


		unsigned short size = getSize();


		return size;
	}


};

} /* namespace fs */

#endif /* BUFFER_H_ */
