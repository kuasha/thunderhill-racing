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
 * \example PublishSubscribe.cpp
 *
 * Publish/Subscribe Example.
 *
 * Shows how to use publish/subscribe routines.
 * The example code registers a subscriber to PolySync diagnostic messages and
 * publishes event messages.
 *
 * The example uses the standard PolySync node template and state machine.
 * Send the SIGINT (control-C on the keyboard) signal to the node/process to do 
 * a graceful shutdown.
 *
 */

#include <iostream>
#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>

using namespace std;

/**
 * @brief Node flags to be OR'd with driver/interface flags.
 *
 * Provided by the compiler so PolySync can add build-specifics as needed.
 *
 */
#ifndef NODE_FLAGS_VALUE
#define NODE_FLAGS_VALUE (0)
#endif
#ifndef ENABLE_CONTROLS_COMMAND_ID
#define ENABLE_CONTROLS_COMMAND_ID (5001)
#endif

/**
 * @brief PublisherSubscriberNode class
 *
 * The PublisherSubscriberNode class exists to override the functions defined in the
 * base class. The functions exist in the base class but are stubbed out
 * and must be overloaded in order for them to do something. In this instance
 * we are overriding the main functions for sending and receiving messages as 
 * well as responding to error and warning type events.
 * 
 */
class TestNode : public polysync::Node
{
private:
    const string node_name = "test-node-cpp";

    ps_msg_type _brakeReport;
    ps_msg_type _steeringReport;
    ps_msg_type _throttleReport;

    unsigned long long steeringID = 0;
    unsigned long long brakeID = 0;
    unsigned long long throttleID = 0;
    int initialised = 0;

    float angle1 = -6.; 
    float angle2 = 6.;
    float throttle_val = 0.1;
    float break_val = 0.7;

    int sleeping_interval = 10000;
    
public:
    
    TestNode()
    {
        setNodeType( PSYNC_NODE_TYPE_API_USER );
        setDomainID( PSYNC_DEFAULT_DOMAIN );
        setSDFID( PSYNC_SDF_ID_INVALID );
        setFlags( NODE_FLAGS_VALUE | PSYNC_INIT_FLAG_STDOUT_LOGGING );
        setNodeName( node_name );
    }
    
    ~TestNode()
    {
        
    }

    void initStateEvent() override
    {
        _steeringReport = getMessageTypeByName( "ps_platform_steering_report_msg" );
        registerListener( _steeringReport );
         _brakeReport = getMessageTypeByName( "ps_platform_brake_report_msg" );
        registerListener( _brakeReport );
         _throttleReport = getMessageTypeByName( "ps_platform_throttle_report_msg" );
        registerListener( _throttleReport );
    }
    
    void releaseStateEvent() override
    {
        // do nothing, sleep for 10 milliseconds
        polysync::sleepMicro( 10000 );
    }
    
    void errorStateEvent() override
    {
        // do nothing, sleep for 10 milliseconds
        polysync::sleepMicro( 10000 );
    }
    
    void fatalStateEvent() override
    {
        // do nothing, sleep for 10 milliseconds
        polysync::sleepMicro( 10000 );
    }
    
    void warnStateEvent() override
    {
        // do nothing, sleep for 10 milliseconds
        polysync::sleepMicro( 10000 );
    }
    
    /**
     * @brief okStateEvent
     *
     * Override the base class functionality to send messages when the node
     * reaches the "ok" state. This is the state where the node is in its
     * normal operating mode.
     * 
     * @param void
     * @return void
     */
    void okStateEvent() override
    {   
	//cout << brakeID << "  " << initialised << endl;
	fflush(stdout);
	if(initialised == 0 && brakeID != 0) {
	printf("In here\n");
	fflush(stdout);
	    polysync::datamodel::CommandMessage cmdMsg( *this);
	    cmdMsg.setId(ENABLE_CONTROLS_COMMAND_ID);
	    cmdMsg.setDestGuid(brakeID);
            cmdMsg.setHeaderTimestamp( polysync::getTimestamp() );
            cmdMsg.setTimestamp( polysync::getTimestamp() );
	    cmdMsg.publish();
	    initialised = 1;
	}
	if(initialised != 0) {

            steerCommand(angle1);

            throttleCommand(throttle_val);

            brakeCommand(break_val);
	    pause();
	}
    }

