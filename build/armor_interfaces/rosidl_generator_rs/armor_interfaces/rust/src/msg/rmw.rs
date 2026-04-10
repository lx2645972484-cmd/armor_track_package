#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "armor_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__ArmorInfo() -> *const std::ffi::c_void;
}

#[link(name = "armor_interfaces__rosidl_generator_c")]
extern "C" {
    fn armor_interfaces__msg__ArmorInfo__init(msg: *mut ArmorInfo) -> bool;
    fn armor_interfaces__msg__ArmorInfo__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<ArmorInfo>, size: usize) -> bool;
    fn armor_interfaces__msg__ArmorInfo__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<ArmorInfo>);
    fn armor_interfaces__msg__ArmorInfo__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<ArmorInfo>, out_seq: *mut rosidl_runtime_rs::Sequence<ArmorInfo>) -> bool;
}

// Corresponds to armor_interfaces__msg__ArmorInfo
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct ArmorInfo {

    // This member is not documented.
    #[allow(missing_docs)]
    pub yaw: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub time: f64,

}



impl Default for ArmorInfo {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !armor_interfaces__msg__ArmorInfo__init(&mut msg as *mut _) {
        panic!("Call to armor_interfaces__msg__ArmorInfo__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for ArmorInfo {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__ArmorInfo__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__ArmorInfo__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__ArmorInfo__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for ArmorInfo {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for ArmorInfo where Self: Sized {
  const TYPE_NAME: &'static str = "armor_interfaces/msg/ArmorInfo";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__ArmorInfo() }
  }
}


#[link(name = "armor_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__SerialDriver() -> *const std::ffi::c_void;
}

#[link(name = "armor_interfaces__rosidl_generator_c")]
extern "C" {
    fn armor_interfaces__msg__SerialDriver__init(msg: *mut SerialDriver) -> bool;
    fn armor_interfaces__msg__SerialDriver__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<SerialDriver>, size: usize) -> bool;
    fn armor_interfaces__msg__SerialDriver__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<SerialDriver>);
    fn armor_interfaces__msg__SerialDriver__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<SerialDriver>, out_seq: *mut rosidl_runtime_rs::Sequence<SerialDriver>) -> bool;
}

// Corresponds to armor_interfaces__msg__SerialDriver
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct SerialDriver {

    // This member is not documented.
    #[allow(missing_docs)]
    pub yaw: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub pitch: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub symbol: i32,

}



impl Default for SerialDriver {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !armor_interfaces__msg__SerialDriver__init(&mut msg as *mut _) {
        panic!("Call to armor_interfaces__msg__SerialDriver__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for SerialDriver {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialDriver__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialDriver__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialDriver__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for SerialDriver {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for SerialDriver where Self: Sized {
  const TYPE_NAME: &'static str = "armor_interfaces/msg/SerialDriver";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__SerialDriver() }
  }
}


#[link(name = "armor_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__JointState() -> *const std::ffi::c_void;
}

#[link(name = "armor_interfaces__rosidl_generator_c")]
extern "C" {
    fn armor_interfaces__msg__JointState__init(msg: *mut JointState) -> bool;
    fn armor_interfaces__msg__JointState__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<JointState>, size: usize) -> bool;
    fn armor_interfaces__msg__JointState__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<JointState>);
    fn armor_interfaces__msg__JointState__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<JointState>, out_seq: *mut rosidl_runtime_rs::Sequence<JointState>) -> bool;
}

// Corresponds to armor_interfaces__msg__JointState
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct JointState {
    /// 时间戳和坐标系ID
    pub header: std_msgs::msg::rmw::Header,

    /// 关节名称列表
    pub name: rosidl_runtime_rs::Sequence<rosidl_runtime_rs::String>,

    /// 关节位置（旋转关节为弧度，平移关节为米）
    pub position: rosidl_runtime_rs::Sequence<f64>,

    /// 关节速度（弧度/秒 或 米/秒）
    pub velocity: rosidl_runtime_rs::Sequence<f64>,

    /// 关节力矩或力（Nm 或 N）
    pub effort: rosidl_runtime_rs::Sequence<f64>,

}



impl Default for JointState {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !armor_interfaces__msg__JointState__init(&mut msg as *mut _) {
        panic!("Call to armor_interfaces__msg__JointState__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for JointState {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__JointState__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__JointState__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__JointState__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for JointState {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for JointState where Self: Sized {
  const TYPE_NAME: &'static str = "armor_interfaces/msg/JointState";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__JointState() }
  }
}


#[link(name = "armor_interfaces__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__SerialReceiveData() -> *const std::ffi::c_void;
}

#[link(name = "armor_interfaces__rosidl_generator_c")]
extern "C" {
    fn armor_interfaces__msg__SerialReceiveData__init(msg: *mut SerialReceiveData) -> bool;
    fn armor_interfaces__msg__SerialReceiveData__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<SerialReceiveData>, size: usize) -> bool;
    fn armor_interfaces__msg__SerialReceiveData__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<SerialReceiveData>);
    fn armor_interfaces__msg__SerialReceiveData__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<SerialReceiveData>, out_seq: *mut rosidl_runtime_rs::Sequence<SerialReceiveData>) -> bool;
}

// Corresponds to armor_interfaces__msg__SerialReceiveData
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct SerialReceiveData {

    // This member is not documented.
    #[allow(missing_docs)]
    pub yaw: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub pitch: f64,

}



impl Default for SerialReceiveData {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !armor_interfaces__msg__SerialReceiveData__init(&mut msg as *mut _) {
        panic!("Call to armor_interfaces__msg__SerialReceiveData__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for SerialReceiveData {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialReceiveData__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialReceiveData__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { armor_interfaces__msg__SerialReceiveData__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for SerialReceiveData {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for SerialReceiveData where Self: Sized {
  const TYPE_NAME: &'static str = "armor_interfaces/msg/SerialReceiveData";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__armor_interfaces__msg__SerialReceiveData() }
  }
}


