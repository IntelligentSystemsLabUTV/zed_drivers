/**
 * ZED Mini Driver node definition.
 *
 * Roberto Masocco <robmasocco@gmail.com>
 * Intelligent Systems Lab <isl.torvergata@gmail.com>
 *
 * June 13, 2023
 */

#ifndef ZED_DRIVERS__ZED_MINI_DRIVER_HPP
#define ZED_DRIVERS__ZED_MINI_DRIVER_HPP

#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <Eigen/Geometry>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <sl/Camera.hpp>

#include <rclcpp/rclcpp.hpp>

#include <pose_kit/pose.hpp>
#include <pose_kit/kinematic_pose.hpp>

#include <dua_node/dua_node.hpp>
#include <dua_qos/dua_qos.hpp>

#include <camera_info_manager/camera_info_manager.hpp>
#include <image_transport/image_transport.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <tf2/exceptions.h>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <dua_interfaces/msg/point_cloud2_with_roi.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <std_msgs/msg/header.hpp>

#include <std_srvs/srv/set_bool.hpp>

using namespace dua_interfaces::msg;
using namespace geometry_msgs::msg;
using namespace nav_msgs::msg;
using namespace sensor_msgs::msg;
using namespace std_msgs::msg;

using namespace std_srvs::srv;

namespace ZEDMiniDriver
{

/**
 * Drives a ZED Mini camera.
 */
class ZEDMiniDriverNode : public DUANode::NodeBase
{
public:
  ZEDMiniDriverNode(const rclcpp::NodeOptions & opts = rclcpp::NodeOptions());
  virtual ~ZEDMiniDriverNode();

private:
  /* Node initialization routines. */
  void init_atomics();
  void init_parameters();
  void init_publishers();
  void init_services();

  /* TF listeners, thread, and related data. */
  std::shared_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  TransformStamped odom_to_camera_odom_{};
  TransformStamped base_link_to_camera_{};
  TransformStamped map_to_camera_odom_{};
  std::thread tf_thread_;
  void tf_thread_routine();

  /* Topic publishers. */
  rclcpp::Publisher<Odometry>::SharedPtr base_link_odom_pub_;
  rclcpp::Publisher<Odometry>::SharedPtr camera_odom_pub_;
  rclcpp::Publisher<Imu>::SharedPtr imu_pub_;
  rclcpp::Publisher<PointCloud2>::SharedPtr point_cloud_pub_;
  rclcpp::Publisher<PointCloud2WithROI>::SharedPtr point_cloud_roi_pub_;
  rclcpp::Publisher<Odometry>::SharedPtr rviz_base_link_odom_pub_;
  rclcpp::Publisher<PoseStamped>::SharedPtr rviz_base_link_pose_pub_;
  rclcpp::Publisher<Odometry>::SharedPtr rviz_camera_odom_pub_;
  rclcpp::Publisher<PoseStamped>::SharedPtr rviz_camera_pose_pub_;
  rclcpp::Publisher<PointCloud2>::SharedPtr rviz_point_cloud_pub_;

  /* image_transport publishers. */
  std::shared_ptr<image_transport::CameraPublisher> left_rect_pub_;
  std::shared_ptr<image_transport::CameraPublisher> right_rect_pub_;
  std::shared_ptr<image_transport::Publisher> depth_pub_;

  /* Service servers. */
  rclcpp::Service<SetBool>::SharedPtr enable_srv_;

  /* Service callbacks. */
  void enable_callback(
    const SetBool::Request::SharedPtr req,
    const SetBool::Response::SharedPtr resp);

  /* Synchronization primitives. */
  std::mutex tf_lock_;

  /* Internal state and data. */
  sl::Camera zed_;
  std::atomic<bool> running_;
  std::atomic<bool> tf_listening_;
  camera_info_manager::CameraInfo left_info_{};
  camera_info_manager::CameraInfo right_info_{};

  /* Camera sampling thread and routine. */
  void camera_routine();
  std::thread camera_thread_;

  /* Node parameters. */
  int64_t confidence_ = 50;
  sl::DEPTH_MODE depth_mode_ = sl::DEPTH_MODE::QUALITY;
  int fps_ = 15;
  std::string link_namespace_ = "";
  sl::RESOLUTION resolution_ = sl::RESOLUTION::HD720;
  int64_t texture_confidence_ = 100;
  bool verbose_ = false;

  /* Node parameters validators. */
  bool validate_depth_mode(const rclcpp::Parameter & p);
  bool validate_fps(const rclcpp::Parameter & p);
  bool validate_resolution(const rclcpp::Parameter & p);

  /* Auxiliary routines. */
  bool open_camera();
  void close_camera();
  cv::Mat slMat2cvMat(sl::Mat & input);
  Image::SharedPtr frame_to_msg(cv::Mat & frame);
};

} // namespace ZEDMiniDriver

#endif // ZED_DRIVERS__ZED_MINI_DRIVER_HPP
