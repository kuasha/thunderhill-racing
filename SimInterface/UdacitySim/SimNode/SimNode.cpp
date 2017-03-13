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
 * \modified from example Publish.cpp
 *
 * Publish/Subscribe Example.
 *
 * Shows how to use publish/subscribe communication model, and the Messaging API
 * routines. This half of the example code populates and publishes a message
 * to the PolySync bus.
 *
 * The second half of the example - Subscribe.cpp - runs as a seperate node,
 * and subscribes to the messages to access the data buffer.
 *
 * The example uses the standard PolySync node template and state machine.
 * Send the SIGINT (control-C on the keyboard) signal to the node/process to do 
 * a graceful shutdown.
 *
 */

#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>
#include <PolySyncVideo.hpp>
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
typedef void (*simsensorrequestcallback)();
typedef void (*simcommandcallback)(double, double, double);

class SimNode : public polysync::Node
{
private:
    const string node_name = "sim-node-cpp";

    // messages published by simulator ---> to other nodes
    ps_msg_type _ps_image_data_msg;
    ps_msg_type _ps_platform_steering_report_msg;
    ps_msg_type _ps_platform_throttle_report_msg;
    ps_msg_type _ps_platform_brake_report_msg;
    ps_msg_type _ps_platform_motion_msg;

    // messages subscribed by simulator <--- from other nodes
    ps_msg_type _ps_platform_steering_command_msg;
    ps_msg_type _ps_platform_throttle_command_msg;
    ps_msg_type _ps_platform_brake_command_msg;

    // from simulator
    // last sensor measurements
    DDS_unsigned_long_long simts;

    double simSteering;
    double simThrottle;
    double simBraking;

    // GPS
    double simLat;
    double simLon;

    // orientation quaternion
    double simOrient1;
    double simOrient2;
    double simOrient3;
    double simOrient4;

    // heading
    double simHeading;

    // velocity
    double simVelocityX;
    double simVelocityY;
    double simVelocityZ;

    // Set the desired format for video device.
    // polysync::VideoFormat deviceFormat{
    //     PIXEL_FORMAT_MJPEG,
    //     320, 160,
    //     PSYNC_VIDEO_DEFAULT_FRAMES_PER_SECOND };

    // Set format for encoder output
    // polysync::VideoFormat encodedFormat{
    //     PIXEL_FORMAT_H264,
    //     320, 160,
    //     PSYNC_VIDEO_DEFAULT_FRAMES_PER_SECOND };

    // Set format for decoder output
    // polysync::VideoFormat decodedFormat{
    //     PIXEL_FORMAT_MJPEG,
    //     320, 160,
    //     PSYNC_VIDEO_DEFAULT_FRAMES_PER_SECOND };

    // Create encoder/decoder
    // polysync::VideoEncoder encoder{ deviceFormat, encodedFormat };
    // polysync::VideoDecoder decoder{ encodedFormat, decodedFormat };

    // image
    unsigned int imgWidth = 320;
    unsigned int imgHeight = 160;
    unsigned int imgDepth = 3;
    unsigned int imgSize = imgWidth * imgHeight * imgDepth;
    vector<unsigned char> simImgData;

    // to simulator
    // last commands
    DDS_unsigned_long_long comts;
    double comSteering;
    double comThrottle;
    double comBraking;

public:
    simcommandcallback acommandcallback = NULL;
    simsensorrequestcallback asensorcallback = NULL;

    SimNode()
    {
        setNodeType( PSYNC_NODE_TYPE_API_USER );
        setDomainID( PSYNC_DEFAULT_DOMAIN );
        setSDFID( PSYNC_SDF_ID_INVALID );
        setFlags( NODE_FLAGS_VALUE );
        setNodeName( node_name );
    }

    ~SimNode()
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
        // messages published by simulator ---> to other nodes
        _ps_image_data_msg = getMessageTypeByName( "ps_image_data_msg" );
        _ps_platform_steering_report_msg = getMessageTypeByName( "ps_platform_steering_report_msg" );
        _ps_platform_throttle_report_msg = getMessageTypeByName( "ps_platform_throttle_report_msg" );
        _ps_platform_brake_report_msg = getMessageTypeByName( "ps_platform_brake_report_msg" );
        _ps_platform_motion_msg = getMessageTypeByName( "ps_platform_motion_msg" );

        // messages subscribed by simulator <--- from other nodes
        _ps_platform_steering_command_msg = getMessageTypeByName( "ps_platform_steering_command_msg" );
        _ps_platform_throttle_command_msg = getMessageTypeByName( "ps_platform_throttle_command_msg" );
        _ps_platform_brake_command_msg = getMessageTypeByName( "ps_platform_brake_command_msg" );

        // Register as a listener for the message type that the publisher
        // is going to send.  Message types are defined in later tutorials.
        registerListener( _ps_platform_steering_command_msg );
        registerListener( _ps_platform_throttle_command_msg );
        registerListener( _ps_platform_brake_command_msg );
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
        if (asensorcallback != NULL)
            asensorcallback();

        // use the same timestamp
        simts = polysync::getTimestamp();

        // Create a PlatformSteeringReportMessage message
        polysync::datamodel::PlatformSteeringReportMessage steeringMsg( *this );

        // Set message data
        steeringMsg.setHeaderTimestamp( simts );
        steeringMsg.setSteeringWheelAngle( comSteering );

