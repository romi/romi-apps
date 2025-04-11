/*
  romi-rover

  Copyright (C) 2019 Sony Computer Science Laboratories
  Author(s) Peter Hanappe

  romi-rover is collection of applications for the Romi Rover.

  romi-rover is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

 */
/* 

    P2P Food Lab Sensorbox

    Copyright (C) 2013  Sony Computer Science Laboratory Paris
    Author: Peter Hanappe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*

  camera.c is based on Tobias Müller's v4l2grab Version 0.1. Original
  copyright below:

  See also https://www.linuxtv.org/downloads/v4l-dvb-apis-old/v4l2grab-example.html

*/

/***************************************************************************
 *   v4l2grab Version 0.1                                                  *
 *   Copyright (C) 2009 by Tobias Müller                                   *
 *   Tobias_Mueller@twam.info                                              *
 *                                                                         *
 *   based on V4L2 Specification, Appendix B: Video Capture Example        *
 *   (http://v4l2spec.bytesex.org/spec/capture-example.html)               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include "util/Logger.h"
#include "camera_v4l.h"


#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef struct _buffer_t {
        void * start;
        size_t length;
} buffer_t;

enum {
        CAMERA_CLEAN,
        CAMERA_OPEN,
        CAMERA_INIT,
        CAMERA_CAPTURING,
        CAMERA_ERROR
};

typedef struct _camera_t {
        int fd;
        char* device_name;
        size_t width;
        size_t height;
        unsigned int frame_rate;
        buffer_t* buffers;
        unsigned int n_buffers;
        unsigned char* rgb_buffer;
        size_t rgb_buffer_size;
        int state;
} camera_t;

static int camera_prepare(camera_t *camera);
static int camera_open(camera_t *camera);
static int camera_init(camera_t *camera);
static int camera_capturestart(camera_t *camera);
static int camera_capturestop(camera_t *camera);
static int camera_close(camera_t *camera);
static int camera_cleanup(camera_t *camera);
static int camera_read(camera_t *camera);
static int camera_convert(camera_t *camera, void* p);
static int camera_set_framerate(camera_t *camera);

static void convert_yuv422_to_rgb888(size_t width, size_t height, unsigned char *src,
                                     unsigned char *dst);

static int camera_mmapinit(camera_t *camera);
static int xioctl(int fd, unsigned long request, void* argp);


camera_t* new_camera(const char* dev, size_t width, size_t height)
{
        camera_t *camera = (camera_t*) malloc(sizeof(camera_t));
        if (camera == nullptr) {
                r_err("Camera: Out of memory");
                return nullptr;
        }
        memset(camera, 0, sizeof(camera_t));
        
        camera->fd = -1;
        camera->device_name = strdup(dev);
        camera->width = width;
        camera->height = height;
        camera->frame_rate = 1;
        camera->buffers = nullptr;
        camera->n_buffers = 0;
        camera->rgb_buffer = nullptr;
        camera->rgb_buffer_size = 0;
        camera->state = CAMERA_CLEAN;

        if (camera_prepare(camera) != 0) {
                r_err("Camera: Failed to prepare the camera");
                delete_camera(camera);
                return nullptr;
        }
        
        return camera;
}

int delete_camera(camera_t *camera)
{
        if (camera) {
                camera_capturestop(camera);
                camera_close(camera);
                camera_cleanup(camera);
                free(camera);
        }
        return 0;
}

static int camera_prepare(camera_t *camera)
{
        if (camera->state == CAMERA_ERROR)
                return -1;

        if ((camera->state == CAMERA_CLEAN) 
            && (camera_open(camera) != 0)) {
                camera->state = CAMERA_ERROR;
                return -1;
        }

        if ((camera->state == CAMERA_OPEN) 
            && (camera_init(camera) != 0)) {
                camera->state = CAMERA_ERROR;
                camera_close(camera);
                camera_cleanup(camera);
                return -1;
        }        

        if ((camera->state == CAMERA_INIT) 
            && (camera_capturestart(camera) != 0)) {
                camera->state = CAMERA_ERROR;
                camera_close(camera);
                camera_cleanup(camera);
                return -1;
        }

        return 0;
}

int camera_capture(camera_t *camera)
{
        int error;

        if ((camera->state != CAMERA_CAPTURING) 
            && (camera_prepare(camera) != 0)) {
                return -1;
        }
        
        for (int again = 0; again < 10; again++) {
                fd_set fds;
                struct timeval tv;
                int r;

                FD_ZERO(&fds);
                FD_SET(camera->fd, &fds);

                /* Timeout. */
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                r = select(camera->fd + 1, &fds, nullptr, nullptr, &tv);

                if (-1 == r) {
                        if (EINTR == errno)
                                continue;

                        r_err("Camera: select error %d, %s", errno, strerror(errno));
                        return -1;
                }

                if (0 == r) {
                        r_err("Camera: select timeout");
                        continue;
                }
                        
                error = camera_read(camera);
                if (error == 0) 
                        break;

                // else: EAGAIN - continue select loop. */
        }

        if (error) {
                r_err("Camera: Failed to grab frame");
                return -1;
        }

        return 0;
}

