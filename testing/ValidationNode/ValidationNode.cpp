#include <iostream>
#include <PolySyncNode.hpp>
#include <PolySyncDataModel.hpp>
#include <fstream>

using namespace std;


class ValidationNode : public polysync::Node
{
private:

    //define report message types
    ps_msg_type _brakeReportType;
    ps_msg_type _throttleReportType;
    ps_msg_type _steerReportType;

    //define command message types
    ps_msg_type _brakeCommandType;
    ps_msg_type _throttleCommandType;
    ps_msg_type _steerCommandType;

    //define image message type
    ps_msg_type _imageType;

    double currentBrakeCommandValue = 0;
    double currentBrakeReportValue = 0;
    double currentSteerCommandValue = 0;
    double currentSteerReportValue = 0;
    double currentThrottleCommandValue = 0;
    double currentThrottleReportValue = 0;

    bool gotBrake = false;
    bool gotThrottle = false;
    bool gotSteer = false;

    double brakeErr = -1;
	double throttleErr = -1;
	double steerErr = -1;

    //output file to record mse from steering, throttle and brake commands
    ofstream outputFile = ofstream("validation_errors.txt", std::ios_base::app | std::ofstream::out);

public:

    void initStateEvent() override
    {
    	//get report messages
    	_brakeReportType = getMessageTypeByName( "ps_platform_brake_report_msg" );
        _throttleReportType = getMessageTypeByName( "ps_platform_throttle_report_msg" );
        _steerReportType = getMessageTypeByName( "ps_platform_steering_report_msg" );

    	//get command messages
    	_brakeCommandType = getMessageTypeByName( "ps_platform_brake_command_msg" );
        _throttleCommandType = getMessageTypeByName( "ps_platform_throttle_command_msg" );
        _steerCommandType = getMessageTypeByName( "ps_platform_steering_command_msg" );

    	//get image message
        _imageType = getMessageTypeByName("ps_image_data_msg");

        //subscribe to report messages
        registerListener(_brakeReportType);
        registerListener(_throttleReportType);
        registerListener(_steerReportType);

        //subscribe to command messages
        registerListener(_brakeCommandType);
        registerListener(_throttleCommandType);
        registerListener(_steerCommandType);

        //subscribe to image message
        registerListener(_imageType);

        //write first line to file
        outputFile << "steeringErr, brakeErr, throttleErr\n";

        //TODO: - reference line, to remove
        // outputFile << "path,heading,longitude,latitude,quarternion0,quarternion1,quarternion2"
        // 	 <<",quarternion3,vel0,vel1,vel2,steering,throttle,brake\n";
    }

    virtual void messageEvent( std::shared_ptr< polysync::Message > message )
    {
		using namespace polysync::datamodel;

		//add command values to command buffers
		if (std::shared_ptr<PlatformBrakeCommandMessage> incomingMessage =
				getSubclass<PlatformBrakeCommandMessage>(message)) {
			std::cout << "brake command" << std::endl;
			gotBrake = true;
			currentBrakeCommandValue = incomingMessage->getBrakeCommand();
		}

		if (std::shared_ptr<PlatformThrottleCommandMessage> incomingMessage =
				getSubclass<PlatformThrottleCommandMessage>(message)) {
			gotThrottle = true;
			currentThrottleCommandValue = incomingMessage->getThrottleCommand();
		}

		if (std::shared_ptr<PlatformSteeringCommandMessage> incomingMessage =
				getSubclass<PlatformSteeringCommandMessage>(message)) {
			gotSteer = true;
			currentSteerCommandValue = incomingMessage->getSteeringWheelAngle();
		}

		//add report values to report buffers
		if (std::shared_ptr<PlatformBrakeReportMessage> incomingMessage =
				getSubclass<PlatformBrakeReportMessage>(message)) {
			std::cout << "brake report" << std::endl;
			currentBrakeReportValue = incomingMessage->getPedalInput();
		}

		if (std::shared_ptr<PlatformThrottleReportMessage> incomingMessage =
				getSubclass<PlatformThrottleReportMessage>(message)) {
			currentThrottleReportValue = incomingMessage->getPedalInput();
		}

		if (std::shared_ptr<PlatformSteeringReportMessage> incomingMessage =
				getSubclass<PlatformSteeringReportMessage>(message)) {
			currentSteerReportValue = incomingMessage->getSteeringWheelAngle();
		}

		//when image message is detected
		if (std::shared_ptr<ImageDataMessage> incomingMessage =
				getSubclass<ImageDataMessage>(message)) {

				if (gotBrake) {
					brakeErr = pow((currentBrakeReportValue - currentBrakeCommandValue), 2);
				}

				if (gotThrottle) {
					throttleErr = pow((currentThrottleReportValue - currentThrottleCommandValue), 2);
				}
				
				if (gotSteer) {
					steerErr = pow((currentSteerReportValue - currentSteerCommandValue), 2);
				}
				
				outputFile << std::to_string(steerErr) + "," 
						+ std::to_string(brakeErr) + "," 
						+ std::to_string(throttleErr) + "\n";

		}
	}

};

int main()
{
    ValidationNode valNode;
    valNode.connectPolySync();

    return 0;
}
