#include <ros/ros.h>
#include <darknet_ros_msgs/BoundingBoxes.h>
#include <darknet_ros_msgs/BoundingBox.h>
#include <string>
#include <soccer_geometry/pose3.hpp>
#include <soccer_fieldline_detection/camera.hpp>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/transform_broadcaster.h>

class BallDetector {
public:
    ros::NodeHandle n;
    ros::Subscriber darknet_pose_sub;
    ros::Publisher head_motor_0;
    std::unique_ptr<Camera> camera;
    tf2_ros::Buffer tfBuffer;
    tf2_ros::TransformListener tfListener;
    tf2_ros::TransformBroadcaster br;

    BallDetector() : tfListener(tfBuffer) {
        darknet_pose_sub = n.subscribe("darknet_ros/bounding_boxes", 1000, &BallDetector::ballDetectorCallback, this);

        geometry_msgs::TransformStamped camera_pose;

        while (ros::ok()) {
            try {
                camera_pose = tfBuffer.lookupTransform("camera", "base_footprint",
                                                       ros::Time(0), ros::Duration(1.0));
                break;
            }
            catch (tf2::TransformException &ex) {
                std::string s = ex.what();
            }
        }

        // Initalize Camera
        Pose3 camera_position;
        camera_position.position.x = camera_pose.transform.translation.x;
        camera_position.position.y = camera_pose.transform.translation.y;
        camera_position.position.z = camera_pose.transform.translation.z;
        camera_position.orientation.w = camera_pose.transform.rotation.w;
        camera_position.orientation.x = camera_pose.transform.rotation.x;
        camera_position.orientation.y = camera_pose.transform.rotation.y;
        camera_position.orientation.z = camera_pose.transform.rotation.z;
        camera = std::make_unique<Camera>(camera_position);
    }

private:
    void ballDetectorCallback(const darknet_ros_msgs::BoundingBoxes::ConstPtr &msg) {
        // Get transformation
        geometry_msgs::TransformStamped camera_pose;
        try {
            camera_pose = tfBuffer.lookupTransform("base_footprint", "camera",
                                                   ros::Time(0), ros::Duration(0.1));

            Pose3 camera_position;
            camera_position.position.x = camera_pose.transform.translation.x;
            camera_position.position.y = camera_pose.transform.translation.y;
            camera_position.position.z = camera_pose.transform.translation.z;
            camera_position.orientation.w = camera_pose.transform.rotation.w;
            camera_position.orientation.x = camera_pose.transform.rotation.x;
            camera_position.orientation.y = camera_pose.transform.rotation.y;
            camera_position.orientation.z = camera_pose.transform.rotation.z;
            camera->setPose(camera_position);
        }
        catch (tf2::TransformException &ex) {
            return;
        }

        if (!camera->ready()) {
            return;
        }

        for (const darknet_ros_msgs::BoundingBox &box: msg->bounding_boxes) {
            std::string objectClass = box.Class;
            if (objectClass != "sports ball") {//sports ball bird for testing

                continue;
            }
            // For now take the center of the box
            int xavg = (box.xmin + box.xmax) / 2;
            int yavg = (box.ymin + box.ymax) / 2;
            Point3 floor_coordinate = camera->FindFloorCoordinate(xavg, yavg);

            geometry_msgs::TransformStamped ball_pose;
            ball_pose.header.frame_id = "base_footprint";
            ball_pose.child_frame_id = "ball";
            ball_pose.header.stamp = msg->header.stamp;
            ball_pose.header.seq = msg->header.seq;
            ball_pose.transform.translation.x = floor_coordinate.x;
            ball_pose.transform.translation.y = floor_coordinate.y;
            ball_pose.transform.translation.z = floor_coordinate.z;
            ball_pose.transform.rotation.x = 0;
            ball_pose.transform.rotation.y = 0;
            ball_pose.transform.rotation.z = 0;
            ball_pose.transform.rotation.w = 1;

            BallDetector::br.sendTransform(ball_pose);
        }
    }
};

int main(int argc, char **argv) {
    ros::init(argc, argv, "ball_detector");

    BallDetector ballDetector;

    ros::spin();
    return 0;
}