    void pause()
    {
        polysync::sleepMicro( sleeping_interval );
    }

    void steerCommand(float angle)
    {
        if (steeringID)
        {
            polysync::datamodel::PlatformSteeringCommandMessage message( *this);
            message.setDestGuid(steeringID);
            message.setTimestamp( polysync::getTimestamp() );
            message.setSteeringWheelAngle(angle);
            message.setMaxSteeringWheelRotationRate(M_PI_2);
            message.setHeaderTimestamp( polysync::getTimestamp() );
	    message.setSteeringCommandKind(STEERING_COMMAND_ANGLE);
	    message.setEnabled(1);
            message.publish();
            //message.print();
        }
        
    }

    void brakeCommand(float value)
    {
        if (brakeID != 0) 
        {
            polysync::datamodel::PlatformBrakeCommandMessage message( *this);
            message.setDestGuid(brakeID);
            message.setTimestamp( polysync::getTimestamp() );
            message.setBrakeCommand(value);
            message.setHeaderTimestamp( polysync::getTimestamp() );
	    message.setBrakeCommandType(BRAKE_COMMAND_PEDAL);
	    message.setEnabled(1);
            message.publish();
            //message.print();
        }
        
    }

    void throttleCommand(float value)
    {
        if (throttleID != 0)
        {
            polysync::datamodel::PlatformThrottleCommandMessage message( *this);
            message.setDestGuid(throttleID);
            message.setTimestamp( polysync::getTimestamp() );
            message.setThrottleCommand(value);
            message.setHeaderTimestamp( polysync::getTimestamp() );
	    message.setThrottleCommandType(THROTTLE_COMMAND_PEDAL);
	    message.setEnabled(1);
            message.publish();
            //message.print();
        }
        
    }
    
    /**
     * @brief messageEvent
     * 
     * Extract the information from the provided message
     * 
     * @param std::shared_ptr< Message > - variable containing the message
     * @return void
     */
    void messageEvent( std::shared_ptr< polysync::Message > message ) override
    {
        using namespace polysync::datamodel;
        if (std::shared_ptr < PlatformSteeringReportMessage > incomingMessage = getSubclass < PlatformSteeringReportMessage > (message))
        {
            if (steeringID == 0)
            {
                steeringID = incomingMessage->getHeaderSrcGuid();
		incomingMessage->print();
            }
        }

        if (std::shared_ptr < PlatformBrakeReportMessage > incomingMessage = getSubclass < PlatformBrakeReportMessage > (message))
        {
            if (brakeID == 0)
            {
                brakeID = incomingMessage->getHeaderSrcGuid();
		incomingMessage->print();
            }
        }

        if (std::shared_ptr < PlatformThrottleReportMessage > incomingMessage = getSubclass < PlatformThrottleReportMessage > (message))
        {
            if (throttleID == 0)
            {
                throttleID = incomingMessage->getHeaderSrcGuid();
		incomingMessage->print();
            }
        }
    }
    
};


/**
 * @brief main
 *
 * Entry point for this tutorial application
 * The "connectToPolySync" is a blocking call, users must use Ctrl-C to exit
 * this function.
 *
 * @return int - exit code
 */
int main()
{
    // Create an instance of the PublisherSubscriberNode and connect it to 
    // PolySync.
    TestNode testNode;

    // When the node has been created, it will cause an initStateEvent to be
    // sent and then proceed into the okState.  connectToPolySync does not
    // return, use Ctrl-C to exit.
    testNode.connectPolySync();

    return 0;
}
