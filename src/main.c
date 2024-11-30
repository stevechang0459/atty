#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "types.h"
#include "list.h"

#define CONFIG_NON_BLOCK_MODE		(0)
#define CONFIG_DEBUG			(0)

#define SERIAL_PORT	"/dev/ttyUSB0"
#define BUFFER_SIZE	4096

extern void serial_port_init(int fd);

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
	#if (CONFIG_NON_BLOCK_MODE)
	int fd = open(SERIAL_PORT, O_RDONLY | O_NOCTTY | O_NDELAY);
	#else
	int fd = open(SERIAL_PORT, O_RDONLY | O_NOCTTY);
	#endif
	if (fd < 0) {
		perror("Failed to open serial port");
		return EXIT_FAILURE;
	}

	serial_port_init(fd);
	printf("Serial port %s opened successfully at 115200 baud.\n", SERIAL_PORT);

	clear_screen();

	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));
	while (1) {
		int data_len = read(fd, buffer, sizeof(buffer)-1);
		if (data_len > 0) {
			#if (CONFIG_DEBUG)
			printf("data_len: %d\n", data_len);
			#else
			printf("%s", buffer);
			#endif
			memset(buffer, 0, data_len);
		} else if (data_len < 0) {
			#if (CONFIG_NON_BLOCK_MODE)
			if (errno == EAGAIN) {
				// usleep(100000);
				usleep(2000000);
			} else {
				fprintf(stderr, "Failed to read from serial port: %s (%d)\n",
					strerror(errno), errno);
				break;
			}
			#else
			fprintf(stderr, "Failed to read from serial port: %s (%d)\n",
				strerror(errno), errno);
			break;
			#endif
		}
	}

	close(fd);

	return EXIT_SUCCESS;
}
