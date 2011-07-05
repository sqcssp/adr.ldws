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

#include <pthread.h>

#include "camera.h"
#include "tvout.h"
#include "buttons.h"
#include "ldws.h"
#include "config.c"


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
#define	VIDEO_FORMATE			V4L2_PIX_FMT_YUV422P

#define VIDIOC_S_TVOUT_ON               _IO ('V', BASE_VIDIOC_PRIVATE+0)
#define VIDIOC_S_TVOUT_OFF              _IO ('V', BASE_VIDIOC_PRIVATE+1)
#define V4L2_CID_CONNECT_TYPE           (V4L2_CID_PRIVATE_BASE+0)

#define V4L2_OUTPUT_TYPE_ANALOG			2
#define V4L2_INPUT_TYPE_MSDMA			3
#define V4L2_INPUT_TYPE_FIFO			4



static int ready_for_ending;
static int fd_mem;
unsigned char *paddr_mem_tv;
long	size_mem;
static int press_key = -1;


static struct buttons_config buttons;
static struct config_camera camera;
static struct config_tvout config;
static struct car_parm_setting car_set;

static enum MSG car_event;

pthread_mutex_t remainMutex;
pthread_cond_t noBuffer;

/*
 * tv-out source buffer address
 * may be change address in the furture
 */
//const long addr_phy_tv = 0x5e366000;
const long addr_phy_tv = 0x5E5F8000;

static void
exit_handler(void)
{
	printf("execute exit handler\n");


	if (paddr_mem_tv != NULL) {
		munmap(paddr_mem_tv, size_mem);
	}

	if(fd_mem > 0) {
		close(fd_mem);
	}


	tvout_close(&config);
	camera_exit(&camera);
	buttons_close(&buttons);

}

static void
signal_init_handler(int signo)
{
	exit(0);
}

void *thread_event(void *nothing)
{
	printf("Detecting Event....\n");
	while(!ready_for_ending) {
		switch(car_event) {
			case WARNING_DRIVING_SAFE:
				printf("=\n");
				break;
			case WARNING_DRIVING_LEFT:
				printf("<\n");
				break;
			case WARNING_DRIVING_RIGHT:
				printf(">\n");
				break;
			case -1:
				printf("P\n");
				break;
		}
		sleep(2);
	}
	return 0;
}

void *thread_buttons(void *nothing)
{
	int num, i;

	printf("Detecting Buttons....\n");

	memset(&buttons, 0, sizeof(buttons));
	buttons_open(&buttons);

	while(!ready_for_ending) {

		num = buttons_get(&buttons);
		if (num < 0) {
			//printf("no buttons press\n");
			//sleep(1);
		} else {
			for (i = 0; i < buttons.num_get_buttons; ++i) {
//				printf("key %d is %s\n", buttons.pbuttons_event[i].key, buttons.pbuttons_event[i].push == '0' ? "up" : "down");
				press_key = buttons.pbuttons_event[i].key;
				if(buttons.pbuttons_event[i].push == '0')
					press_key = -1;
			}
			switch(press_key) {
				case LIGHT_SIGNAL_LEFT:
					printf("L\n");
					break;
				case LIGHT_SIGNAL_RIGHT:
					printf("R\n");
					break;
				default:
					break;
			}
		}
	}
	return 0;
}

void *thread_audio(void *nothing)
{
	while(!ready_for_ending) {
		pthread_mutex_lock(&remainMutex);
//		printf("Receiving Audio....\n");
//		usleep(10000);
		pthread_cond_signal(&noBuffer);
		pthread_mutex_unlock(&remainMutex);
		sleep(rand()%3+1);
	}
	return 0;
}

