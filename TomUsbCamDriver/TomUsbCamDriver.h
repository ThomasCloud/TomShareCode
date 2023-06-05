
#ifndef _TOM_USB_CAM_DRIVER_H
#define _TOM_USB_CAM_DRIVER_H

// Tom: writing driver to talk to the microscoping camera used for Pcb population.

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/usb.h>
#include <linux/uaccess.h>

// V4l2 headers
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>


#include <media/videobuf2-v4l2.h>

#include <media/videobuf2-dma-contig.h>



//Is this needed still?
#include <linux/dma-mapping.h>


// Everything is declared "static" to prevent it being used outside of this object file's scope. See:
// https://stackoverflow.com/questions/7259830/why-and-when-to-use-static-structures-in-c-programming
// I guess this prevents namespace collisions?

// Forward declare these functions so they can be refernced in the file_operations & usb_driver structs
static ssize_t TomUsbCamRead(struct file*, char __user*, size_t, loff_t*);
static ssize_t TomUsbCamWrite(struct file*, const char __user*, size_t, loff_t*);
static int TomUsbCamOpen(struct inode*, struct file*);
static int TomUsbCamRelease(struct inode*, struct file*);
static int TomUsbCamIoctl(struct usb_interface*, unsigned int, void*);

// Probe & disconnect are called automatically when the device is plugged/unplugged.
// These functions are called for each interface, i.e. for both the control and isochronous interface.
static int TomUsbCamProbe(struct usb_interface*, const struct usb_device_id*);
static void TomUsbCamDisconnect(struct usb_interface*);

// Add these here only as forward delarations within the .c file.
static void TomUsbCamCtrlIntfDelete(struct kref *);
static void TomUsbCamIsochronousInputDelete(struct kref *);

// This struct is defined below. Declare here for the enable/disable functions below.
struct TomUsbCamCtrlIntfDevStruct;

// Each device is laid out in a tree with descending associations, possibly many-to-1:
// Device -> Configuration -> Interface -> Endpoint. Some interfaces (e.g. VideolInterface)
// seem to be subinterfaces to other interfaces. Sometimes interfaces have the same number, but are
// differentiated by their "alternate setting" number. Make sure the endpoints keep track of both 
// their interaces and their respective interface alternate setting numbers to avoid confusion.
struct ConfigurationDescriptorStruct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t bMaxPower;
};

struct InterfaceDescriptorStruct
{
    uint8_t ConfigurationAssoc;
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
};

struct EndpointDescriptorStruct
{
    uint8_t ConfigurationAssoc;
    uint8_t ParentInterfaceAssoc;
    uint8_t AlternateSettingAssoc;
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
};

// Only the 1st 3 fields are consistent for the VideoInterface descriptors. 
// VideoInterface's are classified as interfaces, but seem to be subinterfaces.
struct VideoInterfaceDescriptorStruct
{
    uint8_t ConfigurationAssoc;
    uint8_t ParentInterfaceAssoc;
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t *VarData;
};

// V4l2-specific functions
static int TomUsbCamSetV4l2Control(struct v4l2_ctrl *);
static int TomUsbCamQueryCapability(struct file *, void *, struct v4l2_capability *);
static int TomUsbCamTryFormat(struct file *, void *, struct v4l2_format *);
static int TomUsbCamSetFormat(struct file *, void *, struct v4l2_format *);
static int TomUsbCamGetFormat(struct file *, void *, struct v4l2_format *);
static int WriteToCamera(struct usb_device *, __u8 , __u8 , __u8 , __u16 , __u16 , __u16 , unsigned char *, __u16 , int );
static int ReadFromCamera(struct usb_device *, __u8 , __u8 , __u8 , __u16 , __u16 , __u16 , unsigned char *, __u16 , int );
static int QueryCameraFactoryValues(struct usb_device *, __u16, __u16, unsigned char *, int *, int *, int *, int *, bool);
static int EnableCamera(struct usb_device *);
static int DisableCamera(struct usb_device *);
static int SaveAllDescriptors(struct TomUsbCamCtrlIntfDevStruct *);
static void GetConfigurationDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *, uint8_t, struct ConfigurationDescriptorStruct **, int8_t *);
static void GetInterfaceDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *, uint8_t, struct InterfaceDescriptorStruct **, int8_t *);
static void GetEndpointDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *, uint8_t, uint8_t, struct EndpointDescriptorStruct **, int8_t *);
static void GetVideoInterfaceDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *, uint8_t, uint8_t, struct VideoInterfaceDescriptorStruct **, int8_t *);
static int TomUsbCamV4l2QueueSetup(struct vb2_queue *, unsigned int *, unsigned int *,
		                           unsigned int[], struct device *[]);
