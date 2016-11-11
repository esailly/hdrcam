#include <opencv2/opencv.hpp>
#include <libuvc/libuvc.h>

#include <iostream>
#include <unistd.h>

void cb(uvc_frame_t *frame, void *ptr) {

    uvc_frame_t *bgr;
    uvc_error_t ret;

    bgr = uvc_allocate_frame(frame->width * frame->height * 3);
    if (not bgr) {
        std::cerr << "unable to allocate bgr frame!\n";
        return;
    }

    ret = uvc_any2bgr(frame, bgr);
    if (ret) {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
    }

    cv::Mat img(cv::Size(bgr->width, bgr->height), CV_8UC3, bgr->data);
    cv::imshow("Test", img);
    uvc_free_frame(bgr);

    int key = cv::waitKey(10);
    if (key % 0x100 == 27) 
        exit(0);
}

void tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}

int main(int argc, char **argv) {

    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;

    tryUvc("init", uvc_init(&ctx, NULL));
    tryUvc("find_device", uvc_find_device(ctx, &dev, 0, 0, NULL));

    tryUvc("open", uvc_open(dev, &devh));
    uvc_print_diag(devh, stderr);

    tryUvc("get_ctrl", uvc_get_stream_ctrl_format_size(devh, &ctrl, 
                UVC_FRAME_FORMAT_ANY, 640, 480, 30));
    uvc_print_stream_ctrl(&ctrl, stderr);

    tryUvc("start_streaming", uvc_start_streaming(devh, &ctrl, cb, nullptr, 0));
    tryUvc("set_ae_mode", uvc_set_ae_mode(devh, 1));

    for (int i = 1; i <= 10; i++) {
        tryUvc("set_exp_abs", uvc_set_exposure_abs(devh, i%2 == 0 ? 400 : 100 ));
        sleep(1);
    }

    uvc_stop_streaming(devh);
    uvc_close(devh);
    uvc_unref_device(dev);
    uvc_exit(ctx);

    return 0;
}


