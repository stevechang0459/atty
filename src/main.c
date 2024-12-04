#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <time.h>

#include "global.h"
#include "types.h"
#include "list.h"
#include "serial_port.h"

#define ATTY_VERSION		"1.0.0"

#define CONFIG_NON_BLOCK_MODE	(1)
#define CONFIG_MAIN_DEBUG	(0)

#define DEFAULT_SERIAL_PORT	"/dev/ttyUSB0"
#define DEFAULT_BAUD_RATE	115200
#define DATA_IN_BUF_SIZE	8192
#define DATA_OUT_BUF_SIZE	512
#define NFDS			2
#define POLL_TIMEOUT_MS		-1
#define NON_BLOCK_DELAY_MS	100000
#define FILE_NAME_MAX		256
#define DEV_NAME_MAX		256

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
			DEFAULT_SERIAL_PORT, strerror(errno), errno);
	}
}

int main(int argc, char *argv[])
{
	int ret;
	FILE *fp = NULL;
	char *end;
	size_t len;
	ssize_t bytes_read, bytes_written;
	char data_in[DATA_IN_BUF_SIZE];
	char data_out[DATA_OUT_BUF_SIZE];
	char file_name[FILE_NAME_MAX];
	char dev_name[DEV_NAME_MAX];
	memcpy(dev_name, DEFAULT_SERIAL_PORT, sizeof(DEFAULT_SERIAL_PORT));

	struct serial_cfg cfg = {
		.dev_name 	= dev_name,
		.baud_rate 	= DEFAULT_BAUD_RATE,
		.help 		= 0,
		.output_file 	= 0,
		.save 		= 0,
		.icrnl 		= 0,
		.onlret 	= 0,
		.onlcr 		= 0
	};

	int opt;
	/* handle (optional) flags first */
	while ((opt = getopt(argc, argv, "cd:hlno:r:sv")) != -1) {
		#if (CONFIG_MAIN_DEBUG)
		printf("opt: %c\n", (char)opt);
		#endif
		switch (opt) {
		case 'c':
			// ICRNL: Map CR to NL on input (for MAP1602)
			cfg.icrnl = 1;
			break;
		case 'd':
			cfg.dev_name = optarg;
			memcpy(cfg.dev_name, optarg, strlen(optarg));
			printf("dev_name[%ld]: %s\n", strlen(cfg.dev_name),
				cfg.dev_name);
			break;
		case 'h':
			cfg.help = 1;
			break;
		case 'l':
			// ONLCR: Map NL to CR-NL on output (for MAP1602)
			cfg.onlcr = 1;
			break;
		case 'n':
			// ONLRET: NL performs CR function (for Raspberry Pi 5)
			cfg.onlret = 1;
			break;
		case 'o':
			cfg.output_file = 1;
			cfg.save = 1;
			len = strlen(optarg);
			memcpy(file_name, optarg, len);
			file_name[len] = '\0';
			printf("output_file[%ld]: %s\n", len, file_name);
			break;
		case 'r':
			cfg.baud_rate = strtol(optarg, &end, 0);
			if (cfg.baud_rate < 0) {
				fprintf(stderr, "Invalid baud rate %s: %s (%d)\n",
					optarg, strerror(errno), errno);
				exit(EXIT_FAILURE);
			}

			printf("baud_rate[%ld]: %s, %ld\n",
				strlen(optarg), optarg, cfg.baud_rate);
			break;
		case 's':
			if (cfg.output_file)
				break;
			cfg.save = 1;
			time_t t = time(NULL);
			struct tm *local = localtime(&t);
			len = sprintf(file_name, "atty-%04d%02d%02d-%02d%02d%02d.txt",
				local->tm_year + 1900,
				local->tm_mon,
				local->tm_mday,
				local->tm_hour,
				local->tm_min,
				local->tm_sec);
			file_name[len] = '\0';
			printf("output_file[%ld]: %s\n", len, file_name);
			break;
		case 'v':
			printf("atty version %s\n", ATTY_VERSION);
		case '?':
			exit(EXIT_SUCCESS);
			break;
		}
	}

	#if (CONFIG_NON_BLOCK_MODE)
	int fd = open(cfg.dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
	#else
	int fd = open(cfg.dev_name, O_RDWR | O_NOCTTY);
	#endif
	if (fd < 0) {
		fprintf(stderr, "Failed to open serial port %s: %s (%d)\n",
			cfg.dev_name, strerror(errno), errno);
		return EXIT_FAILURE;
	}

	signal(SIGINT, sigint_handler);

	ret = serial_port_init(fd, &cfg);
	if (ret < 0)
		goto exit;

	printf("Serial port %s opened successfully at %ld baud.\n",
		cfg.dev_name, cfg.baud_rate);

	if (cfg.output_file || cfg.save) {
		printf("Save log to the file '%s'\n", file_name);
		fp = fopen(file_name, "w");
		if (fp < 0) {
			fprintf(stderr, "Failed to open file %s: %s (%d)\n",
				file_name, strerror(errno), errno);
			goto exit;
		}
	}

	fds[0].fd = fd;
	fds[0].events = POLLIN;
	fds[1].fd = STDIN_FILENO;
	fds[1].events = POLLIN;

	clear_screen();

	while (1) {
		ret = poll(fds, NFDS, POLL_TIMEOUT_MS);
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
				if (cfg.save)
					fprintf(fp, "%s", data_in);
				printf("%s", data_in);
				#endif
				fflush(stdout);
			} else if (bytes_read < 0) {
				#if (CONFIG_NON_BLOCK_MODE)
				if (errno == EAGAIN) {
					usleep(NON_BLOCK_DELAY_MS);
				} else {
					fprintf(stderr, "Failed to read from serial port %s: %s (%d)\n",
						DEFAULT_SERIAL_PORT, strerror(errno), errno);
					break;
				}
				#else
				fprintf(stderr, "Failed to read from serial port %s: %s (%d)\n",
					DEFAULT_SERIAL_PORT, strerror(errno), errno);
				break;
				#endif
			}
		}

		if (fds[0].revents & POLLHUP) {
			printf("Serial port %s disconnected\n", DEFAULT_SERIAL_PORT);
			break;
		}

		if (fds[0].revents & POLLERR) {
			fprintf(stderr, "Error on serial port %s: %s (%d)\n",
				DEFAULT_SERIAL_PORT, strerror(errno), errno);
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

			if (strcmp(data_out, "atty\n") == 0) {
				printf("%s", data_out);
				break;
			}

			// // Raspberry Pi 5: ONLRET
			// data_out[strcspn(data_out, "\n")] = '\r';
			// data_out[strlen(data_out)+1] = '\0';

			// // MAP1602: ONLCR
			// data_out[strcspn(data_out, "\n")] = '\r';
			// data_out[strlen(data_out)+1] = '\0';
			// data_out[strlen(data_out)] = '\n';

			bytes_written = write(fd, data_out, strlen(data_out));
			#if (CONFIG_MAIN_DEBUG)
			if (bytes_written > 0)
				printf("bytes_written: %ld\n", bytes_written);
			#endif
			if (bytes_written < 0) {
				fprintf(stderr, "Failed to write data to %s: %s (%d)\n",
					DEFAULT_SERIAL_PORT, strerror(errno), errno);
				break;
			}
		}
	}

exit:
	if (fp) {
		ret = fclose(fp);
		if (ret < 0)
			fprintf(stderr, "Failed to close file %s: %s (%d)\n",
				file_name, strerror(errno), errno);
	}

	ret = close(fd);
	if (ret < 0) {
		fprintf(stderr, "Failed to close fd (%d): %s (%d)\n",
			fd, strerror(errno), errno);
	}

	return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
