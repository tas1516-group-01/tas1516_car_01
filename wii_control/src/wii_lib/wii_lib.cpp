#include "wii_lib.h"

wii_lib::wii_lib()
{
    /*Initilaization of publishers, subscribers and messages*/
    wii_servo_pub_ = nh_.advertise<geometry_msgs::Vector3>("servo", 1);

    wii_communication_pub = nh_.advertise<std_msgs::Int16MultiArray>("wii_communication",1);

    wii_sub_ = nh_.subscribe<wiimote::State>("wiimote/state",100,&wii_lib::wiiStateCallback,this);

    interrupt = nh_.subscribe<std_msgs::Bool>("/interrupt",100, &wii_lib::stopCallback,this);	//subscribe to topic interrupt which indicates if servo is occupied (AC)

    msg_Initialization(wii_state_);


    controlMode.data = 0;
    emergencyBrake.data = 1;
    stop = false;   //stop flag which is set to indicate if wii communication active (AC)
}

//callback to set stop flag
void wii_lib::stopCallback(const std_msgs::Bool::ConstPtr& stp)
{
     ROS_INFO("flag changed");
     stop = stp.get()->data;	//received flag by short_moves_pub node if it publishes to servo
}

void wii_lib::wiiStateCallback(const wiimote::State::ConstPtr& wiiState)
{
    /*check if C button is pressed*/
    if(wiiState.get()->nunchuk_buttons[WII_BUTTON_NUNCHUK_C]==1)
    {
        controlMode.data = 1; /*setting controlMode flag to 1*/

        if(wiiState.get()->nunchuk_buttons[WII_BUTTON_NUNCHUK_Z]==1)
        {
            emergencyBrake.data = 1; /*setting emergencyBrake flag to 1*/
        }
        else
        {
            emergencyBrake.data = 0; /*setting emergencyBrake flag to 0*/
        }
    }
    else
    {
        controlMode.data = 0;/*setting controlMode flag to 0*/

        /*check if Z button is pressed*/
        if(wiiState.get()->nunchuk_buttons[WII_BUTTON_NUNCHUK_Z]==1)
        {
            emergencyBrake.data = 1; /*setting emergencyBrake flag to 1*/

            servo.x = 1500;
            servo.y = 1500;
        }
        else
        {
            emergencyBrake.data = 0; /*setting emergencyBrake flag to 0*/

            if(wiiState.get()->nunchuk_joystick_zeroed[1]>=0)
            {
                SCALE_FAKTOR_THROTTLE = 200; /*scale factor for driving forward*/
            }
            else
            {
                SCALE_FAKTOR_THROTTLE = 300; /*scale factor for driving reverse*/
            }

            /* mapping analog nunchuk state to servo command*/
            servo.x = 1500 + SCALE_FAKTOR_THROTTLE*wiiState.get()->nunchuk_joystick_zeroed[1];
            servo.y = 1500 + SCALE_FAKTOR_STEERING*wiiState.get()->nunchuk_joystick_zeroed[0];
        }

        //only enable publishing of wii controler if stop flag is not set (AC)
        if(!stop)
        {
            ROS_INFO("active");
            wii_servo_pub_.publish(servo); /*publish servo messages to arduino*/
        }
        else
            ROS_INFO("blocked");
   }

    wii_state_.data[0] = controlMode.data;
    wii_state_.data[1] = emergencyBrake.data;
}

void wii_lib::msg_Initialization(std_msgs::Int16MultiArray &msg)
{
    msg.layout.dim.push_back(std_msgs::MultiArrayDimension());
    msg.layout.dim[0].size = 2;
    msg.layout.dim[0].stride = 1;
    msg.layout.dim[0].label = "flag";
    msg.data.clear();
    msg.data.resize(2);
}

