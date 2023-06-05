
// Writing a test driver to replace the standard Uvc driver. Use with the Cubeternet
// microscope camera.
#include "TomUsbCamDriver.h"
#include "TomUsbCamDriverDefines.h"

MODULE_DESCRIPTION("Test V4l2 Usb driver");
MODULE_AUTHOR("Tom Cloud");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE (usb, TomUsbCamTable);

// Helpful sites:
// [1]: http://www.cs.albany.edu/~sdc/CSI500/linux-2.6.31.14/Documentation/DocBook/usb/re18.html
// [2]: https://elixir.bootlin.com/linux/latest/source/include/linux/usb.h
// [3]: https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/usb/ch9.h#L389
// [4]: https://www.usbmadesimple.co.uk/ums_4.htm
// [5]: "UVC 1.5 Class specification.pdf"
// [6]: "USB_Video_Example 1.5.pdf"

// Tom this is a stub for now
static ssize_t TomUsbCamRead(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    return 0;
}

// Tom this is a stub for now
static ssize_t TomUsbCamWrite(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
{
    return 0;
}

// Tom this is a stub for now
static int TomUsbCamOpen(struct inode *inode, struct file *file)
{
    return 0;
}

// Tom this is a stub for now
static int TomUsbCamRelease(struct inode *inode, struct file *file)
{
    return 0;
}

// Tom this is a stub for now
static int TomUsbCamIoctl(struct usb_interface *UsbDevInterfaceStructPtr, unsigned int code, void *buf)
{
    return -1;
}

// This probe() function is automatically called when the kernel sees a device plugged in that matches this driver.
// Alternatively, if the Usb dev/port descriptor id is piped into /sys/bus/usb/drivers/<driver name>/bind this function
// is called.
// Probe() can be used to save basic data about the device but should run as quick as possible so it doesn't slow other
// probe() functions running on the same thread. More in-depth data should be saved in the Open() function when the
// user actually tries to use the device. See p. 350 of Ldd for more details.
static int TomUsbCamProbe(struct usb_interface *UsbDevInterfaceStructPtr, const struct usb_device_id *id)
{

    // Since this probe function is called for both the control and isochronous interfaces, figure out which
    // one we are dealing with before proceeding.
    int NumOfAltSettingsForThisIntf = UsbDevInterfaceStructPtr->num_altsetting;
    
    
    // The control interface always has a default address of 0x0. There seem to be 2 other interfaces 
    // for this (maybe all?) camera. One is 0x83 which seems to support control, isochronous, interrupt, 
    // and bulk transfers. I beleive this is for sending low-speed still frame captures. The other interface, 0x81,
    // seems to be the streaming interface. Since this probe function is called for each of these 2 capture interfaces,
    // use the low-speed interface call to set up the control interface, which is used to pass camera configuration
    // commands back and forth over Usb. The low-speed interface call can be recognized because it only has 1 alternative
    // Usb setting.  
    bool IntfIsForCtrl = (NumOfAltSettingsForThisIntf > 1) ? false : true;

    int DeviceProbeSuccessStatus = -ENOMEM;
	
	bool MemoryAllocatedForDev = false, CtrlIntfBufferAllocated = false, IsochronousInBufferAllocated = false,
         CorrectIsochronousIntfFound = false;
	
	// Declare both of these structs but only init the one pertinent to this interface.
	struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = NULL;
	struct TomUsbCamIsochronousInputDevStruct *TomUsbCamIsochronousInputDevStructPtr = NULL;	

    // Allocate memory for our camera struct and set it to 0. Returns null on failure.
    if (IntfIsForCtrl)
    {
    
	    TomUsbCamCtrlIntfDevStructPtr = kzalloc(sizeof(struct TomUsbCamCtrlIntfDevStruct), GFP_KERNEL);
	    
	    // Any positive address indicates the allocation succeeded
	    if (TomUsbCamCtrlIntfDevStructPtr)
	    {
	        MemoryAllocatedForDev = true;	        
	    }
	    else
	    {

	        // pr_err is an alias to call printk() with a certain logging level. See: 
	        // https://elinux.org/Debugging_by_printing
		    pr_err("TomUsbCamProbe error: TomUsbCamCtrlIntfDevStructPtr allocation failed");
	    }
    }
    else
    {
	    
	    
//tom skip the isoc intf until the ctrl intf is working with v4l2	    
return -1;	    
	    
	    
	    
	    TomUsbCamIsochronousInputDevStructPtr = kzalloc(sizeof(struct TomUsbCamIsochronousInputDevStruct), GFP_KERNEL);
	    
	    // Any positive address indicates the allocation succeeded
	    if (TomUsbCamIsochronousInputDevStructPtr)
	    {
	        MemoryAllocatedForDev = true;
	    }
	    else
	    {

	        // pr_err is an alias to call printk() with a certain logging level. See: 
	        // https://elinux.org/Debugging_by_printing
		    pr_err("TomUsbCamProbe error: TomUsbCamIsochronousInputDevStructPtr allocation failed");
	    }
	}

	if (MemoryAllocatedForDev)
	{
	    
        if (IntfIsForCtrl)
        {
	    
	        // Seve the usb device and interface structures for later
	        TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr = usb_get_dev(interface_to_usbdev(UsbDevInterfaceStructPtr));

	        // UsbDevInterfaceStructPtr = struct usb_interface defined at: [1]
	        TomUsbCamCtrlIntfDevStructPtr->UsbDevInterfaceStructPtr = UsbDevInterfaceStructPtr;	        	    
        	    
	    }
	    else
	    {
	    
	        TomUsbCamIsochronousInputDevStructPtr->UsbDevStructPtr = usb_get_dev(interface_to_usbdev(UsbDevInterfaceStructPtr));

	        TomUsbCamIsochronousInputDevStructPtr->UsbDevInterfaceStructPtr = UsbDevInterfaceStructPtr;	
	    }
	    
	    // Used for debug printing below
	    int SelectedAltSettingIdx = 0;
	    
	    // Retain the interface alternate setting pointer for selecting the proper endpoint for initialization below.
	    // cur_altsetting = struct usb_host_interface defined at line 81 here: [2]
	    struct usb_host_interface *UsbIntfPtr = UsbDevInterfaceStructPtr->cur_altsetting;	 	    
	           
        // Try to send & receive a message to the device to verify this is the default control interface
        if (IntfIsForCtrl)
        {

            // Use usb_get_descriptor(), usb_control_msg(), and an Urb to get the Vid/Pid as a test.

            // usb_get_descriptor() method
            // ------------------------------
            
            // The kref seems to be a global reference count for this driver. If a single device is plugged in
            // and multiple programs use it, the reference count will be updated accordingly. If multiple
            // devices are plugged in, I think they all get their own instance of this driver. It seems that
            // both the sochronous and control interfaces will use their own reference counts.
            kref_init(&TomUsbCamCtrlIntfDevStructPtr->KernelRefCountStruct);	            
        
            // Allocate space for the temporary descriptor before using it
            // See examples #2 & #4 here:
            // https://cpp.hotexamples.com/examples/-/-/usb_get_descriptor/cpp-usb_get_descriptor-function-examples.html
            struct usb_device_descriptor *TempDescriptorPtr = kzalloc(sizeof(*TempDescriptorPtr), GFP_NOIO);
            
            if (!TempDescriptorPtr)
            {
               pr_err("TomUsbCamProbe error: TempDescriptorPtr allocation failed");
            }               
                       
            // usb_get_descriptor() is the the rx/tx test to make sure this is actually the control interface.
            // If this were a unidirection isochronous streaming interface, no data should be returned, or it should be garbage.
            int usb_get_descriptorBytesRead = usb_get_descriptor(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, USB_DT_DEVICE, 0, 
                                                                 TempDescriptorPtr, sizeof(*TempDescriptorPtr));            
           
            pr_info("TomUsbCamProbe control interface data:\n");           
           
            if (usb_get_descriptorBytesRead > 0)
            {
            
                pr_info("usb_get_descriptor Vid/Pid: 0x%04x/0x%04x, expected = 0x1e4e/0x0109.", 
                        TempDescriptorPtr->idVendor, 
                        TempDescriptorPtr->idProduct);            
            }
            else
            {
                pr_info("TomUsbCamProbe incorrect control interface choosen");
            }

            kfree(TempDescriptorPtr);   
            
            // usb_control_msg() method
            // ------------------------------

            // Allocate 18 bytes for a Pid/Vid query per:
            // https://libusbk.sourceforge.net/UsbK3/struct_u_s_b___d_e_v_i_c_e___d_e_s_c_r_i_p_t_o_r.html
            // usb_get_descriptor is a wrapper around a usb_control_msg() call per:
            // https://elixir.bootlin.com/linux/latest/source/drivers/usb/core/message.c#L780
            // Formulate query per:
            // https://www.beyondlogic.org/usbnutshell/usb6.shtml#StandardDeviceRequests
            // Kernel-allocated memory must be used or else the control message fails per the usbmon utility.
            unsigned char *UsbPacketDataPtr = kzalloc(18, GFP_NOIO);
            
            ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                           GetStandardDescriptorRequest,
                           StandardTypeRequestType,
                           DeviceRecipientRequestType,
                           GetDescriptorDeviceValue,
                           0x0,
                           0x0,
                           UsbPacketDataPtr,
                           GetDeviceDescriptorPacketLen,
                           FiveSecTimeoutInMsecs);            
              
            pr_info("usb_control_msg Vid/Pid: 0x%04x/0x%04x, expected = 0x1e4e/0x0109.", 
                    UsbPacketDataPtr[9] << 8 | UsbPacketDataPtr[8], 
                    UsbPacketDataPtr[11] << 8 | UsbPacketDataPtr[10]);

            kfree(UsbPacketDataPtr);
            
            // Get all of the Usb descriptors for use later on. The descriptors describe the 
            // entire Usb device architecture.
            SaveAllDescriptors(TomUsbCamCtrlIntfDevStructPtr);
            
            // Urb method
            // ------------------------------
            
            /*
	            //Tom use an Urb here, see:
	            //https://github.com/torvalds/linux/blob/5bfc75d92efd494db37f5c4c173d3639d4772966/drivers/media/usb/gspca/konica.c
	            // Pass 0 as the number of "iso packets" since it is being used to communicate with a control endpoint. See:
	            // https://manpages.debian.org/testing/linux-manual-4.9/usb_alloc_urb.9
	            struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);
	            
	            urb->transfer_buffer = usb_alloc_coherent(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, 
	                                                      4,
	                                                      GFP_KERNEL,
	                                                      &urb->transfer_dma);
	            
	            pr_info("***urb->transfer_buffer: %p***", 
                        urb->transfer_buffer);
                   
                  
                //Tom use usb_fill_bulk_urb() instead of a usb_control_msg() to stay with the "Urb" methodology here
                //tom see s2255_write_config for retrieving results of this setting? 	                                             
	            
	            
	            usb_fill_bulk_urb(UsbRequestBlockPtr,
	                              TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                                  usb_sndbulkpipe(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, 
	                                              TomUsbCamCtrlIntfDevStructPtr->CtrlIntfEndpointAddr                        
                        	                                           
	                                            
	            int BytesRcvd = usb_control_msg(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
	                                            usb_rcvctrlpipe(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, 
                                                                DeviceToHostDataPhaseTransferDirectionRequestType),
	                                            0x00,
	                                            0xa3,
	                                            0x0000, //BrightnessValue,
	                                            0x0001, //SelectorOutputTerminalIndex,
	                                            urb->transfer_buffer,
	                                            0x0004,
	                                            1000);
	                                            
                usb_free_coherent(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, 
                                  4,
	                              urb->transfer_buffer,
	                              urb->transfer_dma);
	                              
                usb_free_urb(urb);	                                            	                                            	  	                                                                                      
	            */
        }
	     
        // If this interface has alternate settings, cycle through them to see what they support. For the usb camera
        // on interface 2, it seems like they are all input isochronous & interrupt endpoints with only the buffer size differing.
        // The buffer size of alternate setting 8 is 1023 bytes, which is the largest, so use this setting for now.
        else
        {
        
            kref_init(&TomUsbCamIsochronousInputDevStructPtr->KernelRefCountStruct);	        
        
            // Start with alternate setting index 0 even though this is the default
            for (int AltSettingIdx = 0; AltSettingIdx < NumOfAltSettingsForThisIntf; ++AltSettingIdx)
            {     
                
                // "0" is returned on success
                int InterfaceChangeReturnCode = usb_set_interface(TomUsbCamIsochronousInputDevStructPtr->UsbDevStructPtr, UsbIntfPtr->desc.bInterfaceNumber, AltSettingIdx);
                
                if (InterfaceChangeReturnCode)
                {
                    pr_err("TomUsbCamProbe error: InterfaceChangeReturnCode = %d", InterfaceChangeReturnCode);
                }
                else
                {
                    
                    UsbIntfPtr = UsbDevInterfaceStructPtr->cur_altsetting;
                    
                    // UsbIntfPtr->desc = struct usb_interface_descriptor defined at line 389 here: [3]
                    for (int EndPointNum = 0; EndPointNum < UsbIntfPtr->desc.bNumEndpoints; ++EndPointNum) 
                    {	

                        // UsbIntfPtr->endpoint = struct usb_host_endpoint defined at line 67 here: [2]
                        // &UsbIntfPtr->endpoint[i].desc = struct usb_endpoint_descriptor  defined at line 407 here: [3]
                        struct usb_endpoint_descriptor *TempEndPointPtr = &UsbIntfPtr->endpoint[EndPointNum].desc;
                        
                        size_t BufferSize = TempEndPointPtr->wMaxPacketSize;
                        
                        // 1023 bytes seems to be the largest isochronous interface buffer size for this camera. 
                        // Use this interface alternate setting for now. In total for this camera there are 8 
                        // isochronous settings with sizes (160, 208, 760, 780, 812, 976, 1020, & 1023 for alternative settings
                        // 1 through 8 respectively. UsbDevInterfaceStructPtr seems to retain the last interface setting.
                        if (BufferSize == 1023)
                        {
                            
                            CorrectIsochronousIntfFound = true;
                            SelectedAltSettingIdx = AltSettingIdx;
        
                            break;
                        }                
                    } 
                }
                
                if (CorrectIsochronousIntfFound)
                {
                    break;
                }
            }
        }

	    for (int EndPointNum = 0; EndPointNum < UsbIntfPtr->desc.bNumEndpoints; EndPointNum++) 
	    {	

	        struct usb_endpoint_descriptor *TempEndPointPtr = &UsbIntfPtr->endpoint[EndPointNum].desc;
	        
	        int OverrideAddr = 0;
	        
	        bool EndPointIsForInput = (TempEndPointPtr->bEndpointAddress & USB_DIR_IN);
	        bool EndPointIsForOutput = (TempEndPointPtr->bEndpointAddress & USB_DIR_OUT);
	        
	        bool EndPointIsIsochronous = ((TempEndPointPtr->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) & USB_ENDPOINT_XFER_ISOC);
	        bool EndPointIsInterrupt = ((TempEndPointPtr->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) & USB_ENDPOINT_XFER_INT);
	        bool EndPointIsBulk = ((TempEndPointPtr->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) & USB_ENDPOINT_XFER_BULK);

            size_t BufferSize = TempEndPointPtr->wMaxPacketSize;

	        pr_info("TomUsbCamProbe end point %d, address 0x%x, interface %d, type "
	                "(input/output/intf is for ctrl/isochronous/interrupt/bulk,buffer size): %d/%d/%d/%d/%d/%d/%zu", 
	                EndPointNum, TempEndPointPtr->bEndpointAddress, SelectedAltSettingIdx, EndPointIsForInput,
	                EndPointIsForOutput, IntfIsForCtrl, EndPointIsIsochronous, EndPointIsInterrupt, EndPointIsBulk, 
	                BufferSize);

            // Retain information for both the control interface and isochronous input interface
            if (IntfIsForCtrl)
            {
            
                pr_info("TomUsbCamProbe adding control interface");
                
                // Initialize the buffer that will be used for setting up the V4l2 interface.
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBufferSize = BufferSize;

                // Use the alternate interface address buffer size for now. It seems arbitrary. We really only pass
                // a few bytes back and forth at a time for the camera control.
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer = kzalloc(BufferSize, GFP_KERNEL);
                
                // If memory wasn't allocated, do not continue
    			if (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer) 
    			{
    			    CtrlIntfBufferAllocated = true;
			    }
    			else
    			{
		            pr_err("TomUsbCamProbe error: TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer allocation failed");
		            break;
		        }                 
            
                // Avoid adding the audio interface for now. 
                // Supporting bulk transfer seems to be the only initial difference between the interfaces right now.
                //if (!EndPointIsBulk)
                //{
                //    break;
                //}

                // Make sure the kernel and driver are using the same Dma size, see: 
                // https://www.kernel.org/doc/Documentation/DMA-API-HOWTO.txt
                //if (dma_set_mask(&TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr->dev, DMA_BIT_MASK(32)))
                //{
		        //    pr_err("TomUsbCamProbe error: dma_set_mask() failed");
		        //    break;                   
                //}
                //else
                //{
                //    pr_info("TomUsbCamProbe dma_set_mask worked!");
                //}

                // Pass in generic device defined here:
                // https://elixir.bootlin.com/linux/latest/source/include/linux/usb.h#L626
                // to v4l2_device_register() which will populate TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr->dev->driver_data 
                // and init V4l2DevStruct per:
                // https://www.kernel.org/doc/html/v4.13/media/kapi/v4l2-device.html#c.v4l2_device_register
                // &
                // https://www.kernel.org/doc/html/v4.13/driver-api/infrastructure.html#c.device
                // A non-zero value is returned upon failure.
                if (v4l2_device_register(&TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr->dev, 
                                         &TomUsbCamCtrlIntfDevStructPtr->V4l2DevStruct))
                {
		            pr_err("TomUsbCamProbe error: v4l2_device_register() failed");
		            break;                
                }
	        
	            // Init all the format fields for the frames grabbed from the webcam. See:
	            // https://linuxtv.org/downloads/legacy/video4linux/API/V4L2_API/spec/ch02.html#:~:text=The%20v4l2_pix_format%20structure%20defines%20the,buffer%20formats%20see%20also%20VIDIOC_G_FBUF%20.)
	            
	            int ImageWidth = 1280, ImageHeight = 720;
	            
	            // For every pixel there is 1 Y byte, and 0.5 U & V bytes, for an average of 2 bytes per pixel.
	            float YuyvScalingFactor = 2.0, BytesPerLine = ImageWidth * YuyvScalingFactor,
	                  ImageCompleteSizeInBytes = BytesPerLine * ImageHeight;
	            
	            TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.width = ImageWidth;
	            TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.height = ImageHeight;
	            
	            // Each u,v chrominance value applies to 2 luminance values in the horizontal direction.
	            TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.pixelformat = V4L2_PIX_FMT_YUYV;
	            
	            // Return the entire, non-interleaved, image.
	            TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.field = V4L2_FIELD_NONE;

                // Don't add any padding to image lines
                TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.bytesperline = (int) BytesPerLine;
	                    
                TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.sizeimage = (int) ImageCompleteSizeInBytes;

                // This seems to be for converting to Rgb, not sure why it is needed. I'm just randomly choosing this
                // setting for now.
                TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.colorspace = V4L2_COLORSPACE_SMPTE170M;
	                    
	            TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct.priv = 0;       
	                    
	            // Init the control handler for passing ioctl() controls.
	            // Give a hint as to how many controls this driver wants to export to user space for the user to manipulate.
	            // Possible controls are listed here: 
	            // https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/control.html
	            int NumberOfControlSettings = 3;
	            
	            v4l2_ctrl_handler_init(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler, NumberOfControlSettings);
	            
	            int Max, Min, Step, Default;
	            
                // Check which controls this specific camera actually supports. This information is contained
                // in the device descriptor data that was saved when the camera was plugged in.
                // Parent interface #0 is the control interface, the processing unit selector describes the supported
                // capabilites.
                uint8_t ParentInterfaceAssoc = 0, VideoInterfaceSubtype = InterfaceProcessingUnitIndex;
                int8_t DescriptorReadSuccess;

                struct VideoInterfaceDescriptorStruct *VideoInterfaceDescriptorStructPtr;

                GetVideoInterfaceDescriptorStruct(TomUsbCamCtrlIntfDevStructPtr, ParentInterfaceAssoc, VideoInterfaceSubtype,
                                                  &VideoInterfaceDescriptorStructPtr, &DescriptorReadSuccess);
	            
	            // Section 3.7.2.5 of [5] enumerates what the 2-byte bmControls value translates to. Update the supported
	            // features of this camera so the setters() can be correctly enabled later.
	            uint16_t CameraControlCapabilities = (uint16_t) (VideoInterfaceDescriptorStructPtr->VarData[6] << 8) |
                                                        VideoInterfaceDescriptorStructPtr->VarData[5];

                TomUsbCamCtrlIntfDevStructPtr->HueChangeSupported = CameraControlCapabilities & 0x4;
                TomUsbCamCtrlIntfDevStructPtr->ContrastChangeSupported = CameraControlCapabilities & 0x2;
                TomUsbCamCtrlIntfDevStructPtr->BrightnessChangeSupported = CameraControlCapabilities & 0x1;               

                if (TomUsbCamCtrlIntfDevStructPtr->HueChangeSupported)
                {
                
                    // Query the control values from the actual camera so they can be passed on to the V4l2 driver.               
                    QueryCameraFactoryValues(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, HueValue, BrightnessControlPacketLen,
                                             TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer, &Max, &Min, &Step, &Default, false);		            
           
	                // Last 4 parameters are s32 min, s32 max, u32 step, s32 default.
	                v4l2_ctrl_new_std(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler, &TomUsbCamV4l2ControlOps, 
	                                  V4L2_CID_HUE, Min, Max, Step, Default);
                }
                
                if (TomUsbCamCtrlIntfDevStructPtr->ContrastChangeSupported)
                {                
                    
                    QueryCameraFactoryValues(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, ContrastValue, BrightnessControlPacketLen,
                             TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer, &Max, &Min, &Step, &Default, true);	                    
	                                  
	                v4l2_ctrl_new_std(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler, &TomUsbCamV4l2ControlOps, 
	                                  V4L2_CID_CONTRAST, Min, Max, Step, Default);
                }

                if (TomUsbCamCtrlIntfDevStructPtr->BrightnessChangeSupported)
                {	                      
                
                    QueryCameraFactoryValues(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr, BrightnessValue, BrightnessControlPacketLen,
                                             TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer, &Max, &Min, &Step, &Default, true);	
 
	                v4l2_ctrl_new_std(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler, &TomUsbCamV4l2ControlOps,
			                          V4L2_CID_BRIGHTNESS, Min, Max, Step, Default);
                }	                              
          
	            if (TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler.error) 
	            {
        
                    int ErrorNumber = TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler.error;

                    v4l2_ctrl_handler_free(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler);
        
                    pr_err("TomUsbCamProbe error: v4l2_ctrl_handler error number: %d", ErrorNumber);
                }
            
                TomUsbCamCtrlIntfDevStructPtr->V4l2DevStruct.ctrl_handler = &TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler;

                // Init the v4l2 video image queue. This struct seems to take care of all the coordination for moving video images
                // through memory.
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                
                // Tom do I only need mmap? It doesn't appear Dma is supported right now because any vb2_dma* symbols are missing
                // in /proc/kallsyms .
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.io_modes = VB2_MMAP | 
                                                                             //VB2_DMABUF | 
                                                                             VB2_READ;
                                                                             
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.dev = &TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr->dev;
                
                // Tom Do I really need a pointer to this struct inside of the struct? I think "yes" because
                // the queue struct might be passed to a function, but the function wants access to the parent container.
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.drv_priv = TomUsbCamCtrlIntfDevStructPtr;
                
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.buf_struct_size = sizeof(struct TomUsbCamV4l2VideoBufferContainer);
                
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.ops = &TomUsbCamV4l2QueueOps;
                
                // The pci skeleton driver seemed to support Dma but since I am only supporting Mmap, trying using "vb2_common_vm_ops"
                // from here:
                // https://github.com/torvalds/linux/blob/master/drivers/media/common/videobuf2/videobuf2-memops.c
                // and here:
                // http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch14lev1sec2.html
                //TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.mem_ops = &vb2_dma_contig_memops;
                //TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.mem_ops = &vb2_common_vm_ops;
                // TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.mem_ops = &vb2_mmap;
                // TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.mem_ops = &mmap_mem_ops;
                
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
                
                // Ensure that at least 2 buffers are present before streaming can start.
                TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue.min_buffers_needed = 2;
                
                // Still need to add the "lock" for TomUsbCamV4l2Queue. See https://github.com/torvalds/linux/blob/master/samples/v4l/v4l2-pci-skeleton.c#L845
                // and comment for using a different lock for the usb device.
                
                
                
                
                // Init the video_device struct so this device is recognized as a legitimate v4l2 device.
                // Struct defined here:
                // https://elixir.bootlin.com/linux/latest/source/include/media/v4l2-dev.h#L263
                mutex_init(&TomUsbCamCtrlIntfDevStructPtr->TomUsbCamLock);
                
                __u8 DriverName[] = "TomUsbCam";
                strlcpy(TomUsbCamCtrlIntfDevStructPtr->VideoDevice.name, DriverName, 
                        sizeof(TomUsbCamCtrlIntfDevStructPtr->VideoDevice.name));
                
                // The video_device_release_empty() function is defined in v4l2_dev.h. It is a stub function when no release
                // activity needs to occur. The release function cannot be left unset though.
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.release = video_device_release_empty;
                
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.fops = &TomUsbCamV4l2FileOps;
                
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.ioctl_ops = &TomUsbCamV4l2IoctlOps;
                
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.device_caps = V4L2_CAP_VIDEO_CAPTURE | 
                                                                         V4L2_CAP_READWRITE |
                                                                         V4L2_CAP_AUDIO |
                                                                         V4L2_CAP_STREAMING;
                
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.lock = &TomUsbCamCtrlIntfDevStructPtr->TomUsbCamLock;

                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.queue = &TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue;
            
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.v4l2_dev = &TomUsbCamCtrlIntfDevStructPtr->V4l2DevStruct;
                
                // Save a pointer to the driver data container in the private pointer field so it can be recalled later.
                video_set_drvdata(&TomUsbCamCtrlIntfDevStructPtr->VideoDevice, TomUsbCamCtrlIntfDevStructPtr);
  
                // See https://www.kernel.org/doc/html/v4.9/media/kapi/v4l2-dev.html#video-device-debugging
                TomUsbCamCtrlIntfDevStructPtr->VideoDevice.dev_debug = 0x1f;

                // Save this alternate interface address. It might be the low-speed image address.
                // The control interface address is known to always default to 0x0.            
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfEndpointAlternateAddr = TempEndPointPtr->bEndpointAddress;             
            }
            else
            {
   
                if (EndPointIsForInput)
                {

                    // Only the video interface will have the desired 1023-byte interface. 
                    // The audio interface max size is 64 bytes.
                    if (!CorrectIsochronousIntfFound)
                    {
                        pr_err("TomUsbCamProbe ignoring audio interface for now");
                        break;
                    }

                    pr_info("TomUsbCamProbe adding isochronouse input interface");
                
                    TomUsbCamIsochronousInputDevStructPtr->IsochronousInputBufferSize = BufferSize;
                    
                    TomUsbCamIsochronousInputDevStructPtr->IsochronousInputEndpointAddr = TempEndPointPtr->bEndpointAddress;
                    
                    TomUsbCamIsochronousInputDevStructPtr->IsochronousInputBuffer = kmalloc(BufferSize, GFP_KERNEL);
                    
                    // If memory wasn't allocated, do not continue
        			if (TomUsbCamIsochronousInputDevStructPtr->IsochronousInputBuffer) 
        			{
        			    IsochronousInBufferAllocated = true;
    			    }
        			else
        			{
			            pr_err("TomUsbCamProbe error: TomUsbCamIsochronousInputDevStructPtr->IsochronousInputBuffer allocation failed");
			            break;
			        }
    			} 
            }
	    }
	    
	    // Register each of the interfaces. Only save the reference to TomUsbCamIsochronousInputDevStructPtr one time, 
	    // otherwise this action is redundant.
        if (IntfIsForCtrl)
        {

	        if (CtrlIntfBufferAllocated == true)
	        {

                // Register the v4l2 video device instead of the Usb device.
                // Use "VFL_TYPE_GRABBER" instead of "VFL_TYPE_VIDEO" per:
                // https://github.com/Isaac-Lozano/GV-USB2-Driver/issues/2
                // and 
                // https://lwn.net/Articles/204545/
                // The last "-1" indicates to use the first available minor number.
                DeviceProbeSuccessStatus = video_register_device(&TomUsbCamCtrlIntfDevStructPtr->VideoDevice, VFL_TYPE_GRABBER, -1);

	            // Save the user-defined data struct in the passed-in interface pointer. This same pointer is accessed
	            // in other functions so a global variable doesn't have to be retained for TomUsbCamCtrlIntfDevStructPtr.
	            // Both the isochronous and control interaces have their own structs, so they are essentially treated
	            // as 2 instances of this same driver.
                usb_set_intfdata(UsbDevInterfaceStructPtr, TomUsbCamCtrlIntfDevStructPtr);
                
                if (DeviceProbeSuccessStatus) 
                {
	                
	                pr_err("TomUsbCamProbe error: control interface video_register_device() failed");

	                // Reset the saved user-defined data to be nothing.
	                usb_set_intfdata(UsbDevInterfaceStructPtr, NULL);	                
                }	                           
            }
        }
        else
        {
	    
	        if (IsochronousInBufferAllocated == true)
	        {

                usb_set_intfdata(UsbDevInterfaceStructPtr, TomUsbCamIsochronousInputDevStructPtr);
                
            	DeviceProbeSuccessStatus = usb_register_dev(UsbDevInterfaceStructPtr, &TomUsbCamClass);
            	
            	// A value of 0 from usb_register_dev() indicates success.
                if (DeviceProbeSuccessStatus) 
                {
	            
	                pr_err("TomUsbCamProbe error: isochronous interface usb_register_dev() failed");
	                
	                usb_set_intfdata(UsbDevInterfaceStructPtr, NULL);
                }        	
            }
        }
    }
    
    if (IntfIsForCtrl)
    {    
	
        // Decrement the driver reference count if something in the course of the probe() function failed
        if ((MemoryAllocatedForDev == false) || (CtrlIntfBufferAllocated == false) || (DeviceProbeSuccessStatus))
        {
        
            if (TomUsbCamCtrlIntfDevStructPtr)
            {
	            kref_put(&TomUsbCamCtrlIntfDevStructPtr->KernelRefCountStruct, TomUsbCamCtrlIntfDelete);
            }
        }
        else
        {
         
            // After video_register_device(), the user-defined device is accessible through this interface.
            // dev_info() is the same as pr_info() but provides info about the associated device too. Show the minor 
            // number obtained from the call to usb_register_dev() to the user.   
            dev_info(&TomUsbCamCtrlIntfDevStructPtr->VideoDevice.dev, "TomUsbCam device control interface now attached to video%d (TomUsbCam)", 
                     TomUsbCamCtrlIntfDevStructPtr->VideoDevice.minor);
        }
	}
	else
	{

        if ((MemoryAllocatedForDev == false) || (IsochronousInBufferAllocated == false) || (DeviceProbeSuccessStatus))
        {
        
            if (TomUsbCamIsochronousInputDevStructPtr)
            {
	            kref_put(&TomUsbCamIsochronousInputDevStructPtr->KernelRefCountStruct, TomUsbCamIsochronousInputDelete);
            }
        }
        else
        {
            
            dev_info(&UsbDevInterfaceStructPtr->dev, "TomUsbCam device isochronous interface now attached to video%d (TomUsbCam)", 
                     UsbDevInterfaceStructPtr->minor);
        }	
	}
    	        
    return DeviceProbeSuccessStatus;	
}

