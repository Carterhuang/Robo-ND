#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

namespace
{

    // Define a global client that can request services
    ros::ServiceClient client;

    // This function calls the command_robot service to drive the robot in the specified direction
    void drive_robot(float lin_x, float ang_z)
    {
        // TODO: Request a service and pass the velocities to it to drive the robot
        ball_chaser::DriveToTarget srv;
        srv.request.linear_x = lin_x;
        srv.request.angular_z = ang_z;

        client.call(srv);
    }

    enum BallPosition
    {
        NO_BALL = -1,
        LEFT = 0,
        MID = 1,
        RIGHT = 2,
        TOO_CLOSE = 3
    };

    // This callback function continuously executes and reads the image data
    void process_image_callback(const sensor_msgs::Image img)
    {

        int white_pixel = 255;

        // TODO: Loop through each pixel in the image and check if there's a bright white one
        // Then, identify if this pixel falls in the left, mid, or right side of the image
        // Depending on the white ball position, call the drive_bot function and pass velocities to it
        // Request a stop when there's no white ball seen by the camera
        bool ball_found = false;
        BallPosition ball_position = NO_BALL;
        int white_pixel_count = 0;

        for (int i = 0; i < img.height * img.step; i += 3) // increment by 3 to check RGB values
        {
            if (img.data[i] == white_pixel && img.data[i + 1] == white_pixel && img.data[i + 2] == white_pixel)
            {
                ball_found = true;
                white_pixel_count++;

                int col = (i / 3) % img.width ;// calculate column index
                if (col < img.width / 3)
                    ball_position = LEFT;
                else if (col < 2 * img.width / 3)
                    ball_position = MID;
                else
                    ball_position = RIGHT;

            }

            if (white_pixel_count > img.height * img.width / 20) {
                // if more than 10% of the image is white, consider it too close
                ball_position = TOO_CLOSE;
                break; // stop checking further pixe
            }
        }

        if (ball_found)
        {
            switch (ball_position)
            {
            case LEFT:
                ROS_INFO("Ball found on the left side");
                drive_robot(0.2, 0.7); // move left
                break;
            case MID:
                ROS_INFO("Ball found in the middle");
                drive_robot(0.2, 0.0); // move forward
                break;
            case RIGHT:
                ROS_INFO("Ball found on the right side");
                drive_robot(0.2, -0.7); // move right
                break;
            case NO_BALL:
            case TOO_CLOSE:
                // ROS_INFO("Ball too close or not detected properly");
                drive_robot(0.0, 0.0); // stop the robot
                break;
            default:
                break;
            }
        }
        else
        {
            // ROS_INFO("No ball detected, stopping the robot");
            drive_robot(0.0, 0.0); // stop the robot
        }
    }

} // end of namespace

int main(int argc, char **argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
