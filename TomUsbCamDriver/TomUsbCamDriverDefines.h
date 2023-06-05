
#ifndef _TOM_USB_CAM_DRIVER_DEFINES_H
#define _TOM_USB_CAM_DRIVER_DEFINES_H

// All of the defined bit values that are used in Usb packets.

// Helpful websites and documents:
// [1]: https://www.beyondlogic.org/usbnutshell/usb6.shtml#StandardDeviceRequests
// [2]: https://www.usbmadesimple.co.uk/ums_4.htm
// [3]: "UVC 1.5 Class specification.pdf"
// [4]: "USB_Video_Example 1.5.pdf"

// GET_DESCRIPTOR for Vid/Pid per [1]
#define	GetStandardDescriptorRequest 0x6
#define HostToDeviceDataPhaseTransferDirectionRequestType 0x0 << 7
#define DeviceToHostDataPhaseTransferDirectionRequestType 0x1 << 7
#define StandardTypeRequestType 0x0 << 5
#define ClassTypeRequestType 0x1 << 5
#define VendorTypeRequestType 0x2 << 5
#define DeviceRecipientRequestType 0x0
#define InterfaceRecipientRequestType 0x1
#define EndpointRecipientRequestType 0x2
// High byte defines the type of descriptor to access. 0x1 refers to "Device" per [2].
#define GetDescriptorDeviceValue 0x1 << 8
#define GetDescriptorConfigurationValue 0x2 << 8
#define GetDescriptorProcessingUnitValue 0x24

// Device descriptor is 18 bytes, per [2]
#define GetDeviceDescriptorPacketLen 0x12

#define FiveSecTimeoutInMsecs 0x1388

// See [3] (P.88 & P.111) & [4] (P.26)
// to fill in "SET_*" messages. Appendix tables are in [3].
// See "Table 4-2 Get Request" in [3] for Get() defines.
#define	SetCurrentSelectorControlRequest 0x1

// SET_INTERFACE per [2].
#define	SetInterfaceRequest 0xb
#define	GetCurrentSelectorControlRequest 0x81
#define	GetMinSelectorControlRequest 0x82
#define	GetMaxSelectorControlRequest 0x83
#define	GetResolutionSelectorControlRequest 0x84
#define	GetLenSelectorControlRequest 0x85
#define	GetInfoSelectorControlRequest 0x86
#define	GetDefaultSelectorControlRequest 0x87
#define ZeroBandwidthInterfaceValue 0x0
#define OperationalInterfaceValue 0x1
#define BrightnessValue 0x200
#define ContrastValue 0x300
#define HueValue 0x600
#define SelectorOutputTerminalIndex 0x3 << 8
#define SelectorProcessingUnitIndex 0x5 << 8
#define InterfaceVideoControlIndex 0x0
#define InterfaceVideoStreamingIndex 0x1
#define InterfaceProcessingUnitIndex 0x5

#define BrightnessControlPacketLen 0x2
#define ContrastControlPacketLen 0x2
#define HueControlPacketLen 0x2

#define DescriptorTypeDevice 0x1
#define DescriptorTypeConfiguration 0x2
#define DescriptorTypeInterface 0x4
#define DescriptorTypeEndpoint 0x5
#define DescriptorTypeVideoInterface 0x24

#endif
