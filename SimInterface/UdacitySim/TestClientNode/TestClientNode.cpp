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

#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>
#include <stdio.h>
#include <string>
#include <math.h>

#define PI 3.14159265

using namespace std;

#ifndef NODE_FLAGS_VALUE
#define NODE_FLAGS_VALUE (0)
#endif

/**
 * @brief HellowWorldSubscriberNode class
 *
 * The HelloWorldSubscriberNode class exists to override the functions defined
 * in the base Node class.  The functions exist in the base class but are
 * stubbed out and must be overloaded in order for them to do something.  In
 * this instance the initStateEvent and the messageHandlerEvent are overloaded
 * to register for the messages and receive them, respectively.
 */
typedef void (*sensorcallback)(
  double, double, double,           // steering, throttle and braking
  double, double,                   // lat, lon
  double, double,                   // yaw, heading
  double, double, double,           // vel (x, y, z)
  int, unsigned char *);            // encoded size and JPEG encoded image

class TestClientNode : public polysync::Node
{
private:
    const string node_name = "sim-test-client-cpp";

    // messages subscribed by client <--- from other nodes
    ps_msg_type _ps_image_data_msg;
    ps_msg_type _ps_platform_steering_report_msg;
    ps_msg_type _ps_platform_throttle_report_msg;
    ps_msg_type _ps_platform_brake_report_msg;
    ps_msg_type _ps_platform_motion_msg;

    // messages published by client ---> to other nodes
    ps_msg_type _ps_platform_steering_command_msg;
    ps_msg_type _ps_platform_throttle_command_msg;
    ps_msg_type _ps_platform_brake_command_msg;

    // from simulator
    // last sensor measurements
    DDS_unsigned_long_long simts;

    std::vector <unsigned char> simImageData;
    unsigned int simImageSize;
    double simSteering;
    double simThrottle;
    double simBraking;

    // GPS
    double simLat;
    double simLon;

    // yaw - calculated from orientation quaternion
    double simYaw;

    // heading
    double simHeading;

    // velocity
    double simVelocityX;
    double simVelocityY;
    double simVelocityZ;

    // to simulator
    // last commands
    DDS_unsigned_long_long comts;
    double comSteering;
    double comThrottle;
    double comBraking;

public:
    sensorcallback asensorcallback = NULL;

    TestClientNode()
    {
        setNodeType( PSYNC_NODE_TYPE_API_USER );
        setDomainID( PSYNC_DEFAULT_DOMAIN );
        setSDFID( PSYNC_SDF_ID_INVALID );
        setFlags( NODE_FLAGS_VALUE );
        setNodeName( node_name );
    }

    ~TestClientNode()
    {

    }

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
        // messages published by simulator ---> to other nodes (subscribed by test client)
        _ps_image_data_msg = getMessageTypeByName( "ps_image_data_msg" );
        _ps_platform_steering_report_msg = getMessageTypeByName( "ps_platform_steering_report_msg" );
        _ps_platform_throttle_report_msg = getMessageTypeByName( "ps_platform_throttle_report_msg" );
        _ps_platform_brake_report_msg = getMessageTypeByName( "ps_platform_brake_report_msg" );
        _ps_platform_motion_msg = getMessageTypeByName( "ps_platform_motion_msg" );

        // messages subscribed by simulator <--- from other nodes (published by test client)
        _ps_platform_steering_command_msg = getMessageTypeByName( "ps_platform_steering_command_msg" );
        _ps_platform_throttle_command_msg = getMessageTypeByName( "ps_platform_throttle_command_msg" );
        _ps_platform_brake_command_msg = getMessageTypeByName( "ps_platform_brake_command_msg" );

