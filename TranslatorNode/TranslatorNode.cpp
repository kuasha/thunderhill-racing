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
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/graph/default_device.h"

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
class TranslatorNode : public polysync::Node
{
private:
    ps_msg_type _messageType;
    ps_msg_type _brakeType;
    ps_msg_type _throttleType;
    ps_msg_type _steerType;
    ps_msg_type _imageType;
    Buffer<polysync::datamodel::Motion> *motionBuffer = new Buffer<polysync::datamodel::Motion>(1);
    Buffer<DDS_float> *steerBuffer = new Buffer<DDS_float>(1);
    Buffer<DDS_float> *brakeBuffer = new Buffer<DDS_float>(1);
    Buffer<DDS_float> *throttleBuffer = new Buffer<DDS_float>(1);
    Buffer<polysync::datamodel::ImageDataMessage> *imageBuffer = new Buffer<polysync::datamodel::ImageDataMessage>(1);

    const static DDS_unsigned_long fwdFacingCameraSensorId = 15395510;

    tensorflow::GraphDef _graphDef;
    tensorflow::SessionOptions _sessionOptions;
    const string _graphPath = "/home/vlad/projects/thunderhill-racing/TranslatorNode/tensorflow_inception_graph.pb";

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

        loadTfModel();
    }

    /**
     * @brief messageEvent
     *
     * Extract the information from the provided message
     *
     * @param std::shared_ptr< Message > - variable containing the message
     * @return void
     */
    void messageEvent(std::shared_ptr< polysync::Message > message) override
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
            if (incomingMessage->getSensorDescriptor().getId() == fwdFacingCameraSensorId) {
                std::vector<DDS_octet> imageData =
                        incomingMessage->getDataBuffer();
            }
        }
    }

    void setConfigurationEvent( const GetOpt & commandLineOptions ) override {
    }

    void loadTfModel() {
        TF_CHECK_OK(ReadBinaryProto(tensorflow::Env::Default(), _graphPath, &_graphDef));

        // Configure GPU
        // graph::SetDefaultDevice("/gpu:0", &_graphDef);
        // _sessionOptions.config.mutable_gpu_options()->set_per_process_gpu_memory_fraction(0.5);
        // _sessionOptions.config.mutable_gpu_options()->set_allow_growth(true);

        fprintf(stderr, "Tensorflow model was successfully loaded from file!\n");
    }

    void runTfModel(DDS_float steerData, DDS_float brakeData, DDS_float throttleData, std::vector<DDS_octet> imageData) {
        Session* session;
        TF_CHECK_OK(NewSession(_sessionOptions, &session));

        TF_CHECK_OK(session->Create(_graphDef));

        Tensor steer(DT_FLOAT, TensorShape());
        steer.scalar<float>()() = steerData;

        Tensor brake(DT_FLOAT, TensorShape());
        brake.scalar<float>()() = brakeData;

        Tensor throttle(DT_FLOAT, TensorShape());
        throttle.scalar<float>()() = throttleData;

        std::vector<Tensor> outputs;
        TF_CHECK_OK(session->Run({{"steer", steer}, {"brake", brake}, {"throttle", throttle}}, {}, {}, &outputs));

        outputs.clear();
        session->Close();
        delete session;

        fprintf(stderr, "Tensorflow model was successfully ran!\n");
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
    TranslatorNode subscriberNode;

    // When the node has been created, it will cause an initStateEvent to
    // to be sent.
    subscriberNode.connectPolySync();

    return 0;
}