// V4l2-specific functions
//-----------------------------------------------------------------------------------------------

// Handle camera control requests coming from user space. The v4l2 driver still calls the "VIDIOC_S_CTRL"
// function within itself, but the actual communication with the Usb device is handled here. I guess the 
// VIDIOC_S_CTRL activity is just an internal bookkeeping function. I'm not sure if the device can retain
// settings statically, but if it could then it seems the v4l2 driver and the actual device would be out of sync.
static int TomUsbCamSetV4l2Control(struct v4l2_ctrl *V4l2ControlReq)
{
    
    // Get the top-level struct associated with this user space control request so the
    // actual value can be changed through the Usb interface.
	struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = 
	        container_of(V4l2ControlReq->handler, struct TomUsbCamCtrlIntfDevStruct, V4l2CtrlHandler);

    // Make sure to always disable and re-enable the camera when changing parameters, otherwise they won't take.
	switch (V4l2ControlReq->id) 
	{
		
		// Comments for brightness generally apply to other settings as well.	    
	    case V4L2_CID_BRIGHTNESS:

            if (TomUsbCamCtrlIntfDevStructPtr->BrightnessChangeSupported)
            {

                // Brightness value is 2 little-endian bytes.
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[0] = V4l2ControlReq->val & 0xff;
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[1] = (V4l2ControlReq->val & 0xff00) >> 8;

                DisableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);
                                        
                // Read the before and after brightness value to ensure it was set correctly.                              
                // Read to the 3rd byte since the new brightness value occupies the first 2 bytes.                                            
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               BrightnessValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               BrightnessControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int PreviousBrightnessValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                              TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];

                // All the fields for this Usb message are defined here. 
                // The other Usb messages follow roughly the same format.
                // Message direction: host->device (set request) 
                // Request type: class 
                // Recipient: control interface (SC_VIDEOCONTROL), per table A.2.
                // Value: brightness control setting (PU_BRIGHTNESS_CONTROL), per table A.13.
                // Index: output terminal selector (VC_OUTPUT_TERMINAL), per table A.5, &
                //        video control interface (VC_CONTROL_UNDEFINED), per table A.9.
                // Usb data to send is the brightness level in 2 bytes.
                WriteToCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                              SetCurrentSelectorControlRequest,
                              ClassTypeRequestType,
                              InterfaceRecipientRequestType,
                              BrightnessValue,
                              SelectorOutputTerminalIndex,
                              InterfaceVideoControlIndex,
                              TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer,
                              BrightnessControlPacketLen,
                              FiveSecTimeoutInMsecs); 
                                                                                                         
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               BrightnessValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               BrightnessControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int CurrentBrightnessValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                             TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];
                                                      
                pr_info("V4L2_CID_BRIGHTNESS value (requested/previous/current): %d / %d / %d", 
                        V4l2ControlReq->val,
                        PreviousBrightnessValue,
                        CurrentBrightnessValue);

                EnableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);

		        break;
	        }
	        else
	        {
	            return -EINVAL;
	        }		      
		    
	    case V4L2_CID_CONTRAST:

            if (TomUsbCamCtrlIntfDevStructPtr->ContrastChangeSupported)
            {

                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[0] = V4l2ControlReq->val & 0xff;
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[1] = (V4l2ControlReq->val & 0xff00) >> 8;

                DisableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);
                                           
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               ContrastValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               ContrastControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int PreviousContrastValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                            TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];

                WriteToCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                              SetCurrentSelectorControlRequest,
                              ClassTypeRequestType,
                              InterfaceRecipientRequestType,
                              ContrastValue,
                              SelectorOutputTerminalIndex,
                              InterfaceVideoControlIndex,
                              TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer,
                              ContrastControlPacketLen,
                              FiveSecTimeoutInMsecs); 
                                                                                                         
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               ContrastValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               ContrastControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int CurrentContrastValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                           TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];
                                                      
                pr_info("V4L2_CID_CONTRAST value (requested/previous/current): %d / %d / %d", 
                        V4l2ControlReq->val,
                        PreviousContrastValue,
                        CurrentContrastValue);

                EnableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);

		        break;
	        }
	        else
	        {
	            return -EINVAL;
	        }
		    
	    case V4L2_CID_HUE:
            
            if (TomUsbCamCtrlIntfDevStructPtr->HueChangeSupported)
            {
            
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[0] = V4l2ControlReq->val & 0xff;
                TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[1] = (V4l2ControlReq->val & 0xff00) >> 8;

                DisableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);
                                           
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               HueValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               HueControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int PreviousHueValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                       TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];

                WriteToCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                              SetCurrentSelectorControlRequest,
                              ClassTypeRequestType,
                              InterfaceRecipientRequestType,
                              HueValue,
                              SelectorOutputTerminalIndex,
                              InterfaceVideoControlIndex,
                              TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer,
                              HueControlPacketLen,
                              FiveSecTimeoutInMsecs); 
                                                                                                         
                ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                               GetCurrentSelectorControlRequest,
                               ClassTypeRequestType,
                               InterfaceRecipientRequestType,
                               HueValue,
                               SelectorOutputTerminalIndex,
                               InterfaceVideoControlIndex,
                               &TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3],
                               HueControlPacketLen,
                               FiveSecTimeoutInMsecs);
                                                      
                int CurrentHueValue = (signed char) (TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[4] << 8) |
                                      TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer[3];
                                                      
                pr_info("V4L2_CID_HUE value (requested/previous/current): %d / %d / %d", 
                        V4l2ControlReq->val,
                        PreviousHueValue,
                        CurrentHueValue);

                EnableCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);

		        break;
	        }
	        else
	        {
	            return -EINVAL;
	        }
		    
	    default:
		    
		    return -EINVAL;
	}
	
	return 0;
}

