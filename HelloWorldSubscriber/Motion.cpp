/*
 * Motion.cpp
 *
 *  Created on: Mar 7, 2017
 *      Author: jjordening
 */

#include "Motion.h"


namespace polysync{
namespace datamodel{

Motion::Motion(DDS_unsigned_long_long timestamp, double yaw,
		double heading, std::array< DDS_double, 3 > vel) {
	this->timestamp = timestamp;
	this->yaw = yaw;
	this->heading = heading;
	this->vel = vel;
}

Motion::Motion(const Motion & other) : timestamp(other.timestamp),
		yaw(other.yaw), heading(other.heading), vel(other.vel){
}

Motion::Motion() : timestamp(0), yaw(0), heading(0){
	this->vel = {0,0,0};
}

int Motion::print(std::ofstream & file) {
	std::string dataString = std::to_string(this->timestamp)+","+std::to_string(this->yaw) +","+
			std::to_string(this->heading)+","+std::to_string(this->vel[0])+","+std::to_string(this->vel[1])+","+
			std::to_string(this->vel[2])+"\n";
	file << dataString;
	return 1;

}
Motion Motion::fromMotionMessage(std::shared_ptr<PlatformMotionMessage> message) {

    std::array< DDS_double, 4 > orient = message->getOrientation();
    double yaw = asin((2 * orient[0] * orient[1]) + (2 * orient[2] * orient[3]))  * 180.0 / PI;

	Motion mot = Motion(message->getHeaderTimestamp(), yaw, message->getHeading(), message->getVelocity());
    return mot;


}

} //namespace datamodel
} //namespace polysync
