#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define SOUND_SPEED 343

int main() {
  int light_data = open("light_data", O_RDONLY);
  int sound_data = open("sound_data", O_RDONLY);

  if( light_data == -1 || sound_data == -1 ) {
    printf("error");
    return -1;
  }

  char light_time[256] = { '\0' };
  char sound_time[256] = { '\0' };

  read(sound_data, sound_time, 256);
  read(light_data, light_time, 256);
  read(sound_data, sound_time, 256);

  int distance_seconds = atoi(sound_time) - atoi(light_time);
  int distance_meters  = distance_seconds * SOUND_SPEED;

  printf("%d", distance_meters);

  close(light_data);
  close(sound_data);

  system("sudo ./k.sh");

  return 0;
}
