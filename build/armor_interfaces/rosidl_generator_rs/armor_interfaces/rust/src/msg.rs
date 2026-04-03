#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to armor_interfaces__msg__ArmorInfo

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::ArmorInfo::default())
  }
}

impl rosidl_runtime_rs::Message for ArmorInfo {
  type RmwMsg = super::msg::rmw::ArmorInfo;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        yaw: msg.yaw,
        time: msg.time,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      yaw: msg.yaw,
      time: msg.time,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      yaw: msg.yaw,
      time: msg.time,
    }
  }
}


// Corresponds to armor_interfaces__msg__SerialDriver

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::SerialDriver::default())
  }
}

impl rosidl_runtime_rs::Message for SerialDriver {
  type RmwMsg = super::msg::rmw::SerialDriver;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        yaw: msg.yaw,
        pitch: msg.pitch,
        symbol: msg.symbol,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      yaw: msg.yaw,
      pitch: msg.pitch,
      symbol: msg.symbol,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      yaw: msg.yaw,
      pitch: msg.pitch,
      symbol: msg.symbol,
    }
  }
}


