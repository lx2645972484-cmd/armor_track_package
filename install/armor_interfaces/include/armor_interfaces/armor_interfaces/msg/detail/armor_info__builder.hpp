// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from armor_interfaces:msg/ArmorInfo.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__ARMOR_INFO__BUILDER_HPP_
#define ARMOR_INTERFACES__MSG__DETAIL__ARMOR_INFO__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "armor_interfaces/msg/detail/armor_info__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace armor_interfaces
{

namespace msg
{

namespace builder
{

class Init_ArmorInfo_time
{
public:
  explicit Init_ArmorInfo_time(::armor_interfaces::msg::ArmorInfo & msg)
  : msg_(msg)
  {}
  ::armor_interfaces::msg::ArmorInfo time(::armor_interfaces::msg::ArmorInfo::_time_type arg)
  {
    msg_.time = std::move(arg);
    return std::move(msg_);
  }

private:
  ::armor_interfaces::msg::ArmorInfo msg_;
};

class Init_ArmorInfo_yaw
{
public:
  Init_ArmorInfo_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ArmorInfo_time yaw(::armor_interfaces::msg::ArmorInfo::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_ArmorInfo_time(msg_);
  }

private:
  ::armor_interfaces::msg::ArmorInfo msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::armor_interfaces::msg::ArmorInfo>()
{
  return armor_interfaces::msg::builder::Init_ArmorInfo_yaw();
}

}  // namespace armor_interfaces

#endif  // ARMOR_INTERFACES__MSG__DETAIL__ARMOR_INFO__BUILDER_HPP_