        // Publish to the PolySync bus
        steeringMsg.publish();

        // Create a PlatformThrottleReportMessage message
        polysync::datamodel::PlatformThrottleReportMessage throttleMsg( *this );

        // Set message data
        throttleMsg.setHeaderTimestamp( simts );
        throttleMsg.setPedalOutput( comThrottle );

        // Publish to the PolySync bus
        throttleMsg.publish();

        // Create a PlatformBrakeReportMessage message
        polysync::datamodel::PlatformBrakeReportMessage brakingMsg( *this );

        // Set message data
        brakingMsg.setHeaderTimestamp( simts );
        brakingMsg.setPedalOutput( comBraking );

        // Publish to the PolySync bus
        brakingMsg.publish();

        std::array< DDS_double, 4 > orient;
        orient[0] = simOrient1;
        orient[1] = simOrient2;
        orient[2] = simOrient3;
        orient[3] = simOrient4;
        std::array< DDS_double, 3 > velocity;
        velocity[0] = simVelocityX;
        velocity[1] = simVelocityY;
        velocity[2] = simVelocityZ;

        // Create a Platform Motion message
        polysync::datamodel::PlatformMotionMessage motionMsg( *this );

        // Set message data
        motionMsg.setHeaderTimestamp( simts );
        motionMsg.setLatitude( simLat );
        motionMsg.setLongitude( simLon );
        motionMsg.setOrientation( orient );
        motionMsg.setHeading( simHeading );
        motionMsg.setVelocity( velocity );

        // Publish to the PolySync bus
        motionMsg.publish();

        // Create an Image Data message
        polysync::datamodel::ImageDataMessage imagedataMsg( *this );
        imagedataMsg.setPixelFormat( PIXEL_FORMAT_MJPEG );
        imagedataMsg.setWidth( imgWidth );
        imagedataMsg.setHeight( imgHeight );
        imagedataMsg.setDataBuffer( simImgData );

        // Publish to the PolySync bus
        imagedataMsg.publish();

        // The ok state is called periodically by the system, sleep to reduce
        // the number of messages sent
        // polysync::sleepMicro( 500000 );
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

        if( std::shared_ptr <PlatformSteeringCommandMessage>
            incomingMessage = getSubclass< PlatformSteeringCommandMessage >( message ) )
        {
            comts = incomingMessage->getHeaderTimestamp(); 
            comSteering = incomingMessage->getSteeringWheelAngle();
        }

        if( std::shared_ptr <PlatformThrottleCommandMessage>
            incomingMessage = getSubclass< PlatformThrottleCommandMessage >( message ) )
        {
            comts = incomingMessage->getHeaderTimestamp();
            comThrottle = incomingMessage->getThrottleCommand();
        }

        if( std::shared_ptr <PlatformBrakeCommandMessage>
            incomingMessage = getSubclass< PlatformBrakeCommandMessage >( message ) )
        {
            comts = incomingMessage->getHeaderTimestamp();
            comBraking = incomingMessage->getBrakeCommand();
        }

        if (acommandcallback != NULL)
            acommandcallback(comSteering, comThrottle, comBraking);
    }

    void setSimulatorValues( double steering, double throttle, double braking, double lat, double lon, double orient1, double orient2, double orient3, double orient4, double heading, double velx, double vely, double velz)
    {
        simSteering = steering;
        simThrottle = throttle;
        simBraking = braking;
    
        // lat lon
        simLat = lat;
        simLon = lon;

        // orientation quaternion
        simOrient1 = orient1;
        simOrient2 = orient2;
        simOrient3 = orient3;
        simOrient4 = orient4;
    
        // heading
        simHeading = heading;
       
        // velocity
        simVelocityX = velx;
        simVelocityY = vely;
        simVelocityZ = velz;
    }

    void setSimulatorImage( int width, int height, int depth, unsigned int size, unsigned char *buffer)
    {
        if (size > 0)
        {
            imgWidth = width;
            imgHeight = height;
            imgDepth = depth;
            imgSize = size;
            if (simImgData.size() < imgSize) {
                simImgData = vector<unsigned char> (imgSize);
            }
            unsigned char *p = simImgData.data();
            unsigned char *q = buffer;
            for (unsigned int i=0; i<imgSize; i++)
            {
                *p = *q;
                ++p;
                ++q;
            }
        }
    }
};

extern "C" {
    SimNode* SimNode_new(){ return new SimNode(); }
    void SimNode_connectPolySync(SimNode* node){ node->connectPolySync(); }
    void SimNode_setRequestSensorCallback(SimNode* node, simsensorrequestcallback asensorcallback){ node->asensorcallback = asensorcallback; }
    void SimNode_setCommandCallback(SimNode* node, simcommandcallback acommandcallback){ node->acommandcallback = acommandcallback; }
    void SimNode_sendSensorValues(SimNode* node, double steering, double throttle, double braking, double lat, double lon, double orient1, double orient2, double orient3, double orient4, double heading, double velx, double vely, double velz) { node->setSimulatorValues( steering, throttle, braking, lat, lon, orient1, orient2, orient3, orient4, heading, velx, vely, velz); }
    void SimNode_sendSensorImage(SimNode* node, int width, int height, int depth, int size, unsigned char *buffer) { node->setSimulatorImage( width, height, depth, size, buffer ); }
}

