/*
 * Copyright (c) 2016 PolySync
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * \example HelloWorldSubscriber.cpp
 *
 * PolySync Hello World Subscriber C++ API example application
 *      Demonstrate how to subscribe a node to a message
 *
 */

#include <iostream>
#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>
#include <fstream>
#include "Buffer.h"
#include "Motion.h"
#include "tensorflow/core/public/session.h"

using namespace std;
using namespace tensorflow;


/**
 * @brief HellowWorldSubscriberNode class
 *
 * The HelloWorldSubscriberNode class exists to override the functions defined
 * in the base Node class.  The functions exist in the base class but are
 * stubbed out and must be overloaded in order for them to do something.  In
 * this instance the initStateEvent and the messageHandlerEvent are overloaded
 * to register for the messages and receive them, respectively.
 */
class RecorderNode : public polysync::Node
{
private:
    ps_msg_type _messageType;
    ps_msg_type _brakeType;
    ps_msg_type _throttleType;
    ps_msg_type _steerType;
    ps_msg_type _imageType;
    ofstream file = ofstream("output.txt", std::ios_base::app | std::ofstream::out);
    Buffer<polysync::datamodel::Motion> *motionBuffer = new Buffer<polysync::datamodel::Motion>(1);
    Buffer<DDS_double> *steerBuffer = new Buffer<DDS_double>(1);
    Buffer<DDS_double> *brakeBuffer = new Buffer<DDS_double>(1);
    Buffer<DDS_double> *throttleBuffer = new Buffer<DDS_double>(1);
    Buffer<polysync::datamodel::ImageDataMessage> *imageBuffer = new Buffer<polysync::datamodel::ImageDataMessage>(1);

public:
    /**
     * @brief initStateEvent
     *
     * Subscribe to a message that the publisher node will send.
     *
     * @param void
     * @return void
     */
    void initStateEvent() override
    {
        _messageType = getMessageTypeByName( "ps_platform_motion_msg" );
        _brakeType = getMessageTypeByName( "ps_platform_brake_report_msg" );
        _throttleType = getMessageTypeByName( "ps_platform_throttle_report_msg" );
        _steerType = getMessageTypeByName( "ps_platform_steering_report_msg" );
        _imageType = getMessageTypeByName("ps_image_data_msg");

        // Register as a listener for the message type that the publisher
        // is going to send.  Message types are defined in later tutorials.
        registerListener( _messageType );
        registerListener(_brakeType);
        registerListener(_throttleType);
        registerListener(_steerType);
        registerListener(_imageType);
        file << "path,heading,longitude,latitude,quarternion0,quarternion1,quarternion2"
        	 <<",quarternion3,vel0,vel1,vel2,steering,throttle,brake\n";
    }

    /**
     * @brief messageEvent
     *
     * Extract the information from the provided message
     *
     * @param std::shared_ptr< Message > - variable containing the message
     * @return void
     */
    virtual void messageEvent( std::shared_ptr< polysync::Message > message )
    {
		using namespace polysync::datamodel;
		if (std::shared_ptr<PlatformMotionMessage> incomingMessage =
				getSubclass<PlatformMotionMessage>(message)) {
			Motion mot = Motion::fromMotionMessage(incomingMessage);
			motionBuffer->push(mot);

			//incomingMessage->getMessageTimestamp();
		}
		if (std::shared_ptr<PlatformBrakeReportMessage> incomingMessage =
				getSubclass<PlatformBrakeReportMessage>(message)) {
			brakeBuffer->push(incomingMessage->getPedalCommand());

			//incomingMessage->getMessageTimestamp();
		}
		if (std::shared_ptr<PlatformThrottleReportMessage> incomingMessage =
				getSubclass<PlatformThrottleReportMessage>(message)) {
			throttleBuffer->push(incomingMessage->getPedalCommand());

			//incomingMessage->getMessageTimestamp();
		}
		if (std::shared_ptr<PlatformSteeringReportMessage> incomingMessage =
				getSubclass<PlatformSteeringReportMessage>(message)) {
			steerBuffer->push(incomingMessage->getSteeringWheelAngle());

			//incomingMessage->getMessageTimestamp();
		}
		if (std::shared_ptr<ImageDataMessage> incomingMessage =
				getSubclass<ImageDataMessage>(message)) {
			if (incomingMessage->getSensorDescriptor().getId() == 15395510) {
				FILE *stream;
				DDS_unsigned_long_long ts =
						incomingMessage->getHeaderTimestamp();
				std::string imageName = "IMG/"
						+ std::to_string(
								incomingMessage->getSensorDescriptor().getId())
						+ '-' + std::to_string(ts) + ".jpeg";
				const char * c = imageName.c_str();
				if ((stream = freopen(c, "w", stdout)) == NULL) {
					fprintf(stderr,
							"Can't create file: %s\nTry:\n\tmkdir IMG\n\n",
							imageName.c_str());
					fflush(stderr);
					exit(-1);
				}

				std::vector<DDS_octet> imageData =
						incomingMessage->getDataBuffer();
				for (int d : imageData) {
					printf("%c", d);
				}
				file
						<< imageName + "," + motionBuffer->pull().getCsvValues()
								+ "," + std::to_string(steerBuffer->pull())
								+ "," + std::to_string(throttleBuffer->pull())
								+ "," + std::to_string(brakeBuffer->pull())
								+ "\n";
			}
		}
	}

};

/**
 * @brief main
 *
 * Entry point for this tutorial application
 * The "connectPolySync" begins this node's PolySync execution loop.
 *
 * @return int - exit code
 */
int main()
{
    // Just testing
    Tensor a(DT_FLOAT, TensorShape());
    a.scalar<float>()() = 3.0;

    // Create an instance of the HelloWorldNode and connect it to PolySync
    RecorderNode subscriberNode;

    // When the node has been created, it will cause an initStateEvent to
    // to be sent.
    subscriberNode.connectPolySync();

    return 0;
}
