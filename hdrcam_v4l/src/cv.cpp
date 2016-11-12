#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <libv4l2.h>
#include <string>

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
        //fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main() {

    struct v4l2_format              fmt;
    struct v4l2_buffer              buf;
    struct v4l2_requestbuffers      req;
    enum v4l2_buf_type              type;
    fd_set                          fds;
    struct timeval                  tv;
    int                             r, fd = -1;
    unsigned int                    i, n_buffers;
    char                            dev_name[] = "/dev/video0";
    buffer_t                   *buffers;

    fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        perror("Cannot open device");
        exit(EXIT_FAILURE);
    }

    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = 640;
    fmt.fmt.pix.height      = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    xioctl(fd, VIDIOC_S_FMT, &fmt);
    if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
        printf("Warning: driver is sending image at %dx%d\\n",
                fmt.fmt.pix.width, fmt.fmt.pix.height);

    CLEAR(req);
    req.count = 2;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    xioctl(fd, VIDIOC_REQBUFS, &req);

    // set exposure to manual
    v4l2_control c;
    c.id = V4L2_CID_EXPOSURE_AUTO;
    c.value = V4L2_EXPOSURE_MANUAL;
    xioctl(fd, VIDIOC_S_CTRL, &c);

    buffers = (buffer_t*)calloc(req.count, sizeof(*buffers));
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        CLEAR(buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        xioctl(fd, VIDIOC_QUERYBUF, &buf);

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                PROT_READ | PROT_WRITE, MAP_SHARED,
                fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start) {
            perror("mmap");
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < n_buffers; ++i) {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(fd, VIDIOC_QBUF, &buf);
    }
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    xioctl(fd, VIDIOC_STREAMON, &type);
    for (i = 0; i < 20; i++) {
        do {
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));
        if (r == -1) {
            perror("select");
            return errno;
        }

    v4l2_control c;
    c.id = V4L2_CID_EXPOSURE_ABSOLUTE;
    c.value = 100;
    xioctl(fd, VIDIOC_S_CTRL, &c);

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);

        void * data = buffers[buf.index].start;
        cv::Mat mYUYV(480, 640, CV_8UC2, data);
        cv::Mat mBGR(480, 640, CV_8UC3);
        cv::cvtColor(mYUYV, mBGR, cv::COLOR_YUV2BGR_YUYV);
        cv::imwrite("frame_"+std::to_string(i)+".png", mBGR);

        xioctl(fd, VIDIOC_QBUF, &buf);
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (i = 0; i < n_buffers; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
    v4l2_close(fd);

    return 0;
}

