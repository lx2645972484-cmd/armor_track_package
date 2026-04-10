// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from armor_interfaces:msg/JointState.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__BUILDER_HPP_
#define ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "armor_interfaces/msg/detail/joint_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace armor_interfaces
{

namespace msg
{

namespace builder
{

class Init_JointState_effort
{
public:
  explicit Init_JointState_effort(::armor_interfaces::msg::JointState & msg)
  : msg_(msg)
  {}
  ::armor_interfaces::msg::JointState effort(::armor_interfaces::msg::JointState::_effort_type arg)
  {
    msg_.effort = std::move(arg);
    return std::move(msg_);
  }

private:
  ::armor_interfaces::msg::JointState msg_;
};

class Init_JointState_velocity
{
public:
  explicit Init_JointState_velocity(::armor_interfaces::msg::JointState & msg)
  : msg_(msg)
  {}
  Init_JointState_effort velocity(::armor_interfaces::msg::JointState::_velocity_type arg)
  {
    msg_.velocity = std::move(arg);
    return Init_JointState_effort(msg_);
  }

private:
  ::armor_interfaces::msg::JointState msg_;
};

class Init_JointState_position
{
public:
  explicit Init_JointState_position(::armor_interfaces::msg::JointState & msg)
  : msg_(msg)
  {}
  Init_JointState_velocity position(::armor_interfaces::msg::JointState::_position_type arg)
  {
    msg_.position = std::move(arg);
    return Init_JointState_velocity(msg_);
  }

private:
  ::armor_interfaces::msg::JointState msg_;
};

class Init_JointState_name
{
public:
  explicit Init_JointState_name(::armor_interfaces::msg::JointState & msg)
  : msg_(msg)
  {}
  Init_JointState_position name(::armor_interfaces::msg::JointState::_name_type arg)
  {
    msg_.name = std::move(arg);
    return Init_JointState_position(msg_);
  }

private:
  ::armor_interfaces::msg::JointState msg_;
};

class Init_JointState_header
{
public:
  Init_JointState_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_JointState_name header(::armor_interfaces::msg::JointState::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_JointState_name(msg_);
  }

private:
  ::armor_interfaces::msg::JointState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::armor_interfaces::msg::JointState>()
{
  return armor_interfaces::msg::builder::Init_JointState_header();
}

}  // namespace armor_interfaces

#endif  // ARMOR_INTERFACES__MSG__DETAIL__JOINT_STATE__BUILDER_HPP_
