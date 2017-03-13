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
#include <stdio.h>
#include <string>
#include <math.h>

/**
 * @brief Node flags to be OR'd with driver/interface flags.
 *
 * Provided by the compiler so PolySync can add build-specifics as needed.
 *
 */
#ifndef NODE_FLAGS_VALUE
#define NODE_FLAGS_VALUE (0)
#endif

#define PI 3.14159265

using namespace std;

/**
 * @brief HellowWorldSubscriberNode class
 *
 * The HelloWorldSubscriberNode class exists to override the functions defined
 * in the base Node class.  The functions exist in the base class but are
 * stubbed out and must be overloaded in order for them to do something.  In
 * this instance the initStateEvent and the messageHandlerEvent are overloaded
 * to register for the messages and receive them, respectively.
 */
typedef void (*latloncallback)(double, double);
typedef void (*yawheadingcallback)(double, double);
typedef void (*velocitycallback)(double, double, double);
typedef void (*imagecallback)(std::vector < DDS_octet >);

class RecorderNode : public polysync::Node
{
private:
    const string node_name = "polysync-python-test-client-cpp";
    ps_msg_type _messageType;
    ps_msg_type _imageType;

public:
    latloncallback alatloncallback = NULL;
    yawheadingcallback ayawheadingcallback = NULL;
    velocitycallback avelocitycallback = NULL;
    imagecallback aimagecallback = NULL;

    RecorderNode()
    {
        setNodeType( PSYNC_NODE_TYPE_API_USER );
        setDomainID( PSYNC_DEFAULT_DOMAIN );
        setSDFID( PSYNC_SDF_ID_INVALID );
        setFlags( NODE_FLAGS_VALUE );
        setNodeName( node_name );
    }

    ~RecorderNode()
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
            //Not used right now
            //DDS_unsigned_long_long ts = incomingMessage->getHeaderTimestamp();

            double lat = incomingMessage->getLatitude();
            double lon = incomingMessage->getLongitude();
            if (alatloncallback != NULL)
                alatloncallback(lat, lon);

            //to find yaw, use orientation quaternion
            //yaw   =  Mathf.Asin(2*x*y + 2*z*w);
            std::array< DDS_double, 4 > orient = incomingMessage->getOrientation();
            double yaw = asin((2 * orient[0] * orient[1]) + (2 * orient[2] * orient[3]))  * 180.0 / PI;
            double heading = incomingMessage->getHeading();
            if (ayawheadingcallback != NULL)
                ayawheadingcallback(yaw, heading);

            //to put to csv: ts, yaw, heading, velocity (x, y, z)
            std::array< DDS_double, 3 > vel = incomingMessage->getVelocity();
            if (avelocitycallback != NULL)
                avelocitycallback(vel[0], vel[1], vel[2]);

            // std::string dataString = std::to_string(ts)+","+std::to_string(yaw) +","+std::to_string(heading)+","+std::to_string(vel[0])+","+std::to_string(vel[1])+","+std::to_string(vel[2])+"\n";
            // file << dataString;
        }

        if (std::shared_ptr < ImageDataMessage > incomingMessage = getSubclass < ImageDataMessage > (message))
        {
            FILE *stream;
            DDS_unsigned_long_long ts = incomingMessage->getHeaderTimestamp();
            std::string imageName = "IMG/" + std::to_string(incomingMessage->getSensorDescriptor().getId()) + '-' + std::to_string(ts) + ".jpeg";
            const char * c = imageName.c_str();
            if((stream = freopen(c, "w", stdout)) == NULL) {
              fprintf(stderr,"Can't create file: %s\nTry:\n\tmkdir IMG\n\n", imageName.c_str());
              fflush(stderr);
                exit(-1);
            }

            std::vector < DDS_octet > imageData = incomingMessage->getDataBuffer();
            if (aimagecallback != NULL)
                aimagecallback(imageData);
            
            //for (int d: imageData) {
            //    printf("%c", d);
            //}
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
/*
int main()
{
    // Create an instance of the HelloWorldNode and connect it to PolySync
    RecorderNode recorderNode;

    // When the node has been created, it will cause an initStateEvent to
    // to be sent.
    recorderNode.connectPolySync();

    return 0;
}
*/


extern "C" {
    RecorderNode* RecorderNode_new(){ return new RecorderNode(); }
    void RecorderNode_connectPolySync(RecorderNode* node){ node->connectPolySync(); }
    void RecorderNode_setLatLonCallback(RecorderNode* node, latloncallback alatloncallback){ node->alatloncallback = alatloncallback; }
    void RecorderNode_setYawCallback(RecorderNode* node, yawheadingcallback ayawheadingcallback){ node->ayawheadingcallback = ayawheadingcallback; }
    void RecorderNode_setVelocityCallback(RecorderNode* node, velocitycallback avelocitycallback){ node->avelocitycallback = avelocitycallback; }
    void RecorderNode_setImageCallback(RecorderNode* node, imagecallback aimagecallback){ node->aimagecallback = aimagecallback; }
}

