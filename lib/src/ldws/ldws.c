#include <unistd.h>
#include <stdlib.h>

#include "ldws.h"


void *set_car_parm(struct car_parm_setting *pParm)
{
	unsigned char	*pBuf;
	int width = pParm->img_width;
	int height = pParm->img_height;


	if(pParm->gps_speed <= START_SPEED_KMH)	{
		pParm->car_event = -1;
		return (pParm->pOut_addr = pParm->pIn_addr);
	}

	pBuf = pParm->pIn_addr;

	if(pParm->init_flag == 1) {
		pParm->init_flag = 0;
		pParm->carbody_width = pParm->carbody_width;
		pParm->camera_high_degree = pParm->camera_high_degree;
	}

//	usleep(200000);

	pParm->car_event = rand()%3;

	pParm->pOut_addr = pParm->pIn_addr;

	return pBuf;
}
