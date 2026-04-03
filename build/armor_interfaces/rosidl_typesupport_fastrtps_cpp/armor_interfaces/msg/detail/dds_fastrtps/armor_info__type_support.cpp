// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__type_support.cpp.em
// with input from armor_interfaces:msg/ArmorInfo.idl
// generated code does not contain a copyright notice
#include "armor_interfaces/msg/detail/armor_info__rosidl_typesupport_fastrtps_cpp.hpp"
#include "armor_interfaces/msg/detail/armor_info__struct.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_fastrtps_cpp/wstring_conversion.hpp"
#include "fastcdr/Cdr.h"


// forward declaration of message dependencies and their conversion functions

namespace armor_interfaces
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_armor_interfaces
cdr_serialize(
  const armor_interfaces::msg::ArmorInfo & ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Member: yaw
  cdr << ros_message.yaw;
  // Member: time
  cdr << ros_message.time;
  return true;
}

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_armor_interfaces
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  armor_interfaces::msg::ArmorInfo & ros_message)
{
  // Member: yaw
  cdr >> ros_message.yaw;

  // Member: time
  cdr >> ros_message.time;

  return true;
}  // NOLINT(readability/fn_size)

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_armor_interfaces
get_serialized_size(
  const armor_interfaces::msg::ArmorInfo & ros_message,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Member: yaw
  {
    size_t item_size = sizeof(ros_message.yaw);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: time
  {
    size_t item_size = sizeof(ros_message.time);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_armor_interfaces
max_serialized_size_ArmorInfo(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;


  // Member: yaw
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }

  // Member: time
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = armor_interfaces::msg::ArmorInfo;
    is_plain =
      (
      offsetof(DataType, time) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static bool _ArmorInfo__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  auto typed_message =
    static_cast<const armor_interfaces::msg::ArmorInfo *>(
    untyped_ros_message);
  return cdr_serialize(*typed_message, cdr);
}

static bool _ArmorInfo__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  auto typed_message =
    static_cast<armor_interfaces::msg::ArmorInfo *>(
    untyped_ros_message);
  return cdr_deserialize(cdr, *typed_message);
}

static uint32_t _ArmorInfo__get_serialized_size(
  const void * untyped_ros_message)
{
  auto typed_message =
    static_cast<const armor_interfaces::msg::ArmorInfo *>(
    untyped_ros_message);
  return static_cast<uint32_t>(get_serialized_size(*typed_message, 0));
}

static size_t _ArmorInfo__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_ArmorInfo(full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}

static message_type_support_callbacks_t _ArmorInfo__callbacks = {
  "armor_interfaces::msg",
  "ArmorInfo",
  _ArmorInfo__cdr_serialize,
  _ArmorInfo__cdr_deserialize,
  _ArmorInfo__get_serialized_size,
  _ArmorInfo__max_serialized_size
};

static rosidl_message_type_support_t _ArmorInfo__handle = {
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
  &_ArmorInfo__callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace armor_interfaces

namespace rosidl_typesupport_fastrtps_cpp
{

template<>
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_EXPORT_armor_interfaces
const rosidl_message_type_support_t *
get_message_type_support_handle<armor_interfaces::msg::ArmorInfo>()
{
  return &armor_interfaces::msg::typesupport_fastrtps_cpp::_ArmorInfo__handle;
}

}  // namespace rosidl_typesupport_fastrtps_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, armor_interfaces, msg, ArmorInfo)() {
  return &armor_interfaces::msg::typesupport_fastrtps_cpp::_ArmorInfo__handle;
}

#ifdef __cplusplus
}
#endif