// This function seems to be required when using v4l2. For example, when using the v4l2-ctl program to set a control,
// the "vidioc_querycap" function is called.
static int TomUsbCamQueryCapability(struct file *File, void *Priv, struct v4l2_capability *V4l2CapabilitiesStructPtr)
{

    // Use the passed-in file pointer to get our device struct. It doesn't seem like this private data needs to be manually
    // set anywhere. Somehow, it just figures out the video_device parent I guess. See "video_drvdata" here:
    // https://www.kernel.org/doc/html/v4.17/media/kapi/v4l2-dev.html
	struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = video_drvdata(File);

    // Fill the passed-in capability struct pointer so that user space can know the device capabilities.
    // See here for the struct layout:
    // https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/vidioc-querycap.html#c.v4l2_capability

    __u8 DriverName[] = "TomUsbCam";
    strlcpy(V4l2CapabilitiesStructPtr->driver, DriverName, sizeof(V4l2CapabilitiesStructPtr->driver));
    
    // Tom report camera Vid/Pid here
    __u8 DeviceName[] = "Generic Web Cam";
    strlcpy(V4l2CapabilitiesStructPtr->card, DeviceName, sizeof(V4l2CapabilitiesStructPtr->card));
    
    //  Tom report bus location here.
    __u8 DeviceBusLocation[] = "usb-0";
    strlcpy(V4l2CapabilitiesStructPtr->bus_info, DeviceBusLocation, sizeof(V4l2CapabilitiesStructPtr->bus_info));
    
    // Use a made-up version of 1.2.3 for now.
    __u32 KernelVersion = (1 << 16) + (2 << 8) + 3;
    V4l2CapabilitiesStructPtr->version = KernelVersion;
		 
	// Report all capabilites of the device.
    __u32 Capabilities = V4L2_CAP_VIDEO_CAPTURE | 
                         V4L2_CAP_READWRITE |
                         V4L2_CAP_AUDIO |
                         V4L2_CAP_STREAMING | 
                         V4L2_CAP_DEVICE_CAPS;
    V4l2CapabilitiesStructPtr->capabilities = Capabilities;
		
    __u32 DeviceCapabilities = V4L2_CAP_VIDEO_CAPTURE | 
                               V4L2_CAP_READWRITE |
                               V4L2_CAP_AUDIO |
                               V4L2_CAP_STREAMING;
    V4l2CapabilitiesStructPtr->device_caps = DeviceCapabilities;
                                             
    // strlcpy(V4l2CapabilitiesStructPtr->reserved, {'\0', '\0', '\0'}, sizeof(V4l2CapabilitiesStructPtr->reserved));
		 
    // Return "0" on success.
	return 0;
}

