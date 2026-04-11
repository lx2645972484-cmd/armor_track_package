// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from armor_interfaces:msg/JointState.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__STRUCT_H_
#define ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'name'
#include "rosidl_runtime_c/string.h"
// Member 'position'
// Member 'velocity'
// Member 'effort'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/JointState in the package armor_interfaces.
typedef struct armor_interfaces__msg__JointState
{
  /// 时间戳和坐标系ID
  std_msgs__msg__Header header;
  /// 关节名称列表
  rosidl_runtime_c__String__Sequence name;
  /// 关节位置（旋转关节为弧度，平移关节为米）
  rosidl_runtime_c__double__Sequence position;
  /// 关节速度（弧度/秒 或 米/秒）
  rosidl_runtime_c__double__Sequence velocity;
  /// 关节力矩或力（Nm 或 N）
  rosidl_runtime_c__double__Sequence effort;
} armor_interfaces__msg__JointState;

// Struct for a sequence of armor_interfaces__msg__JointState.
typedef struct armor_interfaces__msg__JointState__Sequence
{
  armor_interfaces__msg__JointState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} armor_interfaces__msg__JointState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__STRUCT_H_
