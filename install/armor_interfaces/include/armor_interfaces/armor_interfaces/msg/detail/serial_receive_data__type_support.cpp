// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from armor_interfaces:msg/SerialReceiveData.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "armor_interfaces/msg/detail/serial_receive_data__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace armor_interfaces
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void SerialReceiveData_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) armor_interfaces::msg::SerialReceiveData(_init);
}

void SerialReceiveData_fini_function(void * message_memory)
{
  auto typed_message = static_cast<armor_interfaces::msg::SerialReceiveData *>(message_memory);
  typed_message->~SerialReceiveData();
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember SerialReceiveData_message_member_array[2] = {
  {
    "yaw",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(armor_interfaces::msg::SerialReceiveData, yaw),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "pitch",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(armor_interfaces::msg::SerialReceiveData, pitch),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers SerialReceiveData_message_members = {
  "armor_interfaces::msg",  // message namespace
  "SerialReceiveData",  // message name
  2,  // number of fields
  sizeof(armor_interfaces::msg::SerialReceiveData),
  SerialReceiveData_message_member_array,  // message members
  SerialReceiveData_init_function,  // function to initialize message memory (memory has to be allocated)
  SerialReceiveData_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t SerialReceiveData_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &SerialReceiveData_message_members,
  get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace armor_interfaces


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<armor_interfaces::msg::SerialReceiveData>()
{
  return &::armor_interfaces::msg::rosidl_typesupport_introspection_cpp::SerialReceiveData_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, armor_interfaces, msg, SerialReceiveData)() {
  return &::armor_interfaces::msg::rosidl_typesupport_introspection_cpp::SerialReceiveData_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
