import os

from ament_index_python.packages import get_package_share_directory

import launch
import launch_ros.actions
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    joy_config = launch.substitutions.LaunchConfiguration('joy_config')
    config_filepath = launch.substitutions.LaunchConfiguration('config_filepath')

    joy_launch_file_dir = os.path.join(get_package_share_directory('caddy_ai2_ros2_teleop_radio_esp32_bridge'), 'launch')

    usb_device_dir = LaunchConfiguration('usb_device', default='/dev/ttyUSB1')
    ros_distro_dir = LaunchConfiguration('ros_distro', default=os.environ.get('ROS_DISTRO', 'jazzy'))

    return launch.LaunchDescription([
        launch.actions.DeclareLaunchArgument('joy_vel', default_value='cmd_vel'), 
        launch.actions.DeclareLaunchArgument('joy_config', default_value='joy_teleop_twist'),       
        launch.actions.DeclareLaunchArgument('config_filepath', default_value=[
            launch.substitutions.TextSubstitution(text=os.path.join(
                get_package_share_directory('caddy_ai2_ros2_teleop_radio_esp32_bridge'), 'bringup', 'config', '')),
            joy_config, launch.substitutions.TextSubstitution(text='.yaml')]),

        DeclareLaunchArgument(
            'usb_device',
            default_value=usb_device_dir,
            description='USB device for micro-ROS agent'),

        DeclareLaunchArgument(
            'ros_distro',
            default_value=ros_distro_dir,
            description='ROS 2 distribution'),

        # Incluir el joy_bridge.launch.py que lanza el Docker con micro-ROS
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource([joy_launch_file_dir, '/joy_bridge.launch.py']),
            launch_arguments={
                'usb_device': usb_device_dir,
                'ros_distro': ros_distro_dir
            }.items(),
        ),

        # Usar el nodo estándar de teleop_twist_joy
        launch_ros.actions.Node(
            package='teleop_twist_joy', 
            executable='teleop_node',
            name='teleop_twist_joy_node',
            parameters=[config_filepath],
            remappings=[
                ('/cmd_vel', '/bicycle_steering_controller/reference')
            ]
        ),
    ])
