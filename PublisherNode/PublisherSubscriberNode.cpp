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
 * \example HelloWorldPublisher.cpp
 *
 * PolySync Hello World Publisher C++ API example application
 *      Demonstrate how to publish a message from a node
 *
 * Node Class definition and how to create your own Nodes
 */

#include <iostream>
#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>

using namespace std;

/**
 * @brief HellowWorldPublisherNode class
 *
 * The HelloWorldNode class exists to override the functions defined in the
 * base class.  The functions exist in the base class but are stubbed out
 * and must be overloaded in order for them to do something. In this instance
 * only the okStateEvent function is overloaded to publish a message that the
 * subscriber can receive.
 */
class PublisherSubscriberNode : public polysync::Node
{

private:
	const string node_name = "polysync-publish-cpp";
	const string platform_motion_msg = "ps_platform_motion_msg";
	const string platform_brake_msg = "ps_platform_brake_report_msg";
	const string platform_throttle_msg = "ps_platform_throttle_report_msg";
	const string platform_steer_msg = "ps_platform_steering_report_msg";
	const string platform_image_msg = "ps_image_data_msg";

	ps_msg_type _motionMessageType;
	ps_msg_type _brakeMessageType;
	ps_msg_type _throttleMessageType;
	ps_msg_type _steerMessageType;
	ps_msg_type _imageMessageType;

public:
	PublisherSubscriberNode()
	{
		setNodeType(PSYNC_NODE_TYPE_API_USER);
		setDomainID(PSYNC_DEFAULT_DOMAIN);
		setSDFID(PSYNC_SDF_ID_INVALID);
		setFlags(PSYNC_INIT_FLAG_STDOUT_LOGGING);
		setNodeName(node_name);
	}

	~PublisherSubscriberNode()
	{

	}

	void initStateEvent() override
	{
		_motionMessageType = getMessageTypeByName(platform_motion_msg);
		_brakeMessageType = getMessageTypeByName(platform_brake_msg);
		_throttleMessageType = getMessageTypeByName(platform_throttle_msg);
		_steerMessageType = getMessageTypeByName(platform_steer_msg);
		_imageMessageType = getMessageTypeByName(platform_image_msg);

	}

	void releaseStateEvent() override
	{
		// do nothing, sleep for 10 milliseconds
		polysync::sleepMicro(10000);
	}

	void errorStateEvent() override
	{
		// do nothing, sleep for 10 milliseconds
		polysync::sleepMicro(10000);
	}

	void fatalStateEvent() override
	{
		// do nothing, sleep for 10 milliseconds
		polysync::sleepMicro(10000);
	}

	void warnStateEvent() override
	{
		// do nothing, sleep for 10 milliseconds
		polysync::sleepMicro(10000);
	}

	void okStateEvent() override
	{
		// Create a message
		polysync::datamodel::PlatformMotionMessage message(*this);

		// Set the message timestamp (UTC), representing when the data was
		// received or when it originated
		message.setTimestamp(polysync::getTimestamp());

		message.setLatitude(45.515289);

		message.setLongitude(-122.654355);

		// Set the message header timestamp, representing when the message
		// was published to the PolySync bus
		message.setHeaderTimestamp(polysync::getTimestamp());

		// Publish the message to the PolySync bus
		message.publish();

		// The ok state is called periodically by the system, sleep to reduce
		// the number of messages sent
		polysync::sleepMicro(1000000);

		// Throttle Command message
		polysync::datamodel::PlatformThrottleCommandMessage::PlatformThrottleCommandMessage throttleMessage(*this);

		// Brake Command Message
		polysync::datamodel::PlatformBrakeCommandMessage::PlatformBrakeCommandMessage brakeMessage(*this);
	}
};
int main()
{
	// Create an instance of the PublisherSubscriberNode and connect it to 
	// PolySync
	PublisherSubscriberNode publisherNode;

	// When the node has been created, it will cause an initStateEvent to be
	// sent and then proceed into the okState.  connectToPolySync does not
	// return, use Ctrl-C to exit
	publisherNode.connectPolySync();

	return 0;
}
  