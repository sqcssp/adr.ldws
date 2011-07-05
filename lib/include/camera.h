#ifndef	CAMERA_INCLUDE_CAMERA_H
#define	CAMERA_INCLUDE_CAMERA_H

#include <linux/videodev2.h>

struct format_frame {
	__u32 width;
	__u32 height;
	__u32 pixelformat;
};

struct config_camera {
	struct format_frame format;
};

struct frame_buffer {
	unsigned char *buf;
	int size;
};

struct config_camera_overlay {
	struct format_frame format;
	struct frame_buffer frame_buf;
};

//int camera_get_one_frame(unsigned char *ppFrameAddr, long *FrameSize);
void * camera_get_one_frame(struct config_camera *camera, long *FrameSize);
void * camera_get_one_frame_noneblock(struct config_camera *camera, long *FrameSize);
void *camera_get_one_frame_phy_address(struct config_camera *camera);
int camera_get_one_frame_complete(struct config_camera *camera);
int camera_exit(struct config_camera *camera);
int camera_init(struct config_camera *camera);


int camera_overlay_get_one_frame(struct config_camera_overlay *camera);
int camera_overlay_exit(struct config_camera_overlay *camera);
int camera_overlay_init(struct config_camera_overlay *camera);

#endif	/* CAMERA_INCLUDE_CAMERA_H */