void *thread_ldws(void *nothing)
{
	int width = 320;
	int height = 240;
	unsigned char *paddr_camera;
	long	size_camera;
	unsigned int i=0;
	char fn[32] = "/sdcard/car.config";


	printf("Starting LDWS....\n");

	if(initlib(fn) >= 0) {
		if (isname("PARAM","carbody_width"))
			car_set.carbody_width = atoi(getvalue("carbody_width"));
		if (isname("PARAM","camera_high_degree"))
			car_set.camera_high_degree = atoi(getvalue("camera_high_degree"));
		tidyup();
	}
	else {
		car_set.carbody_width = 150;
		car_set.camera_high_degree = 50;
	}

	car_set.init_flag = 1;
	car_set.img_width = width;
	car_set.img_height = height;

	printf("/*......Car Setting......*/\n");
	printf("  carbody width = %d cm \n", car_set.carbody_width);
	printf("  camera high degree = %d cm \n", car_set.camera_high_degree);
	printf("  image width = %d pixel \n", car_set.img_width);
	printf("  image height = %d pixel \n", car_set.img_height);
	printf("/*.......................*/\n");


	tvout_open(&config);

	while(!ready_for_ending) {

		paddr_camera = camera_get_one_frame(&camera, &size_camera);

//		pthread_mutex_lock(&remainMutex);

		car_set.pIn_addr = paddr_camera;
		car_set.gps_speed = 70;
		car_set.light_signal = press_key;//LIGHT_SIGNAL_OFF;

//		if ((i > 4) && (i & 1)) {

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

			/* copy Y-field of video */
			/* chang paddr_camera from your Y-field graphic */
			memcpy(paddr_mem_tv, paddr_camera, width*height);
//		}

		config.addr_phy = addr_phy_tv;
		tvout_exe(&config);

		camera_get_one_frame_complete(&camera);

//		pthread_mutex_unlock(&remainMutex);

		i++;
	}

	return 0;
}

void *thread_record(void *nothing)
{
	while(!ready_for_ending) {
		pthread_mutex_lock(&remainMutex);
		pthread_cond_wait(&noBuffer, &remainMutex);
//		printf("Recording SD Card....\n");
//		usleep(50000);
		pthread_cond_signal(&noBuffer);
		pthread_mutex_unlock(&remainMutex);
		sleep(rand()%5+1);
	}
	return 0;
}

void *thread_video(void *nothing)
{
	while(!ready_for_ending) {
		pthread_mutex_lock(&remainMutex);
//		printf("Receiving Video....\n");
//		usleep(20000);
		pthread_cond_signal(&noBuffer);
		pthread_mutex_unlock(&remainMutex);
		sleep(rand()%4+1);
	}
	return 0;
}

int IOrun()
{
	pthread_t _thread_event, _thread_buttons;

//	pthread_create(&_thread_event, NULL, (void *)thread_event, (void *)0);
	pthread_create(&_thread_buttons, NULL, (void *)thread_buttons, (void *)0);
	usleep(1);

	return 0;
}

int CSrun()
{
	pthread_t _thread_ldws;

	pthread_create(&_thread_ldws, NULL, (void *)thread_ldws, (void *)0);
	usleep(1);

	return 0;
}

int DSrun()
{
	pthread_t _thread_record;

	pthread_create(&_thread_record, NULL, (void *)thread_record, (void *)0);
	usleep(1);

	return 0;
}

int AVrun()
{
	pthread_t _thread_audio, _thread_video;

	pthread_create(&_thread_audio, NULL, (void *)thread_audio, (void *)0);
	usleep(1);
	pthread_create(&_thread_video, NULL, (void *)thread_video, (void *)0);
	usleep(1);

	return 0;
}

int main(int argc, char* argv[])
{
	int ret;
	int width = 320;
	int height = 240;

	ready_for_ending = 0;

	if(atexit(exit_handler) != 0) {
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, signal_init_handler) == SIG_ERR) {
		exit(EXIT_FAILURE);
	}

	IOrun();

	DSrun();

	AVrun();

	config.width = width;
	config.height = height;
	config.format = V4L2_PIX_FMT_YUV420;
	//config.format = V4L2_PIX_FMT_YUV422P;


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

	CSrun();

	while(!ready_for_ending) {
	};

	return 0;
}
