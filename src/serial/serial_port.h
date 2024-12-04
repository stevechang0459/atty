#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include "types.h"

struct serial_cfg
{
	char *dev_name;
	long baud_rate;
	bool help;
	bool output_file;
	bool save;
	bool icrnl;
	bool onlret;
	bool onlcr;
};

extern int serial_port_init(int fd, struct serial_cfg *cfg);

#endif