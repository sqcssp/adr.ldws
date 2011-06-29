#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>


/* i2c dwonload initial code */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <assert.h>

#include "camera.h"
#include "tvout.h"
#include "ldws.h"

#define	WIDTH_VGA			640
#define	HEIGHT_VGA			320

#define	WIDTH_QVGA			320
#define	HEIGHT_QVGA			240

#define	WIDTH_QQVGA			160
#define	HEIGHT_QQVGA			120

#define	BYTES_PER_PIXEL_YUV422		2
#define	SIZE				WIDTH*HEIGHT*BYTES_PER_PIXEL

#define	DEVICE_CAMERA			"/dev/video14"
#define	FILE_TEMP			"/tmp/video.yuv"

/* this value could only set YUV420 and RGB565 now */
#define	VIDEO_FORMATE			V4L2_PIX_FMT_YUV420

#define VIDIOC_S_TVOUT_ON               _IO ('V', BASE_VIDIOC_PRIVATE+0)
#define VIDIOC_S_TVOUT_OFF              _IO ('V', BASE_VIDIOC_PRIVATE+1)
#define V4L2_CID_CONNECT_TYPE           (V4L2_CID_PRIVATE_BASE+0)

#define V4L2_OUTPUT_TYPE_ANALOG			2
#define V4L2_INPUT_TYPE_MSDMA			3
#define V4L2_INPUT_TYPE_FIFO			4


#define I2C_RETRIES             0x0701
#define I2C_TIMEOUT             0x0702
#define I2C_I2C_SLAVE_FORCE     0x0706
#define I2C_RDWR                0x0707

#define I2C_M_TEN       0x0010
#define I2C_M_RD        0x0001

#ifdef _DEBUG
#include <stdio.h>
#define DEBUG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define DEBUG(args...)
#endif


static int fd_mem = -1;
static int fd_i2c = -1;
unsigned char *paddr_mem_tv;
long	size_mem;


static struct config_camera camera;
static struct config_tvout config;


struct i2c_msg
{
	__u16 addr;
	__u16 flags;
	__u16 len;
	__u8 *buf;
};

struct i2c_rdwr_ioctl_data
{
	struct i2c_msg *msgs;
	int nmsgs;
};


static char i2c_read_byte(char id,char addr)
{
	struct i2c_rdwr_ioctl_data alldata;

	int ret;
	
	
	alldata.nmsgs = 2;
	alldata.msgs = (struct i2c_msg*)malloc(alldata.nmsgs * sizeof(struct i2c_msg));
	if(!alldata.msgs){
		DEBUG("Memory alloc error!\n");
	close(fd_i2c);
	return 0;
	}

	(alldata.msgs[0]).addr   = id;                   //slave address
	(alldata.msgs[0]).flags  = 0;                      //flag = 0(write) , 1(read)
	(alldata.msgs[0]).len    = 1;
	(alldata.msgs[0]).buf    = (__u8 *)malloc(1);
	(alldata.msgs[0]).buf[0] = addr;                   //sub address (offset)
	(alldata.msgs[1]).addr   = id;
	(alldata.msgs[1]).flags  = I2C_M_RD;
	(alldata.msgs[1]).len    = 1;
	(alldata.msgs[1]).buf    = (__u8 *)malloc(1);
	(alldata.msgs[1]).buf[0] = 0;

	ret = ioctl(fd_i2c,I2C_RDWR,(unsigned long)&alldata);
	if(ret < 0) {
		DEBUG("R ER id = 0x%2x ,address = 0x%2x val=0x%2x ,ret=0x%x\n",id,addr,(alldata.msgs[1]).buf[0],ret);	
	} else {
		DEBUG("R OK id = 0x%2x ,address = 0x%2x val=0x%2x\n",id,addr,(alldata.msgs[1]).buf[0]);
	}

	free((alldata.msgs[0]).buf);
	free((alldata.msgs[1]).buf);
	free(alldata.msgs);	
	
	return alldata.msgs[0].buf[1];
}


static int i2c_write_byte(char id,char addr,char val)
{
	struct i2c_rdwr_ioctl_data alldata;
	int ret;

	alldata.nmsgs = 1;
	alldata.msgs = (struct i2c_msg*)malloc(alldata.nmsgs * sizeof(struct i2c_msg));
	if(!alldata.msgs){
		DEBUG("Memory alloc error!\n");
	close(fd_i2c);
	return 0;
	}

	(alldata.msgs[0]).addr   = id;
	(alldata.msgs[0]).flags  = 0;
	(alldata.msgs[0]).len    = 2;
	(alldata.msgs[0]).buf    = (__u8 *)malloc(2);
	(alldata.msgs[0]).buf[0] = addr;
	(alldata.msgs[0]).buf[1] = val;  //要寫的值

	ret = ioctl(fd_i2c,I2C_RDWR,(unsigned long)&alldata);
	if(ret < 0)
		DEBUG("W ER id = 0x%2x ,address = 0x%2x val=0x%2x ret=0x%4x\n",id,addr,val,ret);
	else
		DEBUG("W OK id = 0x%2x ,address = 0x%2x val=0x%2x \n",id,addr,val);	
	
	
	free((alldata.msgs[0]).buf);
	free(alldata.msgs);
//	sleep(1);  //沒有sleep,會無法讀取.	
	
	return 0;		
}

