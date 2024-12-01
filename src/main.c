#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

#include "global.h"
#include "types.h"
#include "list.h"

#define CONFIG_NON_BLOCK_MODE	(1)
#define CONFIG_MAIN_DEBUG	(0)

#define SERIAL_PORT		"/dev/ttyUSB0"
#define DATA_IN_BUF_SIZE	8192
#define DATA_OUT_BUF_SIZE	512
#define NFDS			2
#define POLL_TIMEOUT		-1

extern void serial_port_init(int fd);

struct pollfd fds[NFDS];

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void sigint_handler(int sig) {
	#if (CONFIG_MAIN_DEBUG)
	printf("\nsigint_handler: %d\n", sig);
	#endif
	char etx = 3;
	ssize_t bytes_written = write(fds[0].fd, &etx, sizeof(etx));
	#if (CONFIG_MAIN_DEBUG)
	if (bytes_written > 0)
		printf("bytes_written: %ld\n", bytes_written);
	#endif
	if (bytes_written < 0) {
		fprintf(stderr, "Failed to write data to %s: %s (%d)\n",
			SERIAL_PORT, strerror(errno), errno);
	}
}

int main(int argc, char *argv[])
{
	int ret;
	ssize_t bytes_read, bytes_written;
	char data_in[DATA_IN_BUF_SIZE];
	char data_out[DATA_OUT_BUF_SIZE];

	#if (CONFIG_NON_BLOCK_MODE)
	int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	#else
	int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY);
	#endif
	if (fd < 0) {
		fprintf(stderr, "Failed to open serial port %s: %s (%d)\n",
			SERIAL_PORT, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	signal(SIGINT, sigint_handler);

	serial_port_init(fd);
	printf("Serial port %s opened successfully at 115200 baud.\n", SERIAL_PORT);

	fds[0].fd = fd;
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;

	clear_screen();

	while (1) {
		ret = poll(fds, NFDS, POLL_TIMEOUT);
		if (ret < 0) {
			if (errno == EINTR) {
				#if (CONFIG_MAIN_DEBUG)
				printf("Poll interrupted by signal, resuming...\n");
				#endif
				continue;
			}
			fprintf(stderr, "Error during poll: %s (%d)\n",
				strerror(errno), errno);
			break;
		}

		if (fds[0].revents & POLLIN) {
			bytes_read = read(fd, data_in, sizeof(data_in)-1);
			if (bytes_read > 0) {
				#if (CONFIG_MAIN_DEBUG)
				printf("bytes_read: %ld\n", bytes_read);
				#else
				data_in[bytes_read] = '\0';
				printf("%s", data_in);
				#endif
				fflush(stdout);
			} else if (bytes_read < 0) {
				#if (CONFIG_NON_BLOCK_MODE)
				if (errno == EAGAIN) {
					usleep(100000);
				} else {
					fprintf(stderr, "Failed to read from serial port %s: %s (%d)\n",
						SERIAL_PORT, strerror(errno), errno);
					break;
				}
				#else
				fprintf(stderr, "Failed to read from serial port %s: %s (%d)\n",
					SERIAL_PORT, strerror(errno), errno);
				break;
				#endif
			}
		}

		if (fds[0].revents & POLLHUP) {
			printf("Serial port %s disconnected\n", SERIAL_PORT);
			break;
		}

		if (fds[0].revents & POLLERR) {
			fprintf(stderr, "Error on serial port %s: %s (%d)\n",
				SERIAL_PORT, strerror(errno), errno);
			break;
		}

		if (fds[1].revents & POLLIN) {
			char *s = fgets(data_out, sizeof(data_out), stdin);
			if (s == NULL) {
				if (feof(stdin))
					printf("End of file\n");
				if (ferror(stdin))
					fprintf(stderr, "Failed to read data from stdin: %s (%d)\n",
						strerror(errno), errno);
				break;
			}

			#if (CONFIG_MAIN_DEBUG)
			for (int i = 0; i < strlen(data_out); i++) {
				printf("%02x ", data_out[i]);
				if ((i+1) % 8 == 0)
					printf("\n");
				else if ((i+1) == strlen(data_out))
					printf("\n");
			}
			printf("strlen : %ld\n", strlen(data_out));
			printf("strcspn: %ld\n", strcspn(data_out, "\n"));
			#endif

			if (strcmp(data_out, "exit\n") == 0) {
				printf("%s", data_out);
				break;
			}

			data_out[strcspn(data_out, "\n")] = '\r';
			data_out[strlen(data_out)] = '\0';

			bytes_written = write(fd, data_out, strlen(data_out));
			#if (CONFIG_MAIN_DEBUG)
			if (bytes_written > 0)
				printf("bytes_written: %ld\n", bytes_written);
			#endif
			if (bytes_written < 0) {
				fprintf(stderr, "Failed to write data to %s: %s (%d)\n",
					SERIAL_PORT, strerror(errno), errno);
				break;
			}
		}
	}

	ret = close(fd);
	if (ret < 0) {
		fprintf(stderr, "Failed to close fd (%d): %s (%d)\n",
			fd, strerror(errno), errno);
	}

	return EXIT_SUCCESS;
}
