#include <rclcpp/rclcpp.hpp>
#include <ackermann_msgs/msg/ackermann_drive_stamped.hpp>
#include <std_msgs/msg/float64.hpp>

class AckermannToTopicsNode : public rclcpp::Node
{
public:
    AckermannToTopicsNode() : Node("ackermann_to_topics_node")
    {
        // Declarar parámetros
        this->declare_parameter("input_topic", "ackermann_cmd");
        this->declare_parameter("steering_topic", "steering_angle");
        this->declare_parameter("velocity_topic", "velocity");
        this->declare_parameter("max_steering_angle", 0.52);  // ~30 grados en radianes
        this->declare_parameter("max_velocity", 2.0);  // m/s
        
        // Obtener parámetros
        std::string input_topic = this->get_parameter("input_topic").as_string();
        std::string steering_topic = this->get_parameter("steering_topic").as_string();
        std::string velocity_topic = this->get_parameter("velocity_topic").as_string();
        max_steering_angle_ = this->get_parameter("max_steering_angle").as_double();
        max_velocity_ = this->get_parameter("max_velocity").as_double();
        
        // Crear suscriptor
        ackermann_sub_ = this->create_subscription<ackermann_msgs::msg::AckermannDriveStamped>(
            input_topic, 10,
            std::bind(&AckermannToTopicsNode::ackermannCallback, this, std::placeholders::_1));
        
        // Crear publicadores
        steering_pub_ = this->create_publisher<std_msgs::msg::Float64>(steering_topic, 10);
        velocity_pub_ = this->create_publisher<std_msgs::msg::Float64>(velocity_topic, 10);
        
        RCLCPP_INFO(this->get_logger(), "Ackermann to Topics Node initialized");
        RCLCPP_INFO(this->get_logger(), "  Input topic: %s", input_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Steering topic: %s", steering_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Velocity topic: %s", velocity_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Max steering angle: %.2f rad (%.2f deg)", 
                    max_steering_angle_, max_steering_angle_ * 180.0 / M_PI);
        RCLCPP_INFO(this->get_logger(), "  Max velocity: %.2f m/s", max_velocity_);
    }

private:
    void ackermannCallback(const ackermann_msgs::msg::AckermannDriveStamped::SharedPtr msg)
    {
        // Extraer steering angle
        std_msgs::msg::Float64 steering_msg;
        steering_msg.data = msg->drive.steering_angle;
        
        // Limitar el ángulo de dirección
        if (steering_msg.data > max_steering_angle_) {
            steering_msg.data = max_steering_angle_;
        } else if (steering_msg.data < -max_steering_angle_) {
            steering_msg.data = -max_steering_angle_;
        }
        
        // Extraer velocidad
        std_msgs::msg::Float64 velocity_msg;
        velocity_msg.data = msg->drive.speed;
        
        // Limitar la velocidad
        if (velocity_msg.data > max_velocity_) {
            velocity_msg.data = max_velocity_;
        } else if (velocity_msg.data < -max_velocity_) {
            velocity_msg.data = -max_velocity_;
        }
        
        // Publicar
        steering_pub_->publish(steering_msg);
        velocity_pub_->publish(velocity_msg);
        
        // Log (opcional, comentar en producción para mejor rendimiento)
        // RCLCPP_DEBUG(this->get_logger(), "Steering: %.3f rad, Velocity: %.3f m/s", 
        //              steering_msg.data, velocity_msg.data);
    }
    
    rclcpp::Subscription<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr ackermann_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr steering_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr velocity_pub_;
    
    double max_steering_angle_;
    double max_velocity_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<AckermannToTopicsNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}