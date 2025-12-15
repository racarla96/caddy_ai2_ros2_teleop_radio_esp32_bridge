from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, RegisterEventHandler, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch.event_handlers import OnProcessExit
import os

def launch_setup(context, *args, **kwargs):
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
    # Declare launch arguments
    declare_usb_device_arg = DeclareLaunchArgument(
        'usb_device',
        default_value='/dev/ttyUSB0',
        description='USB device for micro-ROS agent'
    )
    
    declare_ros_distro_arg = DeclareLaunchArgument(
        'ros_distro',
        default_value=os.environ.get('ROS_DISTRO', 'jazzy'),
        description='ROS 2 distribution'
    )

    return LaunchDescription([
        declare_usb_device_arg,
        declare_ros_distro_arg,
        OpaqueFunction(function=launch_setup)
    ])