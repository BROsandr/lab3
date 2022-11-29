#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
  int light_data = open("light_data", O_RDONLY);
  int sound_data = open("sound_data", O_RDONLY);

  if( light_data == -1 || sound_data == -1 ) {
    printf("error");
    return -1;
  }

  char light[256] = { '\0' };
  char sound[256] = { '\0' };

  read(sound_data, sound, 256);
  read(sound_data, sound, 256);
  read(light_data, light, 256);

  printf(light);
  printf(sound);

  close(light_data);
  close(sound_data);

  system("sudo ./k.sh");

  return 0;
}