// Set the image format. Per this site, this is the image formatter that is called for single-plane mode: 
// https://01.org/linuxgraphics/gfx-docs/drm/media/kapi/v4l2-common.html
// We are using single-plane mode since our camera capture Yuyv data.
static int TomUsbCamSetFormat(struct file *File, void *Priv, struct v4l2_format *V4l2ImageFormatStructPtr)
{

    struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = video_drvdata(File);
    
    // A value of "0" indicates success.
    int FormatterErrorValue = TomUsbCamTryFormat(File, Priv, V4l2ImageFormatStructPtr);
    
    if (FormatterErrorValue != 0)
    {
        return FormatterErrorValue;
    }
    
    // If any buffers were already allocated, the format cannot be changed. See:
    // https://linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-vb2-is-busy.html
    if (vb2_is_busy(&TomUsbCamCtrlIntfDevStructPtr->TomUsbCamV4l2Queue))
    {
    
        FormatterErrorValue = -EBUSY;
    
        return FormatterErrorValue;
    }
   
   
   
   
   
    
    
// Use the Usb comms method from TomUsbCamSetV4l2Control here as well to update the image format
    





//Does this setting conflict with the previous V4l2PixFormatStruct settings, i.e. the settings around
//line 300?
    TomUsbCamCtrlIntfDevStructPtr->V4l2PixFormatStruct = V4l2ImageFormatStructPtr->fmt.pix;
 
    return FormatterErrorValue;   
}

