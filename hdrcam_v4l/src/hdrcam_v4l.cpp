#include "Chrono.hpp"

#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <libv4l2.h>
#include <string>
#include <vector>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer_t {
    void   *start;
    size_t length;
};

void xioctl(int fh, int request, void *arg) {

    int r;

    do {
        r = v4l2_ioctl(fh, request, arg);
    } 
    while (r == -1 and (errno == EINTR or errno == EAGAIN));

    if (r == -1) {
        std::cerr << "error " << errno << ", " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char ** argv) {

    const std::string DEVICE_NAME = argc==1 ? "/dev/video0" : argv[1];
    const int WIDTH = 640;
    const int HEIGHT = 480;
    const int tDark = 100;
    const int tBright = 1000;
    const int tSwitch = 50;

    cv::namedWindow("dark");
    cv::namedWindow("bright");

    // open device
    int device = v4l2_open(DEVICE_NAME.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (device < 0) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    // format capture
    struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = WIDTH;
    fmt.fmt.pix.height      = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(device, VIDIOC_S_FMT, &fmt);
    if ((fmt.fmt.pix.width != WIDTH) || (fmt.fmt.pix.height != HEIGHT))
        std::cout << "Warning: driver is sending image at " 
            << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << std::endl;

    // request buffers
    struct v4l2_requestbuffers req;
    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(device, VIDIOC_REQBUFS, &req);

    // init buffers
    std::vector<buffer_t> buffers(req.count);
    unsigned nBuffers = buffers.size();
    std::cout << "nBuffers: " << nBuffers << std::endl;
    for (unsigned iBuffers=0; iBuffers < nBuffers; iBuffers++) {

        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = iBuffers;
        xioctl(device, VIDIOC_QUERYBUF, &buf);

        buffers[iBuffers].length = buf.length;
        buffers[iBuffers].start = v4l2_mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                device, buf.m.offset);

        if (MAP_FAILED == buffers[iBuffers].start) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }

        xioctl(device, VIDIOC_QBUF, &buf);
    }

    // set exposure to manual
    v4l2_set_control(device, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);
    v4l2_set_control(device, V4L2_CID_EXPOSURE_ABSOLUTE, tBright);

    // start streaming
    enum v4l2_buf_type type_streamon;
    type_streamon = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(device, VIDIOC_STREAMON, &type_streamon);

    // process video flux
    Chrono chrono;
    bool isDark = false;
    unsigned usedBuffers = 0;
    while (true) {

        // handle user events
        int key = cv::waitKey(10);
        if (key % 0x100 == 27) 
            break;

        // wait until device is ready
        int res;
        do {
            fd_set fds;
            struct timeval tv;
            FD_ZERO(&fds);
            FD_SET(device, &fds);
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            res = select(device + 1, &fds, NULL, NULL, &tv);
        } while ((res == -1 && (errno = EINTR)));
        if (res == -1) {
            perror("select");
            return errno;
        }

        // dequeue buffer
        struct v4l2_buffer buf;
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(device, VIDIOC_DQBUF, &buf);

        // process data after new controls are used
        usedBuffers++;
        if (chrono.elapsedRunning() > tSwitch and usedBuffers > nBuffers) {

            // get image data
            void * data = buffers[buf.index].start;
            cv::Mat mYUYV(HEIGHT, WIDTH, CV_8UC2, data);
            cv::Mat mBGR(HEIGHT, WIDTH, CV_8UC3);
            cv::cvtColor(mYUYV, mBGR, cv::COLOR_YUV2BGR_YUYV);

            // display frame and prepare for next frame
            if (isDark) {
                cv::imshow("dark", mBGR);
                v4l2_set_control(device, V4L2_CID_EXPOSURE_ABSOLUTE, tBright);
            }
            else {
                cv::imshow("bright", mBGR);
                v4l2_set_control(device, V4L2_CID_EXPOSURE_ABSOLUTE, tDark);
            }
            chrono.restart();
            isDark = not isDark;
            usedBuffers = 0;
        }

        // enqueue buffer
        xioctl(device, VIDIOC_QBUF, &buf);

    }

    // stop streaming
    enum v4l2_buf_type type_streamoff;
    type_streamoff = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(device, VIDIOC_STREAMOFF, &type_streamoff);

    // release buffers
    for (buffer_t & b : buffers)
        v4l2_munmap(b.start, b.length);

    // close device
    v4l2_close(device);

    return 0;
}

