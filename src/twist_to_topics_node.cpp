#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/float64.hpp>

class TwistToTopicsNode : public rclcpp::Node
{
public:
    TwistToTopicsNode() : Node("twist_to_topics_node")
    {
        // Declarar parámetros
        this->declare_parameter("input_topic", "cmd_vel");
        this->declare_parameter("steering_topic", "steering_angle");
        this->declare_parameter("velocity_topic", "velocity");
        this->declare_parameter("max_steering_angle", 0.52);  // ~30 grados en radianes
        this->declare_parameter("max_velocity", 2.0);  // m/s
        this->declare_parameter("wheelbase", 1.0);  // Distancia entre ejes en metros
        this->declare_parameter("use_ackermann_conversion", true);  // Convertir angular.z a steering angle
        
        // Obtener parámetros
        std::string input_topic = this->get_parameter("input_topic").as_string();
        std::string steering_topic = this->get_parameter("steering_topic").as_string();
        std::string velocity_topic = this->get_parameter("velocity_topic").as_string();
        max_steering_angle_ = this->get_parameter("max_steering_angle").as_double();
        max_velocity_ = this->get_parameter("max_velocity").as_double();
        wheelbase_ = this->get_parameter("wheelbase").as_double();
        use_ackermann_conversion_ = this->get_parameter("use_ackermann_conversion").as_bool();
        
        // Crear suscriptor
        twist_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            input_topic, 10,
            std::bind(&TwistToTopicsNode::twistCallback, this, std::placeholders::_1));
        
        // Crear publicadores
        steering_pub_ = this->create_publisher<std_msgs::msg::Float64>(steering_topic, 10);
        velocity_pub_ = this->create_publisher<std_msgs::msg::Float64>(velocity_topic, 10);
        
        RCLCPP_INFO(this->get_logger(), "Twist to Topics Node initialized");
        RCLCPP_INFO(this->get_logger(), "  Input topic: %s", input_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Steering topic: %s", steering_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Velocity topic: %s", velocity_topic.c_str());
        RCLCPP_INFO(this->get_logger(), "  Max steering angle: %.2f rad (%.2f deg)", 
                    max_steering_angle_, max_steering_angle_ * 180.0 / M_PI);
        RCLCPP_INFO(this->get_logger(), "  Max velocity: %.2f m/s", max_velocity_);
        RCLCPP_INFO(this->get_logger(), "  Wheelbase: %.2f m", wheelbase_);
        RCLCPP_INFO(this->get_logger(), "  Ackermann conversion: %s", 
                    use_ackermann_conversion_ ? "enabled" : "disabled");
    }

private:
    void twistCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        // Extraer velocidad lineal
        std_msgs::msg::Float64 velocity_msg;
        velocity_msg.data = msg->linear.x;
        
        // Limitar la velocidad
        if (velocity_msg.data > max_velocity_) {
            velocity_msg.data = max_velocity_;
        } else if (velocity_msg.data < -max_velocity_) {
            velocity_msg.data = -max_velocity_;
        }
        
        // Calcular steering angle
        std_msgs::msg::Float64 steering_msg;
        
        if (use_ackermann_conversion_) {
            // Conversión de velocidad angular a ángulo de dirección Ackermann
            // steering_angle = atan(angular_velocity * wheelbase / linear_velocity)
            if (std::abs(velocity_msg.data) > 0.01) {  // Evitar división por cero
                steering_msg.data = std::atan(msg->angular.z * wheelbase_ / velocity_msg.data);
            } else {
                // Si la velocidad es muy baja, usar directamente angular.z como aproximación
                steering_msg.data = msg->angular.z;
            }
        } else {
            // Usar directamente angular.z como steering angle
            steering_msg.data = msg->angular.z;
        }
        
        // Limitar el ángulo de dirección
        if (steering_msg.data > max_steering_angle_) {
            steering_msg.data = max_steering_angle_;
        } else if (steering_msg.data < -max_steering_angle_) {
            steering_msg.data = -max_steering_angle_;
        }
        
        // Publicar
        steering_pub_->publish(steering_msg);
        velocity_pub_->publish(velocity_msg);
        
        // Log (opcional, comentar en producción para mejor rendimiento)
        // RCLCPP_DEBUG(this->get_logger(), "Steering: %.3f rad, Velocity: %.3f m/s", 
        //              steering_msg.data, velocity_msg.data);
    }
    
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr steering_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr velocity_pub_;
    
    double max_steering_angle_;
    double max_velocity_;
    double wheelbase_;
    bool use_ackermann_conversion_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TwistToTopicsNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}