// Tom this is a stub for now
static int TomUsbCamGetFormat(struct file *File, void *Priv, struct v4l2_format *V4l2ImageFormatStructPtr)
{
    return -1;
}

// Make sure only Yuyv pixel formats are attempted right now.
static int TomUsbCamTryFormat(struct file *File, void *Priv, struct v4l2_format *V4l2ImageFormatStructPtr)
{

    struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = video_drvdata(File);

    struct v4l2_pix_format *V4l2PixelFormat = &V4l2ImageFormatStructPtr->fmt.pix;
    
    int FormatterErrorValue = 0;
    
    // Only Yuyv data is supported right now.
	if (V4l2PixelFormat->pixelformat != V4L2_PIX_FMT_YUYV)
	{
	    FormatterErrorValue = -EINVAL;
	}
	
	return FormatterErrorValue;
}

// WriteToCamera() & ReadFromCamera() are just wrappers around usb_control_msg().
static int WriteToCamera(struct usb_device *UsbDevStructPtr, __u8 UsbMsgRequest, 
                         __u8 UsbMsgRequestType, __u8 UsbMsgRequestTypeRecipient, 
                         __u16 UsbMsgValue, __u16 UsbMsgIndexDestination, __u16 UsbMsgIndexId, 
                         unsigned char *UsbMsgData, __u16 UsbMsgDataSize, int UsbMsgTimeout)
{

    int BytesRcvdOrErrorCode = usb_control_msg(UsbDevStructPtr,
                                               usb_sndctrlpipe(UsbDevStructPtr, 
                                                               HostToDeviceDataPhaseTransferDirectionRequestType),                                                               
                                               UsbMsgRequest,
                                               HostToDeviceDataPhaseTransferDirectionRequestType |
                                               UsbMsgRequestType |
                                               UsbMsgRequestTypeRecipient,
                                               UsbMsgValue,
                                               UsbMsgIndexDestination | UsbMsgIndexId,
                                               UsbMsgData,
                                               UsbMsgDataSize,
                                               UsbMsgTimeout);

    return BytesRcvdOrErrorCode;
}

