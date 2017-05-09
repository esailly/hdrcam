#include <libuvc/libuvc.h>
#include <iostream>

void tryUvc(const char * msg, uvc_error_t res) {
    if (res < 0) {
        uvc_perror(res, msg);
        exit(-1);
    }
}

int main() {

    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;

    tryUvc("init", uvc_init(&ctx, NULL));
    //tryUvc("find_device", uvc_find_device(ctx, &dev, 0, 0, NULL));
    tryUvc("find_device", uvc_find_device(ctx, &dev, 0, 2057, nullptr)); //logitec
    //tryUvc("find_device", uvc_find_device(ctx, &dev, 0, 12314, NULL));
    tryUvc("open", uvc_open(dev, &devh));
    tryUvc("get_ctrl", 
            uvc_get_stream_ctrl_format_size(
                devh, &ctrl, UVC_FRAME_FORMAT_ANY, 640, 480, 30));

    std::cout << "\n**** begin uvc_print_diag ****\n";
    uvc_print_diag(devh, stdout);
    std::cout << "**** end uvc_print_diag ****\n\n";

    std::cout << "\n**** begin uvc_print_stream_ctrl ****\n";
    uvc_print_stream_ctrl(&ctrl, stdout);
    std::cout << "**** end uvc_print_stream_ctrl ****\n\n";

    uvc_close(devh);
    uvc_unref_device(dev);
    uvc_exit(ctx);

    return 0;
}