size_t camera_width(camera_t* camera)
{
        return camera->width;
}

size_t camera_height(camera_t* camera)
{
        return camera->height;
}

uint8_t *camera_getimagebuffer(camera_t *camera)
{
        return camera->rgb_buffer;
}

/**

   Convert from YUV422 format to RGB888. Formulae are described on
   http://en.wikipedia.org/wiki/YUV

   \param width width of image
   \param height height of image
   \param src source
   \param dst destination
*/

#define CLIP(x) (((x) >= 255.0f)? 255 : (((x) <= 0.0f)? 0 : (x)))

float clamp(float d, float min, float max) {
        const float t = d < min ? min : d;
        return t > max ? max : t;
}

void convert_yuv422_to_rgb888(size_t width, size_t height, unsigned char *src, unsigned char *dst)
{
        unsigned char *py, *pu, *pv;
        unsigned char *tmp = dst;
        float y, u, v;
        float r, g, b;

        /* In this format each four bytes is two pixels. Each four
           bytes is two Y's, a Cb and a Cr.  Each Y goes to one of the
           pixels, and the Cb and Cr belong to both pixels. */
        py = src;
        pu = src + 1;
        pv = src + 3;

        for (size_t line = 0; line < height; line++) {
                for (size_t column = 0; column < width; column++) {

                        y = (float) *py;
                        u = (float) *pu;
                        v = (float) *pv;
                        
                        r = y - 0.344f * (u - 128.0f) - 0.714f * (v - 128.0f);
                        g = y + 1.772f * (u - 128.0f);
                        b = y + 1.402f * (v - 128.0f);
                        
                        *tmp++ = (uint8_t)clamp(r, 0, 255);
                        *tmp++ = (uint8_t)clamp(g, 0, 255);
                        *tmp++ = (uint8_t)clamp(b, 0, 255);

                        py += 2;

                        // increase pu, pv every second time
                        if ((column & 1) == 1) {
                                pu += 4;
                                pv += 4;
                        }
                }
        }
}

/**

   Do ioctl and retry if error was EINTR ("A signal was caught during
   the ioctl() operation."). Parameters are the same as on ioctl.

   \param fd file descriptor
   \param request request
   \param argp argument
   \returns result from ioctl
*/
static int xioctl(int fd, unsigned long request, void* argp)
{
        int r;
        do {
                r = ioctl(fd, request, argp);
        } while (-1 == r && EINTR == errno);

        return r;
}

// https://gist.github.com/TIS-Edgar/e2bd764e5594897d1835
static int camera_set_framerate(camera_t *camera)
{
        struct v4l2_streamparm parm;

        parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        parm.parm.capture.timeperframe.numerator = 1;
        parm.parm.capture.timeperframe.denominator = camera->frame_rate;

        int err = xioctl(camera->fd,  VIDIOC_S_PARM, &parm);

        r_info("Camera: frame rate is set to: %d", (int) parm.parm.capture.timeperframe.denominator);

        return err;
}