static int ReadFromCamera(struct usb_device *UsbDevStructPtr, __u8 UsbMsgRequest, 
                         __u8 UsbMsgRequestType, __u8 UsbMsgRequestTypeRecipient, 
                         __u16 UsbMsgValue, __u16 UsbMsgIndexDestination, __u16 UsbMsgIndexId,
                         unsigned char *UsbMsgData, __u16 UsbMsgDataSize, int UsbMsgTimeout)
{

    int BytesRcvdOrErrorCode = usb_control_msg(UsbDevStructPtr,
                                               usb_rcvctrlpipe(UsbDevStructPtr, 
                                                               DeviceToHostDataPhaseTransferDirectionRequestType),                                                               
                                               UsbMsgRequest,
                                               DeviceToHostDataPhaseTransferDirectionRequestType |
                                               UsbMsgRequestType |
                                               UsbMsgRequestTypeRecipient,
                                               UsbMsgValue,
                                               UsbMsgIndexDestination | UsbMsgIndexId,
                                               UsbMsgData,
                                               UsbMsgDataSize,
                                               UsbMsgTimeout);

    return BytesRcvdOrErrorCode;
}

// Query the max/min/step/default values of the camera before adding the V4l2 controls.
// The ResetToDefault flag is used to restore the specific queried value.
static int QueryCameraFactoryValues(struct usb_device *UsbDevStructPtr, __u16 UsbMsgValue, __u16 UsbMsgDataSize,
                                    unsigned char *UsbMsgData, int *Max, int *Min, int *Step, int *Default, bool ResetToDefault)
{

    DisableCamera(UsbDevStructPtr);

    int BytesRcvdOrErrorCode = 0;

    BytesRcvdOrErrorCode |= ReadFromCamera(UsbDevStructPtr,
                                           GetMinSelectorControlRequest,
                                           ClassTypeRequestType,
                                           InterfaceRecipientRequestType,
                                           UsbMsgValue,
                                           SelectorOutputTerminalIndex,
                                           InterfaceVideoControlIndex,
                                           UsbMsgData,
                                           UsbMsgDataSize,
                                           FiveSecTimeoutInMsecs);
                      
    if (UsbMsgDataSize == 1)
    {
        *Min = (int8_t) UsbMsgData[0];
    }
    else if (UsbMsgDataSize == 2)
    {
        *Min = (int16_t) (UsbMsgData[0] |
                          UsbMsgData[1] << 8);                     
    }
    
    BytesRcvdOrErrorCode |= ReadFromCamera(UsbDevStructPtr,
                                           GetMaxSelectorControlRequest,
                                           ClassTypeRequestType,
                                           InterfaceRecipientRequestType,
                                           UsbMsgValue,
                                           SelectorOutputTerminalIndex,
                                           InterfaceVideoControlIndex,
                                           UsbMsgData,
                                           UsbMsgDataSize,
                                           FiveSecTimeoutInMsecs);
                              
    if (UsbMsgDataSize == 1)
    {
        *Max = (int8_t) UsbMsgData[0];
    }
    else if (UsbMsgDataSize == 2)
    {
        *Max = (int16_t) (UsbMsgData[0] |
                          UsbMsgData[1] << 8);
    }    
    
    BytesRcvdOrErrorCode |= ReadFromCamera(UsbDevStructPtr,
                                           GetResolutionSelectorControlRequest,
                                           ClassTypeRequestType,
                                           InterfaceRecipientRequestType,
                                           UsbMsgValue,
                                           SelectorOutputTerminalIndex,
                                           InterfaceVideoControlIndex,
                                           UsbMsgData,
                                           UsbMsgDataSize,
                                           FiveSecTimeoutInMsecs);
                              
    if (UsbMsgDataSize == 1)
    {
        *Step = (int8_t) UsbMsgData[0];
    }
    else if (UsbMsgDataSize == 2)
    {
        *Step = (int16_t) (UsbMsgData[0] |
                           UsbMsgData[1] << 8);
    }            

    BytesRcvdOrErrorCode |= ReadFromCamera(UsbDevStructPtr,
                                           GetDefaultSelectorControlRequest,
                                           ClassTypeRequestType,
                                           InterfaceRecipientRequestType,
                                           UsbMsgValue,
                                           SelectorOutputTerminalIndex,
                                           InterfaceVideoControlIndex,
                                           UsbMsgData,
                                           UsbMsgDataSize,
                                           FiveSecTimeoutInMsecs);
                              
    if (UsbMsgDataSize == 1)
    {
        *Default = (int8_t) UsbMsgData[0];
    }
    else if (UsbMsgDataSize == 2)
    {
        *Default = (int16_t) (UsbMsgData[0] |
                              UsbMsgData[1] << 8);
    }
    
    if (ResetToDefault)
    {
        WriteToCamera(UsbDevStructPtr,
                      SetInterfaceRequest,
                      ClassTypeRequestType,
                      InterfaceRecipientRequestType,
                      UsbMsgValue,
                      SelectorOutputTerminalIndex,
                      InterfaceVideoControlIndex,
                      UsbMsgData,
                      UsbMsgDataSize,
                      FiveSecTimeoutInMsecs);    
    }
                                     
    EnableCamera(UsbDevStructPtr);                                         

    return BytesRcvdOrErrorCode;
}

// EnableCamera() & DisableCamera() are helper functions to start and stop streaming since 
// this is a common occurence, e.g. any time a camera parameter is updated.
static int EnableCamera(struct usb_device *UsbDevStructPtr)
{
    
    int BytesRcvdOrErrorCode = WriteToCamera(UsbDevStructPtr,
                                             SetInterfaceRequest,
                                             StandardTypeRequestType,
                                             InterfaceRecipientRequestType,
                                             OperationalInterfaceValue,
                                             0x0,
                                             InterfaceVideoStreamingIndex,
                                             NULL,
                                             0x0,
                                             FiveSecTimeoutInMsecs);
                               
    return BytesRcvdOrErrorCode;
}

static int DisableCamera(struct usb_device *UsbDevStructPtr)
{

    int BytesRcvdOrErrorCode = WriteToCamera(UsbDevStructPtr,
                                             SetInterfaceRequest,
                                             StandardTypeRequestType,
                                             InterfaceRecipientRequestType,
                                             ZeroBandwidthInterfaceValue,
                                             0x0,
                                             InterfaceVideoStreamingIndex,
                                             NULL,
                                             0x0,
                                             FiveSecTimeoutInMsecs);
                  
    return BytesRcvdOrErrorCode;
}

