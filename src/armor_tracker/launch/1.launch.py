from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import os

def generate_launch_description():
    package_share = get_package_share_directory("armor_tracker_node")

    return LaunchDescription(
        [
            # ===== armor_tracker_node =====
            Node(
                package="armor_tracker_node",
                executable="armor_tracker_node",
                name="armor_tracker_node",
                output="screen",
                parameters=[
                    {'setting': 0}   
                ]
            ),
            
            # ===== armor_drawer =====
            Node(
                package="armor_drawer",
                executable="armor_drawer",
                name="armor_drawer",
                output="screen",
            ),

            # ===== rm_serial_driver 节点 (修正 stop_bits 类型) =====
            Node(
                package="rm_serial_driver",
                executable="rm_serial_driver_node",
                name="rm_serial_driver",
                output="screen",
                parameters=[
                    {
                        "device_name": "/dev/ttyACM0",
                        "baud_rate": 115200,
                        "flow_control": "none",
                        "parity": "none",
                        "stop_bits": "1"   # 修正为字符串类型
                    }
                ],
            )
        ]
    )