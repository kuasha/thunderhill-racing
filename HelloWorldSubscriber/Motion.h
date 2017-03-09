/*
 * MotionStruct.h
 *
 *  Created on: Mar 7, 2017
 *      Author: jjordening
 */

#ifndef THUNDERHILL_RACING_HELLOWORLDSUBSCRIBER_MOTION_H_
#define THUNDERHILL_RACING_HELLOWORLDSUBSCRIBER_MOTION_H_
#include <PolySyncDataModel.hpp>
#include <math.h>
#include <stdio.h>
#include <fstream>
#include <string>


#define PI 3.14159265




namespace polysync {
namespace datamodel{

class Motion{
private:
	DDS_unsigned_long_long timestamp;
	double yaw;
	double heading;
	std::array< DDS_double, 3 > vel;

public:
	Motion(DDS_unsigned_long_long timestamp, double yaw,
			double heading, std::array< DDS_double, 3 > vel);
	Motion(const Motion & other);
	Motion();
	static Motion fromMotionMessage(std::shared_ptr<PlatformMotionMessage> message);

	int print(std::ofstream & file);
	std::string getCsvValues();
	double getHeading() const {
		return heading;
	}

	DDS_unsigned_long_long getTimestamp() const {
		return timestamp;
	}

	std::array<DDS_double, 3> getVel() const {
		return vel;
	}

	double getYaw() const {
		return yaw;
	}
};


} //namespace datamodel
} //namespace polysync


#endif /* THUNDERHILL_RACING_HELLOWORLDSUBSCRIBER_MOTION_H_ */
