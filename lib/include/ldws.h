#ifndef	INCLUDE_LDWS_H
#define	INCLUDE_LDWS_H

#define STATUS_OK	0
#define STATUS_FAIL	1
#define START_SPEED_KMH	60	/* The limit of starting speed */

enum MSG {	/* return driving message */
	WARNING_DRIVING_SAFE = 0,
	WARNING_DRIVING_LEFT,
	WARNING_DRIVING_RIGHT,
};

enum LIGHT {	/* control light signal */
	LIGHT_SIGNAL_OFF = 0,
	LIGHT_SIGNAL_LEFT,
	LIGHT_SIGNAL_RIGHT,
};

struct car_parm_setting {	/* input car parameter */
	unsigned char *pIn_addr;	/* input image address */
	unsigned char *pOut_addr;	/* output image address */
	int init_flag;				/* first setup */
	int carbody_width;			/* unit: cm */
	int camera_high_degree;		/* unit: cm */
	int gps_speed;				/* unit: KM/h */
	enum LIGHT light_signal;
	int img_width;				/* width of image */
	int img_height;				/* height of image */
	enum MSG car_event;			/* the message of driving event */
};

void *set_car_parm(struct car_parm_setting *pParm);

#endif	/* INCLUDE_LDWS_H */
