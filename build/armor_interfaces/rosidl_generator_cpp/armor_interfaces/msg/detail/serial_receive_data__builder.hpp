// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from armor_interfaces:msg/SerialReceiveData.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__BUILDER_HPP_
#define ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "armor_interfaces/msg/detail/serial_receive_data__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace armor_interfaces
{

namespace msg
{

namespace builder
{

class Init_SerialReceiveData_pitch
{
public:
  explicit Init_SerialReceiveData_pitch(::armor_interfaces::msg::SerialReceiveData & msg)
  : msg_(msg)
  {}
  ::armor_interfaces::msg::SerialReceiveData pitch(::armor_interfaces::msg::SerialReceiveData::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return std::move(msg_);
  }

private:
  ::armor_interfaces::msg::SerialReceiveData msg_;
};

class Init_SerialReceiveData_yaw
{
public:
  Init_SerialReceiveData_yaw()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SerialReceiveData_pitch yaw(::armor_interfaces::msg::SerialReceiveData::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_SerialReceiveData_pitch(msg_);
  }

private:
  ::armor_interfaces::msg::SerialReceiveData msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::armor_interfaces::msg::SerialReceiveData>()
{
  return armor_interfaces::msg::builder::Init_SerialReceiveData_yaw();
}

}  // namespace armor_interfaces

#endif  // ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__BUILDER_HPP_
