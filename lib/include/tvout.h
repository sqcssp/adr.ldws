#ifndef	INCLUDE_TVOUT_H
#define	INCLUDE_TVOUT_H

struct config_tvout {
	int	width;
	int	height;
	__u32	format;
	long	addr_phy;
};

int tvout_open(struct config_tvout *tvout);
int tvout_close(struct config_tvout *tvout);
int tvout_exe(struct config_tvout *tvout);

#endif	/* INCLUDE_TVOUT_H */
