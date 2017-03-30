/*
 * Motion.cpp
 *
 *  Created on: Mar 7, 2017
 *      Author: jjordening
 */

#include "Motion.h"


namespace polysync{
namespace datamodel{

Motion::Motion(DDS_unsigned_long_long timestamp,
		double heading, double longitude, double latitude,
		std::array< DDS_double, 3 > vel, std::array< DDS_double, 4 > orientation) {
	this->timestamp = timestamp;
	this->heading = heading;
	this->vel = vel;
	this->longitude = longitude;
	this->latitude = latitude;
	this->orientation = orientation;
}

Motion::Motion(const Motion & other) : timestamp(other.timestamp),
		heading(other.heading), longitude(other.longitude), latitude(other.latitude),
		vel(other.vel), orientation(other.orientation) {
}

Motion::Motion() : timestamp(0), heading(0), longitude(0), latitude(0){
	this->vel = {0,0,0};
	this->orientation = {0,0,0,0};
}

int Motion::print(std::ofstream & file) {
	std::string dataString = std::to_string(this->heading)+","+std::to_string(this->longitude)+","
			+std::to_string(this->latitude)+","
			+","+std::to_string(this->orientation[0])+","+std::to_string(this->orientation[1])
			+","+std::to_string(this->orientation[2])+","+std::to_string(this->orientation[3])
			+std::to_string(this->vel[0])+","+std::to_string(this->vel[1])+","+
			std::to_string(this->vel[2]);
	file << dataString;
	return 1;

}

std::string Motion::getCsvValues() {
  char txt[200];
  sprintf(txt,",%.20f,%.20f,",this->longitude,this->latitude);
	std::string dataString = std::to_string(this->heading)+
      txt
      // ","+std::to_string(this->longitude)+","+std::to_string(this->latitude)+","
			+std::to_string(this->orientation[0])+","+std::to_string(this->orientation[1])
			+","+std::to_string(this->orientation[2])+","+std::to_string(this->orientation[3])+","
			+std::to_string(this->vel[0])+","+std::to_string(this->vel[1])+","+
			std::to_string(this->vel[2]);
	return dataString;

}

Motion Motion::fromMotionMessage(std::shared_ptr<PlatformMotionMessage> message) {

	Motion mot = Motion(message->getHeaderTimestamp(), message->getHeading(),
			message->getLongitude(), message->getLatitude(),
			message->getVelocity(), message->getOrientation());
    return mot;


}

} //namespace datamodel
} //namespace polysync