//Tom is forward declaration of arrays above correct?		                           
//static int TomUsbCamV4l2QueueSetup(struct vb2_queue *VideoBufferQueue,
//		                           unsigned int *NumBuffers, unsigned int *NumImagePlanes,
//		                           unsigned int ImageSizes[], struct device *alloc_devs[]);
static int buffer_prepare(struct vb2_buffer *);
static void buffer_queue(struct vb2_buffer *);
static int start_streaming(struct vb2_queue *, unsigned int);
static void stop_streaming(struct vb2_queue *);

static struct v4l2_file_operations TomUsbCamV4l2FileOps;
		                           

// Cubeternet/Etron Technology Id values for the "USB2.0 Camera". The 0x1e4e (cubeternet) is the device we want.
// The other device, 0x04F2 is for the Chicony camera in the Lenovo laptop.
#define TOM_USB_CAM_VENDOR_ID 0x1E4E
#define TOM_USB_CAM_PRODUCT_ID	0x0109
//#define TOM_USB_CAM_VENDOR_ID 0x04F2
//#define TOM_USB_CAM_PRODUCT_ID 0xB39A

// Table of devices that work with this driver. Include a blank terminating device struct.
// This is used by the hotplug system.
static struct usb_device_id TomUsbCamTable [] = 
{
	{USB_DEVICE(TOM_USB_CAM_VENDOR_ID, TOM_USB_CAM_PRODUCT_ID)},
	{}
};

// Tom get a minor range for your devices from the usb maintainer 
#define TOM_USB_CAM_MINOR_BASE 192

// Structure to hold all of our device specific info.
// An isochrounous input interface is preferred for streaming applications because
// it is lossy and low-latency, as opposed to a bulk interface where data reception is guaranteed.
// Use separate structs for both the control and isochronous interfaces since the driver
// functions are called individually for each interface.
struct TomUsbCamCtrlIntfDevStruct 
{
	struct usb_device *UsbDevStructPtr;
	struct usb_interface *UsbDevInterfaceStructPtr;
	signed char *CtrlIntfBuffer;
	size_t CtrlIntfBufferSize;
	__u8 CtrlIntfEndpointAlternateAddr;
	struct kref KernelRefCountStruct;
	
	// Top-level v4l2 device
	struct v4l2_device V4l2DevStruct;

    // Keeps track of all the controls, I guess for ioctl() calls?
	struct v4l2_ctrl_handler V4l2CtrlHandler;
	
	struct v4l2_pix_format V4l2PixFormatStruct;
	
	// This needs to be here so the proper V4l2 functions can be assigned in the driver probe() section.
	struct video_device VideoDevice;
	
	// Add a lock that the v4l2 class can use.
	struct mutex TomUsbCamLock;
	
	// A queue used for the video frames?
	struct vb2_queue TomUsbCamV4l2Queue;
	
	// Keep track of the all the various descriptors describing this camera.
	struct UsbDescriptorsStruct
	{
	
	    // Keep track of the counts to avoid having to constantly re-querying the device.
	    uint8_t ConfigurationDescriptorStructCount;
	    uint8_t InterfaceDescriptorStructCount;
	    uint8_t EndpointDescriptorStructCount;
	    uint8_t VideoDescriptorStructCount;
	
	    struct ConfigurationDescriptorStruct *ConfigurationDescriptorStructPtr;
	    struct InterfaceDescriptorStruct *InterfaceDescriptorStructPtr;
	    struct EndpointDescriptorStruct *EndpointDescriptorStructPtr;
	    struct VideoInterfaceDescriptorStruct *VideoInterfaceDescriptorStructPtr;
	} 
	UsbDescriptorsForThisCameraStruct;
	
	// Make sure the camera can support these changes by querying the various descriptors.
	bool HueChangeSupported;
    bool ContrastChangeSupported;
    bool BrightnessChangeSupported;
};

struct TomUsbCamIsochronousInputDevStruct 
{
	struct usb_device *UsbDevStructPtr;
	struct usb_interface *UsbDevInterfaceStructPtr;
	unsigned char *IsochronousInputBuffer;
	size_t IsochronousInputBufferSize;
	__u8 IsochronousInputEndpointAddr;
	struct kref KernelRefCountStruct;
	
