#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
#include <math.h>
#include <iostream>

using namespace std;

bool itemPickedUp = false;
bool itemDroppedOff = false;
bool itemIsConsumed = false;

double pickUpX = 6.0;
double pickUpY = -5.0;
double dropOffX = 2.0;
double dropOffY = 0.0;

double robotOffsetX = 2;
double robotOffsetY = 0.35;

double threshold = 0.35;

template <typename T>
T distance(T & x1, T & y1, T & x2, T & y2) {
  return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

void callback(const nav_msgs::Odometry::ConstPtr & msg) {
  // Align with the current robot's pose
  double robotX = msg->pose.pose.position.x + robotOffsetX;
  double robotY = msg->pose.pose.position.y + robotOffsetY;

  if(distance(robotX, robotY, pickUpX, pickUpY) < threshold) {
    itemPickedUp = true;
    ROS_WARN_ONCE("Item picked");
  }
  
  if(distance(robotX, robotY, dropOffX, dropOffY) < threshold && itemPickedUp) {
    itemIsConsumed = true;
    ROS_WARN_ONCE("Item is consumed successfully");
  }
}

int main(int argc, char ** argv) {
  ros::init(argc, argv, "add_markers");
  ros::NodeHandle n;
  ros::Publisher publisher = n.advertise<visualization_msgs::Marker>("visualization_marker", 1);
  ros::Subscriber subscriber = n.subscribe("odom", 10, callback);

  visualization_msgs::Marker marker;

  // Set our initial shape type to be a cube
  uint32_t shape = visualization_msgs::Marker::CUBE;

  // Set the frame ID
  marker.header.frame_id = "map";

  // Set the namespace and id for this marker.  This serves to create a unique ID
  // Any marker sent with the same namespace and id will overwrite the old one
  marker.ns = "cube";
  marker.id = 0;

  // Set the marker type.  Initially this is CUBE, and cycles between that and SPHERE, ARROW, and CYLINDER
  marker.type = shape;

  // Set the pose of the marker.  This is a full 6DOF pose relative to the frame/time specified in the header
  marker.pose.position.x = pickUpX;
  marker.pose.position.y = pickUpY;
  marker.pose.position.z = 0;
  marker.pose.orientation.x = 0.0;
  marker.pose.orientation.y = 0.0;
  marker.pose.orientation.z = 0.0;
  marker.pose.orientation.w = 1.0;

  // Set the scale of the marker -- 1x1x1 here means 1m on a side
  marker.scale.x = 0.2;
  marker.scale.y = 0.2;
  marker.scale.z = 0.2;

  // Set the color -- be sure to set alpha to something non-zero!
  marker.color.r = 0.0f;
  marker.color.g = 1.0f;
  marker.color.b = 0.0f;
  marker.color.a = 1.0;

  marker.lifetime = ros::Duration();
  while (ros::ok()) {
    // Publish the marker
    while (publisher.getNumSubscribers() < 1)  {
      if (!ros::ok()) {
        return 0;
      }

      ROS_WARN_ONCE("Please create a subscriber to the marker");
      sleep(1);
    }
  
    ROS_WARN_ONCE("Someone subscribed to the marker");

    // Set the frame timestamp.
    marker.header.stamp = ros::Time::now();

    if (!itemPickedUp) {
      marker.action = visualization_msgs::Marker::ADD;
      publisher.publish(marker);
    } else {
      if (!itemDroppedOff) {
        marker.action = visualization_msgs::Marker::DELETE;
        publisher.publish(marker);
        ros::Duration(5.0).sleep();

        itemDroppedOff = true;
      } else {
        marker.pose.position.x = dropOffX;
        marker.pose.position.y = dropOffY;

        marker.action = visualization_msgs::Marker::ADD;
        publisher.publish(marker);
      }
    }

    if (itemIsConsumed) {
      return 0;
    }
    
    ros::spinOnce();
  }
}