// Save a list of structs containing all the Usb descriptor information.
static int SaveAllDescriptors(struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr)
{

    // Tom see section 2.3.4.7 of [6] & 3.7.2.5 of [5] for querying the camera capabilites before attempting
    // to set their values.
    
    // Per [4], querying the configuration descriptor also returns all the interface and endpoint descriptors.
    // First get the combined descriptors length, which is cotained in bytes 3 & 4 of the configuration descriptor.
    unsigned char *DescriptorSizePtr = kzalloc(4, GFP_NOIO);

    ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                   GetStandardDescriptorRequest,
                   StandardTypeRequestType,
                   DeviceRecipientRequestType,
                   GetDescriptorConfigurationValue,
                   0x0,
                   0x0,
                   DescriptorSizePtr,
                   0x4,
                   FiveSecTimeoutInMsecs);
                   
    int AllDescriptorsPacketLen = (int16_t) (DescriptorSizePtr[2] | DescriptorSizePtr[3] << 8);                   
                   
    kfree(DescriptorSizePtr);
    
    // Now read all the descriptors now that we know the length.
    unsigned char *AllDescriptorsPtr = kzalloc(AllDescriptorsPacketLen, GFP_NOIO);
    
    ReadFromCamera(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr,
                   GetStandardDescriptorRequest,
                   StandardTypeRequestType,
                   DeviceRecipientRequestType,
                   GetDescriptorConfigurationValue,
                   0x0,
                   0x0,
                   AllDescriptorsPtr,
                   AllDescriptorsPacketLen,
                   FiveSecTimeoutInMsecs);
                   
    int CurrentDescriptorsPacketLoc = 0;
    int ConfigurationDescriptorStructTotalCount = 0;
    int InterfaceDescriptorStructTotalCount = 0;
    int EndpointDescriptorStructTotalCount = 0;
    int VideoDescriptorStructTotalCount = 0;
    
    // Parse all the descriptors into their appropriate structs.
    // First figure out the descriptor count in each catergory for memory allocation.
    while (CurrentDescriptorsPacketLoc < AllDescriptorsPacketLen)
    {
                
        // bLength & bDescriptorType are always the first 2 bytes.
        uint8_t bLength = AllDescriptorsPtr[CurrentDescriptorsPacketLoc];
        uint8_t bDescriptorType = AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 1];

        // Only save the configuration, interface, endpoint and VideoControl descriptors for now.
        if (bDescriptorType == DescriptorTypeConfiguration)
        {
            ConfigurationDescriptorStructTotalCount += 1;
        }
        
        if (bDescriptorType == DescriptorTypeInterface)
        {
            InterfaceDescriptorStructTotalCount += 1;
        }
        
        if (bDescriptorType == DescriptorTypeEndpoint)
        {
            EndpointDescriptorStructTotalCount += 1;
        }        
        
        if (bDescriptorType == DescriptorTypeVideoInterface)
        {
            VideoDescriptorStructTotalCount += 1;
        }   
        
        CurrentDescriptorsPacketLoc += bLength;
    }
    
    // Allocate memory for all the structs. Note that (sizeof *x) is the the dereferenced size, i.e. the size
    // of the struct, not the size of the pointer. See:
    // https://stackoverflow.com/questions/10468128/how-do-you-make-an-array-of-structs-in-c#answer-24542015
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr = 
        kzalloc(ConfigurationDescriptorStructTotalCount * 
                sizeof *TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr, 
                GFP_NOIO);
        
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr = 
        kzalloc(InterfaceDescriptorStructTotalCount * 
                sizeof *TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr, 
                GFP_NOIO);   
        
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr = 
        kzalloc(EndpointDescriptorStructTotalCount * 
                sizeof *TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr, 
                GFP_NOIO);  
                        
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr = 
        kzalloc(VideoDescriptorStructTotalCount *
                sizeof *TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr, 
                GFP_NOIO);                           

    // Reset the packet location for the 2nd iteration to actually divy up the descriptors.
    // Remember to add offsets for the parent struct association numbers when performing memcpy().
    // It seems everything is reported in order, such that each parent config is followed by its children
    // interface and endpoint descriptors. Rely on this to make all of the associations.
    CurrentDescriptorsPacketLoc = 0;
    uint8_t ConfigurationDescriptorStructCount = 0;
    uint8_t InterfaceDescriptorStructCount = 0;
    uint8_t EndpointDescriptorStructCount = 0;
    uint8_t VideoDescriptorStructCount = 0;
    
    uint8_t CurrentConfiguration = 0;
    uint8_t CurrentParentInterfaceInterfaceNumber = 0;
    uint8_t CurrentParentInterfaceAlternateSetting = 0;
    
    while (CurrentDescriptorsPacketLoc < AllDescriptorsPacketLen)
    {
                
        // bLength & bDescriptorType are always the first 2 bytes.
        uint8_t bLength = AllDescriptorsPtr[CurrentDescriptorsPacketLoc];
        uint8_t bDescriptorType = AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 1];
        
        // You could use memcpy() to fill all these structs but I am assigning explicit bytes when a struct has uint16_t's
        // due to byte-ordering of the uint16_t and also struct padding problems from the compiler. See:
        // https://stackoverflow.com/questions/58633342/c-memcpy-to-struct-allocation
        if (bDescriptorType == DescriptorTypeConfiguration)
        {

            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bLength = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc];
                   
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bDescriptorType = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 1];                   
                   
            // Fix the reverse byte ordering for 2-byte values (uint16_t).
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].wTotalLength = 
                (AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 2] | (AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 3] << 8));

            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bNumInterfaces = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 4];  
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bConfigurationValue = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 5];  
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].iConfiguration = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 6];  
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bmAttributes = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 7];
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bMaxPower = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 8];                                                                  

            CurrentConfiguration = 
                TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[ConfigurationDescriptorStructCount].bConfigurationValue;

            ConfigurationDescriptorStructCount += 1;         
        }
        
        if (bDescriptorType == DescriptorTypeInterface)
        {
        
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr[InterfaceDescriptorStructCount].ConfigurationAssoc = 
                CurrentConfiguration;
        
            memcpy(&TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr[InterfaceDescriptorStructCount].bLength, 
                   &AllDescriptorsPtr[CurrentDescriptorsPacketLoc], 
                   bLength);

            CurrentParentInterfaceInterfaceNumber = 
                TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr[InterfaceDescriptorStructCount].bInterfaceNumber;

            CurrentParentInterfaceAlternateSetting = 
                TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr[InterfaceDescriptorStructCount].bAlternateSetting;

            InterfaceDescriptorStructCount += 1;         
        }  
        
        if (bDescriptorType == DescriptorTypeEndpoint)
        {
        
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].ConfigurationAssoc = 
                CurrentConfiguration;
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].ParentInterfaceAssoc = 
                CurrentParentInterfaceInterfaceNumber;   
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].AlternateSettingAssoc = 
                CurrentParentInterfaceAlternateSetting;                                  

           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].bLength = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 0];
                
           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].bDescriptorType = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 1];
                
           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].bEndpointAddress = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 2];
                
           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].bmAttributes = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 3];                                                
           
           // Fix the byte ordering for uint16_t
           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].wMaxPacketSize = 
                (AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 4] | (AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 5] << 8));
                
           TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[EndpointDescriptorStructCount].bInterval = 
                AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 6];

            EndpointDescriptorStructCount += 1;         
        } 
        
        if (bDescriptorType == DescriptorTypeVideoInterface)
        {
        
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructCount].ConfigurationAssoc = 
                CurrentConfiguration;
                
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructCount].ParentInterfaceAssoc = 
                CurrentParentInterfaceInterfaceNumber;          
        
            // Only the 1st 3 fields (bytes) are consistent for the VideoControl descriptors.        
            memcpy(&TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructCount].bLength, 
                   &AllDescriptorsPtr[CurrentDescriptorsPacketLoc], 
                   3);
                   
            TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructCount].VarData = 
                kzalloc(bLength - 3, GFP_NOIO);                      
                                     
            memcpy(&TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructCount].VarData[0], 
                   &AllDescriptorsPtr[CurrentDescriptorsPacketLoc + 3], 
                   (bLength - 3));

           VideoDescriptorStructCount += 1;         
        }                        
        
        CurrentDescriptorsPacketLoc += bLength;
    }
    
    // Keep track of the counts to avoid having to constantly re-querying the device.
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructCount = ConfigurationDescriptorStructCount;
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructCount = InterfaceDescriptorStructCount;
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructCount = EndpointDescriptorStructCount;
    TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoDescriptorStructCount = VideoDescriptorStructCount;

    kfree(AllDescriptorsPtr);                
    
    return 0;                  
}

// Return a user-requested descriptor so the user can extract whatever values they want. Configurations are only 
// enumerated by their index. It seems their is only ever one configuration descriptor for cameras anyways.
static void GetConfigurationDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr, uint8_t DescriptorIndex, 
                                             struct ConfigurationDescriptorStruct **ConfigurationDescriptorStructPtr, int8_t *DescriptorReadSuccess)
{

    *DescriptorReadSuccess = -1;     

    if (DescriptorIndex < TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructCount)
    {

        *ConfigurationDescriptorStructPtr = &TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr[DescriptorIndex];

        *DescriptorReadSuccess = 1;
    }
}

static void GetInterfaceDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr, uint8_t DescriptorIndex,
                                         struct InterfaceDescriptorStruct **InterfaceDescriptorStructPtr, int8_t *DescriptorReadSuccess)
{

    *DescriptorReadSuccess = -1;
    
    if (DescriptorIndex < TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructCount)
    {

        *InterfaceDescriptorStructPtr = &TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr[DescriptorIndex];

        *DescriptorReadSuccess = 1;
    }    
}