/**
   process image read
*/
static int camera_convert(camera_t *camera, void* src)
{
        size_t size = camera->width * camera->height * 3;
        
        if ((camera->rgb_buffer_size != size) 
            && (camera->rgb_buffer != nullptr)) {
                free(camera->rgb_buffer);
                camera->rgb_buffer = nullptr;
        }
        if (camera->rgb_buffer == nullptr) {
                camera->rgb_buffer = (uint8_t *)malloc(size);
                if (camera->rgb_buffer == nullptr) {
                        r_err("Camera: Out of memory");
                        return -1;
                }
                camera->rgb_buffer_size = size;
        }

        convert_yuv422_to_rgb888(camera->width, camera->height, (uint8_t *)src, camera->rgb_buffer);
        
        return 0;
}

static int camera_read(camera_t *camera)
{
        struct v4l2_buffer buf;

        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
 
        if (-1 == xioctl(camera->fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                case EAGAIN:
                        return 1;

                case EIO:
                        // Could ignore EIO, see spec

                        // fall through
                default:
                        r_err("Camera: VIDIOC_DQBUF error %d, %s", errno, strerror(errno));
                        return -1;
                }
        }

        assert(buf.index < camera->n_buffers);

        camera_convert(camera, camera->buffers[buf.index].start);

        if (-1 == xioctl(camera->fd, VIDIOC_QBUF, &buf)) {
                r_err("Camera: VIDIOC_QBUF error %d, %s", errno, strerror(errno));
                return -1;
        }

        return 0;
}

static int camera_capturestop(camera_t *camera)
{
        enum v4l2_buf_type type;

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                
        if (camera->fd == -1)
                return 0;

        if (-1 == xioctl(camera->fd, VIDIOC_STREAMOFF, &type)) {
                r_err("Camera: VIDIOC_STREAMOFF error %d, %s", errno, strerror(errno));
                return -1;
        }

        return 0;
}

static int camera_capturestart(camera_t *camera)
{
        unsigned int i;
        enum v4l2_buf_type type;

        if (camera->state != CAMERA_INIT) {
                r_err("Camera: Not in the init state");
                return -1;
        }

        for (i = 0; i < camera->n_buffers; ++i) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = i;

                if (-1 == xioctl(camera->fd, VIDIOC_QBUF, &buf)) {
                        r_err("Camera: VIDIOC_QBUF error %d, %s", errno, strerror(errno));
                        return -1;
                }
        }
                
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == xioctl(camera->fd, VIDIOC_STREAMON, &type)) {
                r_err("Camera: VIDIOC_STREAMON error %d, %s", errno, strerror(errno));
                return -1;
        }

        camera->state = CAMERA_CAPTURING;

        return 0;
}

static int camera_cleanup(camera_t *camera)
{
        unsigned int i;

        if (camera == nullptr)
                return 0;

        for (i = 0; i < camera->n_buffers; ++i)
                if (-1 == munmap(camera->buffers[i].start, camera->buffers[i].length)) {
                        r_err("Camera: munmap error %d, %s", errno, strerror(errno));
                        return -1;
                }
        
        if (camera->buffers)
                free(camera->buffers);

        if (camera->device_name != nullptr)
                free(camera->device_name);

        if (camera->rgb_buffer != nullptr) 
                free(camera->rgb_buffer);

        return 0;
}


