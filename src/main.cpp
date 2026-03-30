#include "rclcpp/rclcpp.hpp"
#include "nlink_parser2/msg/linktrack_tagframe0.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class LinkTrackDemo : public rclcpp::Node
{
public:

  LinkTrackDemo() : Node("linktrack_demo"), count_(0)
  {
    // ================== 数据订阅部分 ==================

    // 订阅标签数据（移动标签信息）
    tag_sub_ = create_subscription<nlink_parser2::msg::LinktrackTagframe0>(
      "nlink_linktrack_tagframe0", 10,
      std::bind(&LinkTrackDemo::tag_callback, this, std::placeholders::_1));
    // ================== 数据发布部分 ==================

    // 创建发布器：向LinkTrack设备发送控制命令
    data_pub_ = create_publisher<std_msgs::msg::String>(
      "nlink_linktrack_data_transmission", 10);

    timer_ = create_wall_timer(
      1000ms, std::bind(&LinkTrackDemo::timer_callback, this));

    RCLCPP_INFO(get_logger(), "LinkTrack演示节点已启动");
    RCLCPP_INFO(get_logger(), "将发送数据到话题: nlink_linktrack_data_transmission");
    RCLCPP_INFO(get_logger(), "正在监听以下话题:");
    RCLCPP_INFO(get_logger(), "  - nlink_linktrack_tagframe0");
  }

private:
  void tag_callback(const nlink_parser2::msg::LinktrackTagframe0::SharedPtr msg)
  {
    RCLCPP_INFO(get_logger(),
                "标签数据 - 角色: %d, ID: %d, 位置: (%.3f, %.3f, %.3f), "
                "系统时间: %u, 电压: %.2fV",
                msg->role, msg->id, 
                msg->pos_3d[0], msg->pos_3d[1], msg->pos_3d[2],
                msg->system_time, msg->voltage);

    if (std::abs(msg->quaternion[3]) > 0.001) {
      RCLCPP_INFO(get_logger(),
                  "  姿态 - 四元数: (%.3f, %.3f, %.3f, %.3f)",
                  msg->quaternion[0], msg->quaternion[1],
                  msg->quaternion[2], msg->quaternion[3]);
    }

    RCLCPP_INFO(get_logger(), "  距离数组: [%.2f, %.2f, %.2f, ...]",
                msg->dis_arr[0], msg->dis_arr[1], msg->dis_arr[2]);

    RCLCPP_DEBUG(get_logger(),
                "  IMU陀螺仪: (%.3f, %.3f, %.3f)",
                msg->imu_gyro_3d[0], msg->imu_gyro_3d[1], msg->imu_gyro_3d[2]);
    RCLCPP_DEBUG(get_logger(),
                "  IMU加速度: (%.3f, %.3f, %.3f)",
                msg->imu_acc_3d[0], msg->imu_acc_3d[1], msg->imu_acc_3d[2]);
  }

  void timer_callback()
  {
    auto message = std_msgs::msg::String();

    std::string command;
    switch (count_ % 2)
    {
      case 0:
        command = "112233445566778899"; 
        break;
      case 1:
        command = "11122322324434545565677667877799aaa213455765565656565778433334444444444676"; 
        break;
    }

    message.data = command;
    data_pub_->publish(message);

    RCLCPP_INFO(get_logger(), "发送数据到LinkTrack: %s", command.c_str());
    count_++;  
  }

  rclcpp::Subscription<nlink_parser2::msg::LinktrackTagframe0>::SharedPtr tag_sub_;

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr data_pub_;

  rclcpp::TimerBase::SharedPtr timer_;

  int count_;
};

int main(int argc,char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LinkTrackDemo>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
