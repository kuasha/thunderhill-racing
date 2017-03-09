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
	double heading;
	double longitude;
	double latitude;
	std::array< DDS_double, 3 > vel;
	std::array< DDS_double, 4 > orientation;

public:
	Motion(DDS_unsigned_long_long timestamp,
			double heading, double longitude, double latitude,
			std::array< DDS_double, 3 > vel, std::array< DDS_double, 4 > orientation);
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
		return asin((2 * this->orientation[0] * this->orientation[1])
				+ (2 * this->orientation[2] * this->orientation[3]))  * 180.0 / PI;
	}

	double getLatitude() const {
		return latitude;
	}

	double getLongitude() const {
		return longitude;
	}

	std::array<DDS_double, 4> getOrientation() const {
		return orientation;
	}
};


} //namespace datamodel
} //namespace polysync


#endif /* THUNDERHILL_RACING_HELLOWORLDSUBSCRIBER_MOTION_H_ */
