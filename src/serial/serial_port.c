#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#include "serial_port.h"

#define BAUD_RATE	B115200

int serial_select_baud_rate(long b)
{
	switch (b) {
	case 56700:
		return B115200;
	case 115200:
		return B115200;
	default:
		fprintf(stderr, "Invalid baud rate %ld\n", b);
		return -1;
	}
}

int serial_port_init(int fd, struct serial_cfg *cfg)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) != 0) {
		perror("Failed to get serial port attributes");
		return -1;
	}

	int baud_rate = serial_select_baud_rate(cfg->baud_rate);
	if (baud_rate < 0) {
		fprintf(stderr, "Invalid baud rate %ld\n", cfg->baud_rate);
		return -1;
	}

	cfsetispeed(&tty, BAUD_RATE);
	cfsetospeed(&tty, BAUD_RATE);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag &= ~(CRTSCTS | PARENB | CSTOPB);
	tty.c_cflag |= (CLOCAL | HUPCL | CREAD);

	// tty.c_lflag = 0;
	tty.c_lflag &= ~(ECHOE | ECHO | ICANON | ISIG);
	tty.c_lflag &= ~(IEXTEN | ECHOKE | ECHOCTL | ECHOK);

	// tty.c_iflag = 0;
	tty.c_iflag &= ~(IXOFF | IXANY | IXON | ICRNL);
	tty.c_iflag |= IGNBRK;

	// ICRNL: Map CR to NL on input (for MAP1602)
	if (cfg->icrnl)
		tty.c_iflag |= ICRNL;

	// tty.c_oflag = 0;
	tty.c_oflag &= ~(ONLRET | ONLCR | OPOST);

	// ONLRET: NL performs CR function (for Raspberry Pi 5)
	if (cfg->onlret)
		tty.c_oflag |= (ONLRET | OPOST);

	// ONLCR: Map NL to CR-NL on output (for MAP1602)
	if (cfg->onlcr)
		tty.c_oflag |= (ONLCR | OPOST);

	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("Failed to set serial port attribute");
		exit(EXIT_FAILURE);
	}

	return 0;
}
