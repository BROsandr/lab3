#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SOUND_SPEED 343

int get_hr(int seconds) {
  return (int)(seconds/3600);
}

int get_min(int seconds) {
  return ((int)(seconds/60))%60;
}

int get_sec(int seconds) {
  return (int)(seconds%60);
}

int main() {
  int iterations = 1;

  int light_data = open("light_data", O_RDONLY);
  int sound_data = open("sound_data", O_RDONLY);

  if( light_data == -1 || sound_data == -1 ) {
    printf("error");
    return -1;
  }

  char light_time[256] = { '\0' };
  char sound_time[256] = { '\0' };

  char hms_light[256] = { '\0' };
  char hms_sound[256] = { '\0' };

  read(sound_data, sound_time, 256);

  enum{
    LIGHT,
    SOUND,
    NONE
  } who_exited = NONE;

  while(1) {
    read(light_data, light_time, 256);
    if(!strcmp(light_time, "a\n")) {
      printf("light exits\n");
      who_exited = LIGHT;
      break;
    }
    FILE *fp = popen("date -u +%H:%M:%S", "r");
    if (fgets(hms_light, 256, fp) != NULL) {
      printf("время измерения вспышки № %d: %s\n", iterations, hms_light);
    }
    pclose(fp);

    read(sound_data, sound_time, 256);
    // for(int i = 0; i < strlen(sound_time); ++i) {
    //   printf("%c\n", sound_time[i]);
    // }
    // printf("%x", atoi(sound_time));
    if(!strcmp(sound_time, "b\n")) {
      printf("sound exits\n");
      who_exited = SOUND;
      break;
    }
    fp = popen("date -u +%H:%M:%S", "r");
    if (fgets(hms_sound, 256, fp) != NULL) {
      printf("время измерения хлопка № %d: %s\n", iterations, hms_sound);
    }
    pclose(fp);


    int distance_seconds = atoi(sound_time) - atoi(light_time);
    int distance_meters  = distance_seconds * SOUND_SPEED;

    printf("расстояние до грозы № %d: %d\n", iterations, distance_meters);
    fflush(stdout);
    iterations++;
  }
  while(1) {
    if(who_exited == SOUND) {
      read(light_data, light_time, 256);
      if(strcmp(light_time, "a\n")) continue;
      printf("light exited\n");
      break;
    } else {
      read(sound_data, sound_time, 256);
      if(strcmp(sound_time, "b\n")) continue;
      printf("sound exited\n");
      break;
    }
  }

  close(light_data);
  close(sound_data);

  execv("sudo ./k.sh", NULL);

  return 0;
}
