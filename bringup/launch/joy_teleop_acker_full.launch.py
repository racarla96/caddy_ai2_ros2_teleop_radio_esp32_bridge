import os

from ament_index_python.packages import get_package_share_directory

import launch
import launch_ros.actions
from launch.actions import DeclareLaunchArgument, ExecuteProcess, RegisterEventHandler, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch.event_handlers import OnProcessExit


def launch_docker_setup(context, *args, **kwargs):
    # Get the actual values from LaunchConfiguration
    usb_device = LaunchConfiguration('usb_device').perform(context)
    ros_distro = LaunchConfiguration('ros_distro').perform(context)

    # Command to stop any running micro-ROS agent containers
    docker_rm_command = ExecuteProcess(
        cmd=['bash', '-c', 
             f'docker ps --quiet --filter ancestor=microros/micro-ros-agent:{ros_distro} | xargs -r docker stop || true'],
        shell=False,
        output='screen',
        name='stop_previous_agent'
    )

    # Command to run the Docker container
    docker_command = ExecuteProcess(
        cmd=['bash', '-c', 
             f'docker run --rm -v /dev:/dev -v /dev/shm:/dev/shm --privileged --net=host ' +
             f'microros/micro-ros-agent:{ros_distro} serial --dev {usb_device}'],
        shell=False,
        output='screen',
        name='micro_ros_agent'
    )

    return [
        docker_rm_command,
        RegisterEventHandler(
            OnProcessExit(
                target_action=docker_rm_command,
                on_exit=[docker_command],
            )
        )
    ]


def generate_launch_description():
    joy_config = launch.substitutions.LaunchConfiguration('joy_config')
    config_filepath = launch.substitutions.LaunchConfiguration('config_filepath')
    ackermann_config_filepath = launch.substitutions.LaunchConfiguration('ackermann_config_filepath')

    usb_device_dir = LaunchConfiguration('usb_device', default='/dev/ttyUSB0')
    ros_distro_dir = LaunchConfiguration('ros_distro', default=os.environ.get('ROS_DISTRO', 'jazzy'))

    return launch.LaunchDescription([
        launch.actions.DeclareLaunchArgument('joy_vel', default_value='ackermann_cmd'), 
        launch.actions.DeclareLaunchArgument('joy_config', default_value='joy_teleop_acker'),       
        launch.actions.DeclareLaunchArgument('config_filepath', default_value=[
            launch.substitutions.TextSubstitution(text=os.path.join(
                get_package_share_directory('caddy_ai2_ros2_teleop_radio_esp32_bridge'), 'config', '')),
            joy_config, launch.substitutions.TextSubstitution(text='.yaml')]),
        
        launch.actions.DeclareLaunchArgument('ackermann_config_filepath', default_value=[
            launch.substitutions.TextSubstitution(text=os.path.join(
                get_package_share_directory('caddy_ai2_ros2_teleop_radio_esp32_bridge'), 'config', 'ackermann_to_topics.yaml'))]),

        DeclareLaunchArgument(
            'usb_device',
            default_value=usb_device_dir,
            description='USB device for micro-ROS agent'),

        DeclareLaunchArgument(
            'ros_distro',
            default_value=ros_distro_dir,
            description='ROS 2 distribution'),

        # Lanzar el Docker con micro-ROS
        OpaqueFunction(function=launch_docker_setup),

        # Nodo de teleop usando teleop_twist_joy estándar
        launch_ros.actions.Node(
            package='teleop_twist_joy', 
            executable='teleop_node',
            name='teleop_twist_joy_node',
            parameters=[config_filepath],
        ),
        
        # Nodo convertidor de Ackermann a topics individuales
        launch_ros.actions.Node(
            package='caddy_ai2_ros2_teleop_radio_esp32_bridge',
            executable='ackermann_to_topics_node',
            name='ackermann_to_topics_node',
            parameters=[ackermann_config_filepath],
            remappings=[
                ('ackermann_cmd', launch.substitutions.LaunchConfiguration('joy_vel')),
            ],
            output='screen',
        ),
    ])