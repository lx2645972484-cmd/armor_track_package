// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from armor_interfaces:msg/SerialReceiveData.idl
// generated code does not contain a copyright notice

#ifndef ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_
#define ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__armor_interfaces__msg__SerialReceiveData __attribute__((deprecated))
#else
# define DEPRECATED__armor_interfaces__msg__SerialReceiveData __declspec(deprecated)
#endif

namespace armor_interfaces
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct SerialReceiveData_
{
  using Type = SerialReceiveData_<ContainerAllocator>;

  explicit SerialReceiveData_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->yaw = 0.0;
      this->pitch = 0.0;
    }
  }

  explicit SerialReceiveData_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->yaw = 0.0;
      this->pitch = 0.0;
    }
  }

  // field types and members
  using _yaw_type =
    double;
  _yaw_type yaw;
  using _pitch_type =
    double;
  _pitch_type pitch;

  // setters for named parameter idiom
  Type & set__yaw(
    const double & _arg)
  {
    this->yaw = _arg;
    return *this;
  }
  Type & set__pitch(
    const double & _arg)
  {
    this->pitch = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> *;
  using ConstRawPtr =
    const armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__armor_interfaces__msg__SerialReceiveData
    std::shared_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__armor_interfaces__msg__SerialReceiveData
    std::shared_ptr<armor_interfaces::msg::SerialReceiveData_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const SerialReceiveData_ & other) const
  {
    if (this->yaw != other.yaw) {
      return false;
    }
    if (this->pitch != other.pitch) {
      return false;
    }
    return true;
  }
  bool operator!=(const SerialReceiveData_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct SerialReceiveData_

// alias to use template instance with default allocator
using SerialReceiveData =
  armor_interfaces::msg::SerialReceiveData_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace armor_interfaces

#endif  // ARMOR_INTERFACES__MSG__DETAIL__SERIAL_RECEIVE_DATA__STRUCT_HPP_