        // Register as a listener for the message type that the publisher
        // is going to send.  Message types are defined in later tutorials.
        registerListener( _ps_image_data_msg );
        registerListener( _ps_platform_steering_report_msg );
        registerListener( _ps_platform_throttle_report_msg );
        registerListener( _ps_platform_brake_report_msg );
        registerListener( _ps_platform_motion_msg );
    }

    void releaseStateEvent() override
    {
        // do nothing
    }

    void errorStateEvent() override
    {
        // do nothing
    }

    void fatalStateEvent() override
    {
        // do nothing
    }

    void warnStateEvent() override
    {
        // do nothing
    }

    /**
     * Override the base class functionality to send messages when the node
     * reaches the "ok" state. This is the state where the node is in its
     * normal operating mode.
     */
    void okStateEvent() override
    {
        // Use the same timestamp
        DDS_unsigned_long_long ts = polysync::getTimestamp();

        // Create a PlatformSteeringCommandMessage message
        polysync::datamodel::PlatformSteeringCommandMessage steeringMsg( *this );

        // Set message data
        steeringMsg.setHeaderTimestamp( ts );
        steeringMsg.setSteeringWheelAngle( comSteering );

        // Publish to the PolySync bus
        steeringMsg.publish();

        // Create a PlatformThrottleCommandMessage message
        polysync::datamodel::PlatformThrottleCommandMessage throttleMsg( *this );

        // Set message data
        throttleMsg.setHeaderTimestamp( ts );
        throttleMsg.setThrottleCommand( comThrottle );

        // Publish to the PolySync bus
        throttleMsg.publish();

        // Create a PlatformBrakeCommandMessage message
        polysync::datamodel::PlatformBrakeCommandMessage brakingMsg( *this );

        // Set message data
        brakingMsg.setHeaderTimestamp( ts );
        brakingMsg.setBrakeCommand( comBraking );

        // Publish to the PolySync bus
        brakingMsg.publish();

        // The ok state is called periodically by the system, sleep to reduce
        // the number of messages sent
        polysync::sleepMicro( 50000 );
    }

    /**
     * @brief messageEvent
     *
     * Extract the information from the provided message and call pre-set python callbacks
     *
     * @param std::shared_ptr< Message > - variable containing the message
     * @return void
     */
    virtual void messageEvent( std::shared_ptr< polysync::Message > message )
    {
        using namespace polysync::datamodel;

        if( std::shared_ptr <PlatformSteeringReportMessage>
            incomingMessage = getSubclass< PlatformSteeringReportMessage >( message ) )
        {
            simts = incomingMessage->getHeaderTimestamp();
            simSteering = incomingMessage->getSteeringWheelAngle();
        }

        if( std::shared_ptr <PlatformThrottleReportMessage>
            incomingMessage = getSubclass< PlatformThrottleReportMessage >( message ) )
        {
            simts = incomingMessage->getHeaderTimestamp();
            simThrottle = incomingMessage->getPedalOutput();
        }

        if( std::shared_ptr <PlatformBrakeReportMessage>
            incomingMessage = getSubclass< PlatformBrakeReportMessage >( message ) )
        {
            simts = incomingMessage->getHeaderTimestamp();
            simBraking = incomingMessage->getPedalOutput();
        }

        if( std::shared_ptr <PlatformMotionMessage>
            incomingMessage = getSubclass< PlatformMotionMessage >( message ) )
        {
            simts = incomingMessage->getHeaderTimestamp();

            simLat = incomingMessage->getLatitude();
            simLon = incomingMessage->getLongitude();

            //to find yaw, use orientation quaternion
            //yaw  =  Mathf.Asin(2*x*y + 2*z*w);
            std::array< DDS_double, 4 > orient = incomingMessage->getOrientation();
            simYaw = asin((2 * orient[0] * orient[1]) + (2 * orient[2] * orient[3]))  * 180.0 / PI;

            //heading
            simHeading = incomingMessage->getHeading();

            // velocity (x, y, z)
            std::array< DDS_double, 3 > vel = incomingMessage->getVelocity();
            simVelocityX = vel[0];
            simVelocityY = vel[1];
            simVelocityZ = vel[2];
        }

        if (std::shared_ptr < ImageDataMessage >
            incomingMessage = getSubclass < ImageDataMessage > (message))
        {
            simts = incomingMessage->getHeaderTimestamp();
            if (incomingMessage->getPixelFormat() == PIXEL_FORMAT_MJPEG)
            {
                vector <unsigned char> image = incomingMessage->getDataBuffer();
                simImageSize = image.size();
                if (simImageSize > simImageData.size())
                {
                    simImageData = vector<unsigned char> (simImageSize);
                }
                unsigned char *p = simImageData.data();
                unsigned char *q = image.data();
                for (unsigned int i=0; i<simImageSize; i++)
                {
                    *p = *q;
                    ++p;
                    ++q;
                }

                //ts, yaw, heading, velocity (x, y, z)
                if (asensorcallback != NULL)
                    asensorcallback(simSteering, simThrottle, simBraking, simLat, simLon, simYaw, simHeading, simVelocityX, simVelocityY, simVelocityZ, simImageData.size(), simImageData.data());
            }
        }
    }

    void setCommand( double steering, double throttle, double braking )
    {
        comSteering = steering;
        comThrottle = throttle;
        comBraking = braking;
    }
};

extern "C" {
    TestClientNode* TestClientNode_new(){ return new TestClientNode(); }
    void TestClientNode_connectPolySync(TestClientNode* node){ node->connectPolySync(); }
    void TestClientNode_setSensorCallback(TestClientNode* node, sensorcallback asensorcallback){ node->asensorcallback = asensorcallback; }
    void TestClientNode_sendCommand(TestClientNode* node, double steering, double throttle, double braking){ node->setCommand(steering, throttle, braking); }
}

