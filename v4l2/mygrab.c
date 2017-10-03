#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <assert.h>
#include <linux/videodev2.h>

//#include "../libv4l/include/libv4l2.h"
#include<libv4l2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))


static int xioctl(int fd, int request, void *arg)
{
    int r;
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
}


int main(){
	//define constants
	char *dev_name = "/dev/video0";
	int fd = -1;
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers reqbuf;
	struct {
		void *start;
		size_t length;
	} *buffers;
	//struct buffer *buffers
	unsigned int i,n_buffers;
	//opening camera
	fd = v4l2_open(dev_name,O_RDWR|O_NONBLOCK,0);
	if(fd < 0){
		perror("Cannot open device!\n");
		exit(EXIT_FAILURE);
	}
	//querying camera capabilities
	if(-1==xioctl(fd,VIDIOC_QUERYCAP,&cap)){
		perror("Cannot query camera capabilities!\n");
		exit(EXIT_FAILURE);
	}
	printf("Capabilities: 0x%x\n",cap.capabilities);
	printf("Device Caps:  0x%x\n",cap.device_caps);
	//choose camera format
	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; /*https://www.linuxtv.org/downloads/v4l-dvb-apis-new/uapi/v4l/dev-capture.html#capture */
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(-1==xioctl(fd,VIDIOC_S_FMT,&fmt)){
		perror("Cannot set video format!\n");
		exit(EXIT_FAILURE);
	}
	if ((fmt.fmt.pix.width != 640)||(fmt.fmt.pix.height!=480)){
		printf("Warning: driver is sending image at %dx%d\n",
				fmt.fmt.pix.width,fmt.fmt.pix.height);
			}
	//requesting buffers
	CLEAR(reqbuf);
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = 20;
	if(-1==xioctl(fd,VIDIOC_REQBUFS,&reqbuf)){
		if(errno == EINVAL){
			printf("Video capturing or mmap streaming is not supported\n");
		}
		else{
			perror("Cannot request buffers\n");
		}
		exit(EXIT_FAILURE);
	}
	//getting buffers to write to
	if (reqbuf.count < 5) {
		/* You may need to free the buffers here. */
		printf("Not enough buffer memory\\n");
		exit(EXIT_FAILURE);
		}
	buffers = calloc(reqbuf.count, sizeof(*buffers));
	assert(buffers != NULL);
	struct v4l2_buffer buf;
	for(i=0;i<reqbuf.count;i++){
		CLEAR(buf);
		buf.type = reqbuf.type;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		if(-1==xioctl(fd,VIDIOC_QBUF,&buf)){
			perror("Cannot query buffers\n");
			exit(EXIT_FAILURE);
		}
		buffers[i].length = buf.length; /* remember for munmap() */
		buffers[i].start = mmap(NULL, buf.length,
			PROT_READ | PROT_WRITE, /* recommended */
			MAP_SHARED,             /* recommended */
			fd, buf.m.offset);
		if (MAP_FAILED == buffers[i].start) {
			/* If you do not exit here you should unmap() and free()
			 * the buffers mapped so far. */
			 perror("mmap");
			 exit(EXIT_FAILURE);
		 }
	 }
	 if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
	 {
		 perror("Start Capture");
		 return 1;
		 }
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	struct timeval tv = {0};
	tv.tv_sec = 2;
	int r = select(fd+1, &fds, NULL, NULL, &tv);
	if(-1 == r)
	{
		perror("Waiting for Frame");
		return 1;
	}

	if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
	{
		perror("Retrieving Frame");
		return 1;
	}
	//writint to a file

  const int dimx = 600, dimy = 480;
  int j=0;
  i=0;
  FILE *fp = fopen("first.ppm", "wb"); /* b - binary mode */
  (void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
  for (j = 0; j < dimy; ++j)
  {
    for (i = 0; i < dimx; ++i)
    {
      static unsigned char color[3];
      color[0] = i % 256;  /* red */
      //color[1] = j % 256;  /* green */
      //color[2] = (i * j) % 256;  /* blue */
      (void) fwrite(color, 1, 1, fp);
    }
  }
  (void) fclose(fp);
  return EXIT_SUCCESS;

}
