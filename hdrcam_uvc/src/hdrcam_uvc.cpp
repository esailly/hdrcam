#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>
#include <iostream>
#include <mutex>

// uvc data (webcam)
uvc_context_t * gCxt;
uvc_device_handle_t * gDevh;
uvc_device_t * gDev;
uvc_stream_ctrl_t gCtrl;
uvc_frame_t * gFrame;

// image parameters
int gWidth = 640;
int gHeight = 480;
int gBrightExp = 500;
int gDarkExp = 50;

// capture/processing synchronization
bool gIsDark = false;
std::mutex gMutex;

// call uvc function and check return code
void tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}

// capture callback function
void captureFun(uvc_frame_t * frame, void *) {

    // prevent concurrency (image processing)
    std::lock_guard<std::mutex> lock(gMutex);

    // get image from webcam
    tryUvc("any2bgr", uvc_any2bgr(frame, gFrame));

    // change exposure time for next capture
    if (gIsDark) {
        gIsDark = false;
        tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gBrightExp));
    }
    else {
        gIsDark = true;
        tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, gDarkExp));
    }

}

int main() {

    // init opencv data (image processing and UI)
    cv::namedWindow("dark");
    cv::namedWindow("bright");

    // init uvc data (webcam)
    tryUvc("init", uvc_init(&(gCxt), nullptr));
    tryUvc("find_device", uvc_find_device(gCxt, &gDev, 0, 0, nullptr));
    tryUvc("open", uvc_open(gDev, &(gDevh)));
    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(gDevh, 
                &(gCtrl), UVC_FRAME_FORMAT_ANY, gWidth, gHeight, 30));
    tryUvc("set_ae_mode", uvc_set_ae_mode(gDevh, 1));
    tryUvc("set_exp_abs", uvc_set_exposure_abs(gDevh, 100));
    gFrame = uvc_allocate_frame(gWidth * gHeight * 3);
    if (not gFrame) {
        std::cerr << "unable to allocate frame!\n";
        exit(-1);
    }
    tryUvc("start_streaming", uvc_start_streaming(gDevh, 
                &(gCtrl), captureFun, nullptr, 0));

    // main loop
    while (true) {

        // handle user events
        int key = cv::waitKey(30);
        if (key % 0x100 == 27) 
            break;

        // get and display webcam frame
        // prevent concurrency (webcam capture)
        std::lock_guard<std::mutex> lock(gMutex);  
        // process new frame
        cv::Mat mat(gHeight, gWidth, CV_8UC3, gFrame->data);
        if (gIsDark)
            cv::imshow("dark", mat);
        else
            cv::imshow("bright", mat);

    }

    /*
    // free uvc data -> BUG ???
    uvc_free_frame(gFrame);
    uvc_stop_streaming(gDevh);
    uvc_close(gDevh);
    uvc_unref_device(gDev);
    uvc_exit(gCxt);
    */

    return 0;
}

