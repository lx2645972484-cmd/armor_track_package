# from ament_index_python.packages import get_package_share_directory
# from launch import LaunchDescription
# from launch_ros.actions import Node
# from launch.substitutions import PathJoinSubstitution
# from launch_ros.substitutions import FindPackageShare
# import os

# def generate_launch_description():
#     package_share = get_package_share_directory("armor_tracker_node")

#     return LaunchDescription(
#         [
#             # ===== armor_tracker_node =====
#             Node(
#                 package="armor_tracker_node",
#                 executable="armor_tracker_node",
#                 name="armor_tracker_node",
#                 output="screen",
#                 parameters=[
#                     {'setting': 0}   
#                 ]
#             ),
            
#             # ===== armor_drawer =====
#             Node(
#                 package="armor_drawer",
#                 executable="armor_drawer",
#                 name="armor_drawer",
#                 output="screen",
#             ),

#             # ===== rm_serial_driver 节点 (修正 stop_bits 类型) =====
#             Node(
#                 package="rm_serial_driver",
#                 executable="rm_serial_driver_node",
#                 name="rm_serial_driver",
#                 output="screen",
#                 parameters=[
#                     {
#                         "device_name": "/dev/ttyACM0",
#                         "baud_rate": 115200,
#                         "flow_control": "none",
#                         "parity": "none",
#                         "stop_bits": "1"   # 修正为字符串类型
#                     }
#                 ],
#             )
#         ]
#     )

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.parameter_descriptions import ParameterValue
import os

def generate_launch_description():
    # 获取 armor_tracker_node 包路径（用于业务节点）
    tracker_pkg_share = get_package_share_directory("armor_tracker_node")
    # 获取 armor_urdf 包路径（用于 URDF 模型文件）
    urdf_pkg_share = get_package_share_directory("armor_urdf")

    # ========== 模型路径参数 ==========
    default_model_path = os.path.join(urdf_pkg_share, "urdf", "armor_urdf.urdf")
    model_arg = DeclareLaunchArgument(
        name="model",
        default_value=default_model_path,
        description="Absolute path to robot URDF file"
    )

    # ========== robot_state_publisher ==========
    # 使用 cat 命令直接读取 .urdf 文件（如果是 .xacro 文件则需替换为 xacro 命令）
    robot_description_content = Command(["cat ", LaunchConfiguration("model")])
    robot_description = ParameterValue(robot_description_content, value_type=str)
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="robot_state_publisher",
        output="screen",
        parameters=[{"robot_description": robot_description}]
    )

    # ========== joint_state_publisher ==========
    # 如果你的模型全是固定关节，此节点不是必需的，但保留也无妨
    joint_state_publisher_node = Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
        name="joint_state_publisher",
        output="screen"
    )

    # ========== RViz2 ==========
    # rviz2_node = Node(
    #     package="rviz2",
    #     executable="rviz2",
    #     name="rviz2",
    #     output="screen"
    #     # 如需自动加载配置文件，可添加 arguments=["-d", "/path/to/your.rviz"]
    # )

    # ========== 原有业务节点（来自 armor_tracker_node 包） ==========
    armor_tracker_node = Node(
        package="armor_tracker_node",
        executable="armor_tracker_node",
        name="armor_tracker_node",
        output="screen",
        parameters=[{'setting': 0}]
    )

    armor_drawer_node = Node(
        package="armor_drawer",
        executable="armor_drawer",
        name="armor_drawer",
        output="screen"
    )

    rm_serial_driver_node = Node(
        package="rm_serial_driver",
        executable="rm_serial_driver_node",
        name="rm_serial_driver",
        output="screen",
        parameters=[{
            "device_name": "/dev/ttyACM0",
            "baud_rate": 115200,
            "flow_control": "none",
            "parity": "none",
            "stop_bits": "1"
        }]
    )

    return LaunchDescription([
        model_arg,
        robot_state_publisher_node,
        joint_state_publisher_node,
        # rviz2_node,
        armor_tracker_node,
        armor_drawer_node,
        rm_serial_driver_node,
    ])