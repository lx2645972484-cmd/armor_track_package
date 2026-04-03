// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from armor_interfaces:msg/SerialDriver.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__STRUCT_H_
#define ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/SerialDriver in the package armor_interfaces.
typedef struct armor_interfaces__msg__SerialDriver
{
  double yaw;
  double pitch;
  int32_t symbol;
} armor_interfaces__msg__SerialDriver;

// Struct for a sequence of armor_interfaces__msg__SerialDriver.
typedef struct armor_interfaces__msg__SerialDriver__Sequence
{
  armor_interfaces__msg__SerialDriver * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} armor_interfaces__msg__SerialDriver__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__STRUCT_H_
