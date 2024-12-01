#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#define BAUD_RATE	B115200

void serial_port_init(int fd)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) != 0) {
		perror("Failed to get serial port attributes");
		exit(EXIT_FAILURE);
	}

	cfsetospeed(&tty, BAUD_RATE);
	cfsetispeed(&tty, BAUD_RATE);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag |= (CREAD | CLOCAL);

	// tty.c_iflag = 0;
	// tty.c_iflag |= IGNCR;
	tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	// tty.c_oflag = 0;
	tty.c_oflag &= ~OPOST;

	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("Failed to set serial port attribute");
		exit(EXIT_FAILURE);
	}
}
