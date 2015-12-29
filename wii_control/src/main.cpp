//AC
#include "wii_lib/wii_lib.h"
#include "std_msgs/Bool.h"

bool stop = false;	//stop flag 

void stopCallback(const std_msgs::Bool::ConstPtr& stp)
{
  stop = stp;	//received flag by short_moves_pub node if it publishes to servo
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "teleop_wii");
    wii_lib teleop_wii;
    
    ros::NodeHandle n;
    ros::Subscriber interrupt_sub = n.subscribe<std_msgs::Bool>("/interrupt",100, stopCallback);	//subscribe to topic interrupt which indicates if servo is occupied

    ros::Rate loop_rate(50);

    static int count = 0;

    while(ros::ok())
    {

        teleop_wii.wii_communication_pub.publish(teleop_wii.wii_state_);
	//ROS_INFO("Debug message!");
	if(!stop)	//do only wait for wii_lib callback if short_moves_pub does not publish to servo for not colliding
        {
		ros::spinOnce();
	}
        loop_rate.sleep();
    }

    return 0;
}


