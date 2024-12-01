#include <stdio.h>
#include <stdlib.h>
#include <termios.h>

#define BAUD_RATE			B115200

void serial_port_init(int fd)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) != 0) {
		perror("Failed to get serial port attributes");
		exit(EXIT_FAILURE);
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
	tty.c_iflag |= IGNBRK;
	tty.c_iflag &= ~(IXOFF | IXANY | IXON | ICRNL);

	// tty.c_oflag = 0;
	tty.c_oflag &= ~(ONLCR | OPOST);

	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		perror("Failed to set serial port attribute");
		exit(EXIT_FAILURE);
	}
}