static int camera_mmapinit(camera_t *camera)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(camera->fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        r_err("Camera: %s does not support memory mapping",
                              camera->device_name);
                        return -1;
                } else {
                        r_err("Camera: VIDIOC_REQBUFS error %d, %s", errno,
                              strerror(errno));
                        return -1;
                }
        }

        if (req.count < 2) {
                r_err("Camera: Insufficient buffer memory on %s", camera->device_name);
                return -1;
        }

        camera->buffers = (buffer_t*)calloc(req.count, sizeof(buffer_t));
        if (!camera->buffers) {
                r_err("Camera: Out of memory");
                return -1;
        }

        for (camera->n_buffers = 0; camera->n_buffers < req.count; camera->n_buffers++) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = camera->n_buffers;

                if (-1 == xioctl(camera->fd, VIDIOC_QUERYBUF, &buf)) {
                        r_err("Camera: VIDIOC_QUERYBUF error %d, %s", errno,
                              strerror(errno));
                        return -1;
                }
                camera->buffers[camera->n_buffers].length = buf.length;
                camera->buffers[camera->n_buffers].start = mmap(nullptr /* start anywhere */, 
                                                                buf.length, 
                                                                PROT_READ | PROT_WRITE, 
                                                                MAP_SHARED /*recommended*/, 
                                                                camera->fd, (off_t) buf.m.offset);

                if (MAP_FAILED == camera->buffers[camera->n_buffers].start) {
                        r_err("Camera: mmap error %d, %s", errno, strerror(errno));
                        return -1;
                }
        }

        return 0;
}


static int camera_open(camera_t *camera)
{
        struct stat st;

        if (camera->state != CAMERA_CLEAN) {
                r_err("Camera: Not in the clean state");
                return -1;
        }

        // stat file

        if (-1 == stat(camera->device_name, &st)) {
                r_err("Camera: Cannot identify '%s': %d, %s", camera->device_name,
                      errno, strerror(errno));
                return -1;
        }

        // check if its device
        if (!S_ISCHR (st.st_mode)) {
                r_err("Camera: %s is no device", camera->device_name);
                return -1;
        }

        // open device
        r_err("Camera: Opening device '%s'", camera->device_name);
        
        camera->fd = open(camera->device_name, O_RDWR | O_NONBLOCK, 0);

        // check if opening was successfull
        if (-1 == camera->fd) {
                r_err("Camera: Cannot open '%s': %d, %s", camera->device_name,
                      errno, strerror(errno));
                return -1;
        }

        camera->state = CAMERA_OPEN;

        return 0;
}

static int camera_setctrl(camera_t *camera, unsigned int id, int value)
{
        struct v4l2_queryctrl queryctrl;
        struct v4l2_control control;

        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = id;

        if (-1 == ioctl(camera->fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (errno != EINVAL) {
                        r_err("Camera: VIDIOC_QUERYCTRL: error %d, %s", errno,
                              strerror(errno));
                        return -1;
                } else {
                        r_warn("Camera: control %d is not supported", id);
                }
                
        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                r_warn("Camera: Control %d is not supported", id);
                
        } else {
                memset(&control, 0, sizeof (control));
                control.id = id;
                control.value = value;
                if (-1 == ioctl(camera->fd, VIDIOC_S_CTRL, &control)) {
                        r_err("Camera: VIDIOC_S_CTRL: error %d, %s", errno,
                              strerror(errno));
                        return -1;
                }
        }
        return 0;
}

static int camera_setctrl_default(camera_t *camera, unsigned int id)
{
        struct v4l2_queryctrl queryctrl;
        struct v4l2_control control;

        memset(&queryctrl, 0, sizeof(queryctrl));
        queryctrl.id = id;

        if (-1 == ioctl(camera->fd, VIDIOC_QUERYCTRL, &queryctrl)) {
                if (errno != EINVAL) {
                        r_err("Camera: VIDIOC_QUERYCTRL: error %d, %s", errno,
                              strerror(errno));
                        return -1;
                } else {
                        r_warn("Camera: control %d is not supported", id);
                }
                
        } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
                r_warn("Camera: Control %d is not supported", id);
                
        } else {
                memset(&control, 0, sizeof (control));
                control.id = id;
                control.value = queryctrl.default_value;
                if (-1 == ioctl(camera->fd, VIDIOC_S_CTRL, &control)) {
                        r_err("Camera: VIDIOC_S_CTRL: error %d, %s", errno,
                              strerror(errno));
                        return -1;
                }
        }
        return 0;
}