	//struct v4l2_device v4l2_dev;
};

// This struct might not be necessary since the v4l2_file_operations struct is charge of file operations.
static struct file_operations TomUsbCamFileOps = 
{
	.owner =   THIS_MODULE,
	.read =    TomUsbCamRead,
	.write =   TomUsbCamWrite,
	.open =    TomUsbCamOpen,
	.release = TomUsbCamRelease,
};

// Usb class driver info in order to get a minor number from the usb core,
// register with devfs, and register with the driver core.
// Indicate how the device will show up under the /dev dir.
static struct usb_class_driver TomUsbCamClass = 
{

    // It seems the name of the video device must just be "video" for the v4l2-ctl to recognize it as
    // a video device. See https://github.com/cz172638/v4l-utils/blob/master/utils/v4l2-ctl/v4l2-ctl.cpp, line
    // 1175.
	.name = "usb/video%d",
	//.fops = &TomUsbCamV4l2FileOps,
	.fops = &TomUsbCamFileOps,
	.minor_base = TOM_USB_CAM_MINOR_BASE,
};

// Specify how the driver will show up under /sys/bus/usb/drivers/, the devices supported, and
// the probe and disconnect functions that are automatically called.
static struct usb_driver TomUsbCamDriver = 
{
	.name = "TomUsbCam",
	.id_table = TomUsbCamTable,
	.probe = TomUsbCamProbe,
	.disconnect = TomUsbCamDisconnect,
	//.unlocked_ioctl = TomUsbCamIoctl,
};

// Specify what function should handle the V4l2 control requests from user space
static struct v4l2_ctrl_ops TomUsbCamV4l2ControlOps = 
{
	.s_ctrl = TomUsbCamSetV4l2Control,
};

// Specify what function should handle the ioctl() requests user space. I'm not sure
// why this wasn't combined with v4l2_ctrl_ops, since it seems the user only accesses
// v4l2_ctrl_ops through ioctl() calls.
// All available ioctl calls are here:
// https://01.org/linuxgraphics/gfx-docs/drm/media/kapi/v4l2-common.html
static struct v4l2_ioctl_ops TomUsbCamV4l2IoctlOps = 
{
	.vidioc_querycap = TomUsbCamQueryCapability,
	.vidioc_try_fmt_vid_cap = TomUsbCamTryFormat,
	.vidioc_s_fmt_vid_cap = TomUsbCamSetFormat,
	.vidioc_g_fmt_vid_cap = TomUsbCamGetFormat,
};

// Specify all the available file operations on this v4l2 device. The structure is defined here:
// https://docs.huihoo.com/doxygen/linux/kernel/3.7/structv4l2__file__operations.html
// Use the standard open/close methods defined here:
// https://dri.freedesktop.org/docs/drm/media/kapi/v4l2-fh.html
// and here:
// https://01.org/linuxgraphics/gfx-docs/drm/media/kapi/v4l2-common.html
static struct v4l2_file_operations TomUsbCamV4l2FileOps = 
{
	.owner =   THIS_MODULE,
	.open =    v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.read =    vb2_fop_read,
	.mmap =   vb2_fop_mmap,
	
	// I'm not sure if the poll() function is necessary.
	.poll = vb2_fop_poll,
	
	// I'm not sure why this generic ioctl handler is used when more specific ioctl handlers are 
	// used elsewhere, e.g. TomUsbCamQueryCapability
	.unlocked_ioctl = video_ioctl2,
};

// Define all the operations that can occur on the queue.
// An overview of the videobuf2 Api is here:
// https://lwn.net/Articles/447435/
// The vb2 structs are defined here:
// https://www.kernel.org/doc/html/v4.13/media/kapi/v4l2-videobuf2.html
static struct vb2_ops TomUsbCamV4l2QueueOps = 
{

	.queue_setup		= TomUsbCamV4l2QueueSetup,
	.buf_prepare		= buffer_prepare,
	.buf_queue		    = buffer_queue,
	.start_streaming	= start_streaming,
	.stop_streaming		= stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

// Set up the buffer that will be used by V4l2 for video frames.
struct TomUsbCamV4l2VideoBufferContainer 
{
	struct vb2_v4l2_buffer TomUsbCamV4l2VideoBuffer;
	struct list_head TomUsbCamV4l2VideoBufferListHead;
};

#endif
