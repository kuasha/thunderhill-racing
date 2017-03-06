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
class HelloWorldSubscriberNode : public polysync::Node
{
private:
    ps_msg_type _messageType;
    ps_msg_type _imageType;
    ofstream file = ofstream("../output.txt", std::ios_base::app | std::ofstream::out);
    Buffer<polysync::datamodel::PlatformMotionMessage> motionBuffer = Buffer<polysync::datamodel::PlatformMotionMessage>(1);
    Buffer<polysync::datamodel::ImageDataMessage> imageBuffer = Buffer<polysync::datamodel::ImageDataMessage>(1);

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
        _imageType = getMessageTypeByName("ps_image_data_msg");

        // Register as a listener for the message type that the publisher
        // is going to send.  Message types are defined in later tutorials.
        registerListener( _messageType );
        registerListener(_imageType);
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
        if( std::shared_ptr <PlatformMotionMessage> incomingMessage = getSubclass< PlatformMotionMessage >( message ) )
        {
			std::cout << "Motion message" << std::endl;
	    	motionBuffer.push(*incomingMessage.get());
			std::cout << "Motion written to buffer" << std::endl;
            incomingMessage->print(file);

	    //incomingMessage->getMessageTimestamp();
        }
        if (std::shared_ptr < ImageDataMessage > incomingMessage = getSubclass < ImageDataMessage > (message))
        {
			std::cout << "Image Message" << std::endl;
	    	imageBuffer.push(*incomingMessage.get());
			std::cout << "Image written to buffer" << std::endl;
        	incomingMessage->print(file);
			motionBuffer.pull().print(file);
			std::cout << "Motion read from buffer" << std::endl;
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
    HelloWorldSubscriberNode subscriberNode;

    // When the node has been created, it will cause an initStateEvent to
    // to be sent.
    subscriberNode.connectPolySync();

    return 0;
}
