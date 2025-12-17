cmake_minimum_required(VERSION 3.8)
project(caddy_ai2_ros2_teleop_radio_esp32_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
#find_package(ackermann_msgs REQUIRED)
#find_package(std_msgs REQUIRED)
#find_package(geometry_msgs REQUIRED)

# Añadir el ejecutable
#add_executable(ackermann_to_topics_node src/ackermann_to_topics_node.cpp)
#add_executable(twist_to_topics_node src/twist_to_topics_node.cpp)

# Añadir dependencias al ejecutable
#ament_target_dependencies(ackermann_to_topics_node 
#  rclcpp 
#  ackermann_msgs 
#  std_msgs
#)

#ament_target_dependencies(twist_to_topics_node 
#  rclcpp 
#  geometry_msgs 
#  std_msgs
#)

# Instalar el ejecutable
#install(TARGETS
  #ackermann_to_topics_node
  #twist_to_topics_node
  #DESTINATION lib/${PROJECT_NAME}
#)

# Instalar directorios de launch y config
install(DIRECTORY bringup
  DESTINATION share/${PROJECT_NAME}/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()