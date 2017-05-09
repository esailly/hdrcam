/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Device_uvc.cpp
 * Author: esailly
 * 
 * Created on 26 avril 2017, 10:36
 */

#include "Device_uvc.hpp"

Device_uvc::Device_uvc() {
}

Device_uvc::Device_uvc(const Device_uvc& orig) {
}

Device_uvc::~Device_uvc() {
}


// call uvc function and check return code
void Device_uvc::tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}