static int i2c_download_adv7180_init(void)
{
	int ret;

	DEBUG("Open device : /dev/i2c/0\n");
	fd_i2c = open("/dev/i2c/0", O_RDWR);
	if(0 > fd_i2c){
		DEBUG("Error on opening the device file!\n");
		return -1;
	}

	ioctl(fd_i2c,I2C_TIMEOUT,1);
	ioctl(fd_i2c,I2C_RETRIES,1);

	ret = i2c_write_byte(0x21,0x0F,0x34);
	ret = i2c_write_byte(0x20,0x00,0x00);
	ret = i2c_write_byte(0x20,0x04,0x57);
	ret = i2c_write_byte(0x20,0x17,0x41);
	ret = i2c_write_byte(0x20,0x31,0x02);
	ret = i2c_write_byte(0x20,0x3D,0xA2);
	ret = i2c_write_byte(0x20,0x3E,0x6A);
	ret = i2c_write_byte(0x20,0x3F,0xA0);
	ret = i2c_write_byte(0x20,0x0E,0x80);
	ret = i2c_write_byte(0x20,0x55,0x81);
	ret = i2c_write_byte(0x20,0x0E,0x00);

	return ret;
}


/*
 * tv-out source buffer address
 * may be change address in the furture
 */
const long addr_phy_tv = 0x5e366000;
//const long addr_phy_tv = 0x5E5F8000;

static void
exit_handler(void)
{
	DEBUG("execute exit handler\n");


	if (paddr_mem_tv != NULL) {
		munmap(paddr_mem_tv, size_mem);
	}

	if(fd_mem >= 0) {
		close(fd_mem);
		fd_mem = -1;
	}


	if (fd_i2c >= 0) {
		close(fd_i2c);
		fd_i2c = -1;
	}

	tvout_close(&config);
	camera_exit(&camera);

}

static void
signal_init_handler(int signo)
{
	exit(0);
}

int main(int argc, char* argv[])
{
	int ret;

	if(atexit(exit_handler) != 0) {
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, signal_init_handler) == SIG_ERR) {
		exit(EXIT_FAILURE);
	}

	i2c_download_adv7180_init();
	

	int width = 320;
	int height = 240;


	config.width = width;
	config.height = height;
	config.format = V4L2_PIX_FMT_YUV420;
	//config.format = V4L2_PIX_FMT_YUV422P;


	tvout_open(&config);

	fd_mem  = open( "/dev/mem", O_RDWR ); 
//	paddr_mem_tv = mmap( 0, size_mem, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, addr_phy_tv );
//	memset(paddr_mem_tv, 0x80, size_mem);
	size_mem = width * height * 2;
	paddr_mem_tv = mmap( 0, size_mem, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, addr_phy_tv );
	memset(paddr_mem_tv, 0x80, size_mem);


	camera.format.width = width;                            /* set output frame width */
	camera.format.height = height;                          /* set output frame height */
	camera.format.pixelformat = V4L2_PIX_FMT_YUV422P;
	//camera.format.pixelformat = V4L2_PIX_FMT_YUYV;

	ret = camera_init(&camera);
	if(0 > ret) {
		perror("open camera device error");
		exit(EXIT_FAILURE);
	}

	unsigned char *paddr_camera;
	long	size_camera;

	struct timeval start,end;
	long timeuse = 0;
	long time_max = 0;
	long time_temp = 0;
	int i = 0;

	static struct car_parm_setting car_set;
	car_set.carbody_width = 150;
	car_set.camera_high_degree = 50;
	car_set.init_flag = 1;
	car_set.img_width = width;
	car_set.img_height = height;

	while (1) {

//		paddr_camera = camera_get_one_frame_noneblock(&camera, &size_camera);
//
		gettimeofday( &start, NULL );

		paddr_camera = camera_get_one_frame(&camera, &size_camera);
		car_set.pIn_addr = paddr_camera;
		car_set.gps_speed = 70;
		car_set.light_signal = LIGHT_SIGNAL_OFF;

		paddr_camera = set_car_parm(&car_set);
		switch(car_set.car_event) {
			case WARNING_DRIVING_SAFE:
				printf("=\n");
				break;
			case WARNING_DRIVING_LEFT:
				printf("<\n");
				break;
			case WARNING_DRIVING_RIGHT:
				printf(">\n");
				break;
		}

		//car_event = car_set.car_event;
		paddr_camera = car_set.pOut_addr;

		memcpy(paddr_mem_tv, paddr_camera, width*height); 
		camera_get_one_frame_complete(&camera);

		config.addr_phy = addr_phy_tv;
		tvout_exe(&config);


		gettimeofday( &end, NULL );
		time_temp = 1000000 * ( end.tv_sec - start.tv_sec )+ (end.tv_usec - start.tv_usec);
		if (time_temp > time_max) {
			time_max = time_temp;
		}
		timeuse +=time_temp;
		DEBUG("time used: % 7ld us\n", time_temp);


		++i;
		if ( timeuse > 10*1000000) {
			printf("get %d frames spent % 7ld msec  average frame rate = %ld\n", i, timeuse, i*1000000/timeuse);
			i = 0;
			timeuse = 0;
		}
	}

	return 0;
}