static void GetEndpointDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr, uint8_t ParentInterfaceAssoc,
                                        uint8_t AlternateSettingAssoc, struct EndpointDescriptorStruct **EndpointDescriptorStructPtr, 
                                        int8_t *DescriptorReadSuccess)
{

    *DescriptorReadSuccess = -1;
    
    // Loop through all of the endpoint descriptors looking for the matching parent and alternate setting numbers instead of using
    // an endpoint index.
    for (int idx = 0; idx < TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructCount; idx++)
    {

        if ((ParentInterfaceAssoc == TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[idx].ParentInterfaceAssoc) &&
            (AlternateSettingAssoc == TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[idx].AlternateSettingAssoc))
        {

            *EndpointDescriptorStructPtr = &TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr[idx];
            
            *DescriptorReadSuccess = 1;  
        }
    }
}

static void GetVideoInterfaceDescriptorStruct(struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr, uint8_t ParentInterfaceAssoc,
                                              uint8_t VideoInterfaceSubtype, struct VideoInterfaceDescriptorStruct **VideoInterfaceDescriptorStructPtr, 
                                              int8_t *DescriptorReadSuccess)
{

    *DescriptorReadSuccess = -1;
    
    // Loop through all of the video interface descriptors looking for the matching parent number and video interface subtype instead of using
    // a video interface index.
    for (int idx = 0; idx < TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoDescriptorStructCount; idx++)
    {        
        
        if ((ParentInterfaceAssoc == TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[idx].ParentInterfaceAssoc) &&
            (VideoInterfaceSubtype == TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[idx].bDescriptorSubtype))
        {

            *VideoInterfaceDescriptorStructPtr = &TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[idx];
            
            *DescriptorReadSuccess = 1;  
        }                
    }
}

// V4l2-specific functions for the queue
//***********************************************************************************************

// Check the camera to see if a packed/interleaved format or planar format is used. "NumImagePlanes" will
// be "1" for an interleaved format. See:
// "Other videobuf2 callbacks" here: https://lwn.net/Articles/447435/
// Since we're using yuyv for this camera, which is a a packed format, NumImagePlanes == 1.
static int TomUsbCamV4l2QueueSetup(struct vb2_queue *VideoBufferQueue,
		                           unsigned int *NumBuffers, unsigned int *NumImagePlanes,
		                           unsigned int ImageSizes[], struct device *alloc_devs[])
{

    // The pointer to the containing struct was previously saved in this struct's "private data" section.
    struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = vb2_get_drv_priv(VideoBufferQueue);


    // tom continue following "queue_setup()" function here
    
    return -1;
}

// Tom this is a stub for now
static int buffer_prepare(struct vb2_buffer *vb)
{
    return -1;
}

// Tom this is a stub for now
static void buffer_queue(struct vb2_buffer *vb)
{
    return;
}

// Tom this is a stub for now
static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
    return -1;
}

// Tom this is a stub for now
static void stop_streaming(struct vb2_queue *vb)
{
    return;
}

//***********************************************************************************************
//-----------------------------------------------------------------------------------------------

// This is the counterpart to the probe() function, i.e. it is called when a device is unplugged.
static void TomUsbCamDisconnect(struct usb_interface *UsbDevInterfaceStructPtr)
{

    // Since this disconnect function is called for both the control and isochronous interfaces, figure out which
    // one we are dealing with before proceeding.
    int NumOfAltSettingsForThisIntf = UsbDevInterfaceStructPtr->num_altsetting;
        
    bool IntfIsForCtrl = (NumOfAltSettingsForThisIntf > 1) ? false : true;

	// Declare both of these structs but only init the one pertinent to this interface.
	struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = NULL;
	struct TomUsbCamIsochronousInputDevStruct *TomUsbCamIsochronousInputDevStructPtr = NULL;	

    int DeviceMinorNum;

    // Extract the user-defined device and reset the association with this device in the 
    // generic passed-in usb_interface.
    if (IntfIsForCtrl)
    {
    
        // Extract the saved interface pointer so the proper reference count can be decremented.
	    TomUsbCamCtrlIntfDevStructPtr = usb_get_intfdata(UsbDevInterfaceStructPtr);
    
        DeviceMinorNum = TomUsbCamCtrlIntfDevStructPtr->VideoDevice.minor;
	    
	    // Reset the internally saved user data pointer.
	    usb_set_intfdata(UsbDevInterfaceStructPtr, NULL);
	    
	    // Decrement the total number of kernel reference counts to this device
	    kref_put(&TomUsbCamCtrlIntfDevStructPtr->KernelRefCountStruct, TomUsbCamCtrlIntfDelete);
	    
        dev_info(&TomUsbCamCtrlIntfDevStructPtr->VideoDevice.dev, "TomUsbCam #%d now disconnected", DeviceMinorNum);

        video_unregister_device(&TomUsbCamCtrlIntfDevStructPtr->VideoDevice);
	    v4l2_ctrl_handler_free(&TomUsbCamCtrlIntfDevStructPtr->V4l2CtrlHandler);
	    v4l2_device_unregister(&TomUsbCamCtrlIntfDevStructPtr->V4l2DevStruct);

	    // Free all the saved Usb descriptor info
	    uint8_t VideoDescriptorStructCount = TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoDescriptorStructCount;
	    
        for(uint8_t VideoDescriptorStructIdx = 0 ; VideoDescriptorStructIdx < VideoDescriptorStructCount; VideoDescriptorStructIdx++)
        {
            kfree(&TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr[VideoDescriptorStructIdx].VarData[0]);	    
        }
	    
        kfree(TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.ConfigurationDescriptorStructPtr);
        
        kfree(TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.InterfaceDescriptorStructPtr);   
        
        kfree(TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.EndpointDescriptorStructPtr);  
                        
        kfree(TomUsbCamCtrlIntfDevStructPtr->UsbDescriptorsForThisCameraStruct.VideoInterfaceDescriptorStructPtr);

        kfree(TomUsbCamCtrlIntfDevStructPtr);
    }
    else
    {
    
	    TomUsbCamIsochronousInputDevStructPtr = usb_get_intfdata(UsbDevInterfaceStructPtr);
	    
	    DeviceMinorNum = UsbDevInterfaceStructPtr->minor;
	    
	    usb_set_intfdata(UsbDevInterfaceStructPtr, NULL);

	    usb_deregister_dev(UsbDevInterfaceStructPtr, &TomUsbCamClass);

	    kref_put(&TomUsbCamIsochronousInputDevStructPtr->KernelRefCountStruct, TomUsbCamIsochronousInputDelete);
	    
	    dev_info(&UsbDevInterfaceStructPtr->dev, "TomUsbCam #%d now disconnected", DeviceMinorNum);
    }
}

// Use separate delete functions for each of the interfaces. The different interfaces
// are essentially treated as different devices.
static void TomUsbCamCtrlIntfDelete(struct kref *KernelRefCountStructPtr)
{	

    // The kernel reference count struct is used to retreive the custom wrapper struct. After
    // this the usb device is released, and the previously allocated memory in this device is also
    // released.
	struct TomUsbCamCtrlIntfDevStruct *TomUsbCamCtrlIntfDevStructPtr = 
	        container_of(KernelRefCountStructPtr, struct TomUsbCamCtrlIntfDevStruct, KernelRefCountStruct);

	// usb_put_dev(TomUsbCamCtrlIntfDevStructPtr->UsbDevStructPtr);
	
	kfree(TomUsbCamCtrlIntfDevStructPtr->CtrlIntfBuffer);
	
	// This should not be freed until after video_unregister_device() is called
	//kfree(TomUsbCamCtrlIntfDevStructPtr);
}

static void TomUsbCamIsochronousInputDelete(struct kref *KernelRefCountStructPtr)
{	

    pr_info("\nTom TomUsbCamCtrlIntfDelete 2 called here\n");

	struct TomUsbCamIsochronousInputDevStruct *TomUsbCamIsochronousInputDevStructPtr = 
	        container_of(KernelRefCountStructPtr, struct TomUsbCamIsochronousInputDevStruct, KernelRefCountStruct);

	usb_put_dev(TomUsbCamIsochronousInputDevStructPtr->UsbDevStructPtr);
	kfree(TomUsbCamIsochronousInputDevStructPtr->IsochronousInputBuffer);
	kfree(TomUsbCamIsochronousInputDevStructPtr);
}

// This function is called when the module is inserted in the kernel.
static int __init TomUsbCamInit(void)
{

    // Register this driver with the USB subsystem
	int UsbRegisterResult = usb_register(&TomUsbCamDriver);
	
	if (UsbRegisterResult)
	{
	    pr_err("TomUsbCamInit error: usb_register() failed. Error number %d", UsbRegisterResult);
    }

	return UsbRegisterResult;
}

static void __exit TomUsbCamExit(void)
{
	// De-register this driver with the USB subsystem
	usb_deregister(&TomUsbCamDriver);
}

module_init (TomUsbCamInit);
module_exit (TomUsbCamExit);