static int camera_init(camera_t *camera)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if (camera->state != CAMERA_OPEN) {
                r_err("Camera: Not in the open state");
                return -1;
        }

        if (-1 == xioctl(camera->fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        r_err("Camera: %s is no V4L2 device", camera->device_name);
                        return -1;
                } else {
                        r_err("Camera: VIDIOC_QUERYCAP error %d, %s", errno,
                              strerror(errno));
                        return -1;
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                r_err("Camera: %s is no video capture device", camera->device_name);
                return -1;
        }

        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                r_err("Camera: %s does not support streaming i/o", camera->device_name);
                return -1;
        }


        /* Select video input, video standard and tune here. */
        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(camera->fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl(camera->fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {        
                /* Errors ignored. */
        }

        CLEAR(fmt);

        r_info("Camera: Opening video device %s, %dx%d.", camera->device_name,
               camera->width, camera->height);

        // v4l2_format
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = (uint32_t)camera->width;
        fmt.fmt.pix.height = (uint32_t)camera->height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;        
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl(camera->fd, VIDIOC_S_FMT, &fmt)) {
                r_err("Camera: VIDIOC_S_FMT error %d, %s", errno, strerror(errno));
                return -1;
        }

        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) {
                r_err("Camera: failed to set YUYV format. ");
                return -1;
        }
        
        /* Note VIDIOC_S_FMT may change width and height. */
        if (camera->width != fmt.fmt.pix.width) {
                camera->width = fmt.fmt.pix.width;
                r_err("Camera: Image width set to %i by device %s.",
                      camera->width, camera->device_name);
        }
        
        if (camera->height != fmt.fmt.pix.height) {
                camera->height = fmt.fmt.pix.height;
                r_err("Camera: Image height set to %i by device %s.",
                      camera->height, camera->device_name);
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        if (camera_mmapinit(camera) != 0) {
                return -1;
        }

        if (camera_set_framerate(camera) != 0) {
                return -1;
        }
        
        camera_setctrl_default(camera, V4L2_CID_BRIGHTNESS);
        camera_setctrl_default(camera, V4L2_CID_CONTRAST);
        camera_setctrl_default(camera, V4L2_CID_SATURATION);
        camera_setctrl_default(camera, V4L2_CID_HUE);
        camera_setctrl_default(camera, V4L2_CID_GAMMA);
        camera_setctrl_default(camera, V4L2_CID_GAIN);
        camera_setctrl(camera, V4L2_CID_AUTO_WHITE_BALANCE, 1);
        camera_setctrl_default(camera, V4L2_CID_SHARPNESS);
        camera_setctrl_default(camera, V4L2_CID_BACKLIGHT_COMPENSATION);


        /* camera_setctrl(camera, V4L2_CID_CONTRAST, 127); */
        /* camera_setctrl(camera, V4L2_CID_BRIGHTNESS, 127); */
        /* camera_setctrl(camera, V4L2_CID_HUE, 127);         */
        /* camera_setctrl(camera, V4L2_CID_GAMMA, 127); */
        /* camera_setctrl(camera, V4L2_CID_RED_BALANCE, 127); */
        /* camera_setctrl(camera, V4L2_CID_BLUE_BALANCE, 127); */
        /* camera_setctrl(camera, V4L2_CID_EXPOSURE, 127); */
        /* camera_setctrl_default(camera, V4L2_CID_BRIGHTNESS); */
        /* camera_setctrl(camera, V4L2_CID_AUTOGAIN, 1); */
 
        camera->state = CAMERA_INIT;

        return 0;
}

static int camera_close(camera_t *camera)
{
        if (camera->fd == -1)
                return 0;

        int oldfd = camera->fd;
        camera->fd = -1;
        return close(oldfd);
}
