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

#ifndef NODE_FLAGS_VALUE
#define NODE_FLAGS_VALUE (0)
#endif

using namespace std;

typedef void (*imageCallback)(int, unsigned char *);

class MainNode : public polysync::Node
{
private:
    const string node_name = "polysync-python-test-client-cpp";
    ps_msg_type _messageType;
    std::vector <unsigned char> imageData;
    unsigned int imageSize;

public:
    imageCallback imageRecieved = NULL;

    MainNode()
    {
        setNodeType( PSYNC_NODE_TYPE_API_USER );
        setDomainID( PSYNC_DEFAULT_DOMAIN );
        setSDFID( PSYNC_SDF_ID_INVALID );
        setFlags( NODE_FLAGS_VALUE );
        setNodeName( node_name );
    }

    ~MainNode()
    {

    }

    void initStateEvent() override
    {
        _messageType = getMessageTypeByName( "ps_image_data_msg" );
        registerListener( _messageType );
    }

    virtual void messageEvent( std::shared_ptr< polysync::Message > message )
    {
        using namespace polysync::datamodel;

        if (std::shared_ptr < ImageDataMessage > incomingMessage = getSubclass < ImageDataMessage > (message))
        {
            if (incomingMessage->getPixelFormat() == PIXEL_FORMAT_MJPEG) {
                std::vector <unsigned char> image = incomingMessage->getDataBuffer();
                imageSize = image.size();
                if (imageSize > imageData.size())
                {
                    imageData = vector<unsigned char> (imageSize);
                }
                unsigned char *p = imageData.data();
                unsigned char *q = image.data();
                for (unsigned int i=0; i<imageSize; i++)
                {
                    *p = *q;
                    ++p;
                    ++q;
                }
                if (imageRecieved != NULL) {
                    imageRecieved(imageData.size(), imageData.data());
                }
            }

        }
    }

    void steerCommand(float angle)
    {
        polysync::datamodel::PlatformSteeringCommandMessage message( *this);
        message.setTimestamp( polysync::getTimestamp() );
        message.setSteeringWheelAngle(angle);
        message.setHeaderTimestamp( polysync::getTimestamp() );
        polysync::sleepMicro( 1000000 );
        message.print();
    }

    void brakeCommand(float value)
    {
        polysync::datamodel::PlatformBrakeCommandMessage message( *this);
        message.setTimestamp( polysync::getTimestamp() );
        message.setBrakeCommand(value);
        message.setHeaderTimestamp( polysync::getTimestamp() );
        polysync::sleepMicro( 1000000 );
        message.print();
    }

    void throttleCommand(float value)
    {
        polysync::datamodel::PlatformThrottleCommandMessage message( *this);
        message.setTimestamp( polysync::getTimestamp() );
        message.setThrottleCommand(value);
        message.setHeaderTimestamp( polysync::getTimestamp() );
        polysync::sleepMicro( 1000000 );
        message.print();
    }

};

extern "C" {
    MainNode* MainNode_new(){ return new MainNode(); }
    void MainNode_connectPolySync(MainNode* node){ node->connectPolySync(); }
    void MainNode_setImageCallback(MainNode* node, imageCallback imageRecieved){ node->imageRecieved = imageRecieved; }
    void MainNode_steerCommand(MainNode* node, float angle){ node->steerCommand(angle); }
    void MainNode_brakeCommand(MainNode* node, float value){ node->brakeCommand(value); }
    void MainNode_throttleCommand(MainNode* node, float value){ node->throttleCommand(value); }
}

// int main()
// {
//     MainNode node;
//     node.connectPolySync();

//     return 0;
// }
