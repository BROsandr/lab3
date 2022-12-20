/*******************************************************************************
 * Copyright (c) 2022 Sergey Balabaev (sergei.a.balabaev@gmail.com)                     *
 *                                                                             *
 * The MIT License (MIT):                                                      *
 * Permission is hereby granted, free of charge, to any person obtaining a     *
 * copy of this software and associated documentation files (the "Software"),  *
 * to deal in the Software without restriction, including without limitation   *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell   *
 * copies of the Software, and to permit persons to whom the Software is       *
 * furnished to do so, subject to the following conditions:                    *
 * The above copyright notice and this permission notice shall be included     *
 * in all copies or substantial portions of the Software.                      *
 *                                                                             *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,             *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR       *
 * OTHER DEALINGS IN THE SOFTWARE.                                             *
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void help()
{
	printf("    Use this application for reading from sound sensor\n");
	printf("    execute format: ./sound_detect [-h][-q] \n");
	printf("    return: 'Clap!', when sound detected\n");
	printf("    -h - help\n");
	printf("    -q - quiet\n");
}

int channel_general = STDOUT_FILENO;

void interrupt(int unused) {
  write(channel_general, "b\n", 1);
  exit(0);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, interrupt);
    int quiet = 0;
	if (argc > 1){
		if ((strcmp(argv[1], "-h") == 0)) {
			help();
			return 0;
		} else if ((strcmp(argv[1], "-q") == 0)) {
			quiet = 1;
			int channel_named = -1;
			if( argc > 2 ) {
			  channel_named = open(argv[2], O_WRONLY);
			  if( channel_named == -1 ) {
				fprintf(stderr, "cannot open channel\n");
			  	exit(-1);
			  }
			  channel_general = channel_named;
            }
        }
	}
	if (!quiet)
		printf("\nThe soundsensor application was started\n\n");

	sleep(0.05);
	int prev = 0, next = 0;
	while (1) {
		FILE *fp = popen("sudo sh -c 'cat /sys/module/driver/parameters/value'", "r");
		char buff[256] = { '\0' };
		if (fgets(buff, 256, fp) == NULL) {
			printf("Error %d\n", __LINE__);
		}
		pclose(fp);

		next = atoi(buff);

		fflush(stdout);
		if (prev < next) {
			// fprintf(channel, "Clap!\n");
			struct timespec time;
			clock_gettime(CLOCK_REALTIME, &time);
			char buff[256] = { 0 };
			sprintf(buff, "%ld\n", time.tv_sec);
			write(channel_general, buff, strlen(buff));
			system("sudo sh -c 'echo 0 > /sys/module/driver/parameters/value'");
			// fprintf(f, "%d", 0);
			// printf("%d\n", time.tv_sec); 
			//fflush(stdout);
			// if( extern_chan )
			//	close(channel_general);
			usleep(10000);
		}
		prev = next;
	}
	return 0;
}
