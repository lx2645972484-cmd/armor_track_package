// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from armor_interfaces:msg/SerialReceiveData.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_H_
#define ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/SerialReceiveData in the package armor_interfaces.
typedef struct armor_interfaces__msg__SerialReceiveData
{
  double yaw;
  double pitch;
} armor_interfaces__msg__SerialReceiveData;

// Struct for a sequence of armor_interfaces__msg__SerialReceiveData.
typedef struct armor_interfaces__msg__SerialReceiveData__Sequence
{
  armor_interfaces__msg__SerialReceiveData * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} armor_interfaces__msg__SerialReceiveData__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_H_
