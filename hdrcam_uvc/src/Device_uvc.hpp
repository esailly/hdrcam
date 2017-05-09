/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Device_uvc.hpp
 * Author: esailly
 *
 * Created on 26 avril 2017, 10:36
 */

#ifndef DEVICE_UVC_HPP
#define DEVICE_UVC_HPP

#include <libuvc/libuvc.h>

class Device_uvc {
public:
    Device_uvc();
    Device_uvc(const Device_uvc& orig);
    virtual ~Device_uvc();
    
    // call uvc function and check return code
    void tryUvc(const char * msg, uvc_error_t res);
    
private:
    
    // uvc data (webcam)
    uvc_context_t * gCxt;
    uvc_device_handle_t * gDevh;
    uvc_device_t * gDev;
    uvc_stream_ctrl_t gCtrl;
    uvc_frame_t * gFrame;
};

#endif /* DEVICE_UVC_HPP */

