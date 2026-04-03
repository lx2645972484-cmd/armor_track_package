// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from armor_interfaces:msg/SerialDriver.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__BUILDER_HPP_
#define ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "armor_interfaces/msg/detail/serial_driver__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace armor_interfaces
{

namespace msg
{

namespace builder
{

class Init_SerialDriver_symbol
{
public:
  explicit Init_SerialDriver_symbol(::armor_interfaces::msg::SerialDriver & msg)
  : msg_(msg)
  {}
  ::armor_interfaces::msg::SerialDriver symbol(::armor_interfaces::msg::SerialDriver::_symbol_type arg)
  {
    msg_.symbol = std::move(arg);
    return std::move(msg_);
  }

private:
  ::armor_interfaces::msg::SerialDriver msg_;
};

class Init_SerialDriver_pitch
{
public:
  explicit Init_SerialDriver_pitch(::armor_interfaces::msg::SerialDriver & msg)
  : msg_(msg)
  {}
  Init_SerialDriver_symbol pitch(::armor_interfaces::msg::SerialDriver::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return Init_SerialDriver_symbol(msg_);
  }

private:
  ::armor_interfaces::msg::SerialDriver msg_;
};

class Init_SerialDriver_yaw
{
public:
  Init_SerialDriver_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SerialDriver_pitch yaw(::armor_interfaces::msg::SerialDriver::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_SerialDriver_pitch(msg_);
  }

private:
  ::armor_interfaces::msg::SerialDriver msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::armor_interfaces::msg::SerialDriver>()
{
  return armor_interfaces::msg::builder::Init_SerialDriver_yaw();
}

}  // namespace armor_interfaces

#endif  // ARMOR_INTERFACES__MSG__DETAIL__SERIAL_DRIVER__BUILDER_HPP_
