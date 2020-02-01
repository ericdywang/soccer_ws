#include <ros/ros.h>
#include <soccer_fieldline_detection/test_soccer_fieldline_detector.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>
#include <soccer_geometry/segment2.hpp>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <tf/transform_listener.h>

SoccerFieldlineDetector::SoccerFieldlineDetector() : priv_nh("~"), tfListener(tfBuffer){
    image_transport::ImageTransport it(nh);
    image_subscriber = it.subscribe("camera/image_raw", 1, &SoccerFieldlineDetector::imageCallback, this);
    point_cloud_publisher = SoccerFieldlineDetector::nh.advertise<sensor_msgs::PointCloud2> ("field_point_cloud",1);

    image_publisher = it.advertise("camera/line_image",1);

    // Parameters
    priv_nh.getParam("tf_namespace", tf_namespace);

    priv_nh.getParam("cannythreshold1", cannythreshold1);
    priv_nh.getParam("cannythreshold2", cannythreshold2);

    priv_nh.getParam("houghRho", rho);
    priv_nh.getParam("houghTheta", theta);
    priv_nh.getParam("houghThreshold", threshold);
    priv_nh.getParam("houghMinLineLength", minLineLength);
    priv_nh.getParam("houghMaxLineGap", maxLineGap);

    // Initialize Camera
    geometry_msgs::TransformStamped camera_pose;

    while(ros::ok()) {
        try{
            camera_pose = tfBuffer.lookupTransform(tf_namespace + "camera", tf_namespace + "base_footprint",
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

void SoccerFieldlineDetector::imageCallback(const sensor_msgs::ImageConstPtr &msg) {

    std::vector<cv::Vec4i> lines;
    std::vector<Point2> pts;

    if (!camera->ready()) {
        return;
    }

    // Get transformation
    geometry_msgs::TransformStamped camera_pose;
    try{
        camera_pose = tfBuffer.lookupTransform(tf_namespace + "base_footprint", tf_namespace + "camera",
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

    try {
        const cv::Mat image = cv_bridge::toCvShare(msg, "bgr8")->image;

        // Detect Field Lines (Copy from simulink)
        cv::Mat dst, cdst;
        cv::Canny(image, dst,cannythreshold1,cannythreshold2);
        cvtColor(dst, cdst, CV_GRAY2BGR);

        // Cover Horizon
        double roll, pitch, yaw;
        tf2::Quaternion q(camera->getPose().orientation.x,camera->getPose().orientation.y,camera->getPose().orientation.z,camera->getPose().orientation.w);
        tf2::Matrix3x3 m(q);
        m.getRPY(roll, pitch, yaw);
        //Draw black box on screen based on the pitch of the camera
        cv::rectangle(dst, cv::Point(0,(camera->getResolutionY()/2) -400*pitch), cv::Point( camera->getResolutionX(),0), cv::Scalar(0, 0, 0), -1, 8);

        HoughLinesP(dst, lines, rho, theta,threshold,minLineLength,maxLineGap);

        for (const auto& l : lines) {
            cv::line(dst, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, CV_AA);

            Point2 pt1(l[0],l[1]);
            Point2 pt2(l[2],l[3]);
            Segment2 s(pt1,pt2);
            float b = l[1] - s.slope()*l[0];

            for (size_t i = l[0]; i < l[2];i += 3) {
                float y = s.slope()*i + b;
                Point2 pt(i,y);
                pts.push_back(pt);
            }
        }

        sensor_msgs::ImagePtr message = cv_bridge::CvImage(std_msgs::Header(), "bgr8", cdst).toImageMsg();
        //if(image_publisher.getNumSubscribers() > 1){
        image_publisher.publish(message);
        //}

    } catch (const cv_bridge::Exception &e) {
        ROS_ERROR_STREAM("CV Exception" << e.what());
    }


    std::vector<Point3> points3d;
    for (const auto& p : pts) {
        Point3 p2 = camera->FindFloorCoordinate(p.x, p.y);
        points3d.push_back(p2);
    }

    // Publish fieldlines in a LaserScan data format
    sensor_msgs::PointCloud2 point_cloud_msg;
    //Setting up PointCloud2 msg
    point_cloud_msg.header.stamp = ros::Time::now();
    point_cloud_msg.header.frame_id = tf_namespace + "base_footprint";
    point_cloud_msg.height = 1;
    point_cloud_msg.width = points3d.size();
    point_cloud_msg.is_bigendian = false;
    point_cloud_msg.is_dense = false;
    point_cloud_msg.point_step = 12;
    point_cloud_msg.row_step = 12 * points3d.size();

    //Adding points to the PointCloud2 msg
    sensor_msgs::PointCloud2Modifier modifier(point_cloud_msg);
    modifier.setPointCloud2FieldsByString(1,"xyz");
    modifier.resize(points3d.size());

    sensor_msgs::PointCloud2Iterator<float> iter_x(point_cloud_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(point_cloud_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(point_cloud_msg, "z");

    for (const auto& p : points3d){
        *iter_x = p.x;
        *iter_y = p.y;
        *iter_z = p.z;
        ++iter_x;
        ++iter_y;
        ++iter_z;
    }

    point_cloud_publisher.publish(point_cloud_msg);
    pts.clear();
    points3d.clear();

    geometry_msgs::TransformStamped camera_footprint;
    try {
        camera_footprint = tfBuffer.lookupTransform( tf_namespace + "camera",tf_namespace + "base_footprint",
                                               ros::Time(0), ros::Duration(0.1));
        camera_footprint.header.frame_id = tf_namespace + "base_footprint";
        camera_footprint.child_frame_id = tf_namespace + "base_camera";
        camera_footprint.header.stamp = msg->header.stamp;
        camera_footprint.header.seq = msg->header.seq;

        tf2::Quaternion q(camera_footprint.transform.rotation.x,
                          camera_footprint.transform.rotation.y,
                          camera_footprint.transform.rotation.z,
                          camera_footprint.transform.rotation.w);
        tf2::Matrix3x3 m(q);
        double r,p,y;
        m.getRPY(r,p,y);
        q.setRPY(0,0,y);
        q.normalize();
        camera_footprint.transform.rotation.x = q[0];
        camera_footprint.transform.rotation.y = q[1];
        camera_footprint.transform.rotation.z = q[2];
        camera_footprint.transform.rotation.w = q[3];
        camera_footprint.transform.translation.z = 0;

        br.sendTransform(camera_footprint);
    }
    catch (tf2::TransformException &ex) {
        return;
    }
}



int main(int argc, char **argv) {
    ros::init(argc, argv, "soccer_fieldline_detector");

    SoccerFieldlineDetector detector;

    ros::spin();

    return 0;
}