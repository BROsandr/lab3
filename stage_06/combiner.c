#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>

#define SOUND_SPEED 343

int iterations = 1;

int get_hr(int seconds) {
  return (int)(seconds/3600);
}

int get_min(int seconds) {
  return ((int)(seconds/60))%60;
}

int get_sec(int seconds) {
  return (int)(seconds%60);
}

pthread_mutex_t light_ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sound_ready_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t who_exited_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t iterations_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t light_time_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sound_time_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t history_mutex = PTHREAD_MUTEX_INITIALIZER;

char light_time[256] = { '\0' };
char sound_time[256] = { '\0' };

enum{
  LIGHT,
  SOUND,
  NONE
} who_exited = NONE;

char sound_ready = 0;
char light_ready = 0;

char history[3][256];

void* light_detect(void *thread_data) {
  int light_data = open("light_data", O_RDONLY);
  if( light_data == -1 ) {
    printf("error %d\n", __LINE__);
    return NULL;
  }
  while(1) {
    char light_time_local[256] = { '\0' };
    int read_res = read(light_data, light_time_local, 256);
    if(read_res == -1) {
      printf("read error %d\n", __LINE__);
      exit(-1);
    } else if(read_res == 0) {
      printf("light_data closed %d\n", __LINE__);
      exit(-1);
    };
    fflush(stdout);
    if(!strncmp(light_time_local, "a", 1)) {
      printf("light exits\n");

      pthread_mutex_lock(&who_exited_mutex);
      who_exited = LIGHT;
      pthread_mutex_unlock(&who_exited_mutex);

      return NULL;
    }
    fflush(stdout);

    time_t epoch_time = time(0);
    struct tm *calendar_time = localtime(&epoch_time);

    pthread_mutex_lock(&iterations_mutex);
    int iterations_local = iterations;
    pthread_mutex_unlock(&iterations_mutex);

    char history_local[256] = { '\0' };
    sprintf(history_local, "время измерения вспышки № %d: %d:%d:%d\n", iterations_local, calendar_time->tm_hour, calendar_time->tm_min, calendar_time->tm_sec);


    pthread_mutex_lock(&history_mutex);
    strcpy(history[0], history_local);
    pthread_mutex_unlock(&history_mutex);

    printf("%s", history_local);
    fflush(stdout);

    pthread_mutex_lock(&light_time_mutex);
    strcpy(light_time, light_time_local);
    pthread_mutex_unlock(&light_time_mutex);

    pthread_mutex_lock(&light_ready_mutex);
    light_ready = 1;
    pthread_mutex_unlock(&light_ready_mutex);
  }
}

void* sound_detect(void *thread_data) {
  int sound_data = open("sound_data", O_RDONLY);
  if( sound_data == -1 ) {
    printf("error %d\n", __LINE__);
    return NULL;
  }

  char sound_time_local[256] = { '\0' };

  read(sound_data, sound_time_local, 256);

  while(1) {
    int read_res = read(sound_data, sound_time_local, 256);

    if(read_res == -1) {
      printf("read error %d\n", __LINE__);
      exit(-1);
    } else if(read_res == 0) {
      printf("sound_data closed %d\n", __LINE__);
      exit(-1);
    }
    fflush(stdout);
    if(!strncmp(sound_time_local, "b", 1)) {
      printf("sound exits\n");

      pthread_mutex_lock(&who_exited_mutex);
      who_exited = SOUND;
      pthread_mutex_unlock(&who_exited_mutex);

      return NULL;
    }
    fflush(stdout);

    time_t epoch_time = time(0);
    struct tm *calendar_time = localtime(&epoch_time);

    pthread_mutex_lock(&iterations_mutex);
    int iterations_local = iterations;
    pthread_mutex_unlock(&iterations_mutex);

    char history_local[256] = { '\0' };
    sprintf(history_local, "время измерения хлопка № %d: %d:%d:%d\n", iterations_local, calendar_time->tm_hour, calendar_time->tm_min, calendar_time->tm_sec);

    printf("%s", history_local);
    fflush(stdout);

    pthread_mutex_lock(&history_mutex);
    strcpy(history[1], history_local);
    pthread_mutex_unlock(&history_mutex);

    pthread_mutex_lock(&sound_time_mutex);
    strcpy(sound_time, sound_time_local);
    pthread_mutex_unlock(&sound_time_mutex);

    pthread_mutex_lock(&sound_ready_mutex);
    sound_ready = 1;
    pthread_mutex_unlock(&sound_ready_mutex);
  }
}

void* distance_calculate(void *thread_data) {
  while(1) {
    char ready = 0;

    pthread_mutex_lock(&sound_ready_mutex);
    pthread_mutex_lock(&light_ready_mutex);
    ready = light_ready && sound_ready;
    pthread_mutex_unlock(&light_ready_mutex);
    pthread_mutex_unlock(&sound_ready_mutex);

    if(ready) {
      pthread_mutex_lock(&sound_ready_mutex);
      pthread_mutex_lock(&light_ready_mutex);
      light_ready = 0;
      sound_ready = 0;
      pthread_mutex_unlock(&light_ready_mutex);
      pthread_mutex_unlock(&sound_ready_mutex);

      pthread_mutex_lock(&light_time_mutex);
      pthread_mutex_lock(&sound_time_mutex);
      int distance_seconds = atoi(sound_time) - atoi(light_time);
      pthread_mutex_unlock(&light_time_mutex);
      pthread_mutex_unlock(&sound_time_mutex);

      int distance_meters  = distance_seconds * SOUND_SPEED;
      fflush(stdout);

      char history_local[256] = { '\0' };
      pthread_mutex_lock(&iterations_mutex);
      sprintf(history_local, "расстояние до грозы № %d: %d\n", iterations++, distance_meters);
      pthread_mutex_unlock(&iterations_mutex);

      printf("%s", history_local);
      fflush(stdout);

      pthread_mutex_lock(&history_mutex);
      strcpy(history[2], history_local);
      pthread_mutex_unlock(&history_mutex);

      fflush(stdout);
    }
  }
}

pid_t get_pid(const char *name) {
  char command[256] = { '\0' };
  sprintf(command, "pidof -s %s", name);
  char buf[512] = { '\0' };
  FILE *cmd_pipe = popen(command, "r");

  fgets(buf, 512, cmd_pipe);
  pid_t pid = strtoul(buf, NULL, 10);

  pclose( cmd_pipe );

  return pid;
}

void* user_input(void *thread_data) {
  while(1) {
    char input[256] = {'\0'};
    scanf("%s", input);
    if(!strncmp(input, "stop", 4)){
      printf("stopping\n");
      pid_t light_detect = get_pid("light_detect");
      pid_t sound_detect = get_pid("sound_detect");
      kill(light_detect, SIGINT);
      kill(sound_detect, SIGINT);
//      popen("pkill -SIGINT light_detect", "r");
//      system("pkill -SIGINT light_detect");
//      system("pkill -SIGINT light_detect");
    }
    if(!strncmp(input, "restart", 7)){
      printf("restarted\n");
      pthread_mutex_lock(&iterations_mutex);
      iterations = 1;
      pthread_mutex_unlock(&iterations_mutex);
    }
    if(!strncmp(input, "last", 4)){
      printf("printing history\n");
      pthread_mutex_lock(&history_mutex);
      printf("%s", history[0]);
      printf("%s", history[1]);
      printf("%s", history[2]);
      pthread_mutex_unlock(&history_mutex);
    }
  }
}

int main() {
  pthread_t thread_sound;
  if (pthread_create(&thread_sound, NULL, sound_detect, NULL) != 0) {
    fprintf(stderr, "error: pthread_create was failed %d\n", __LINE__);
    exit(-1);
  }

  pthread_t thread_light;
  if (pthread_create(&thread_light, NULL, light_detect, NULL) != 0) {
    fprintf(stderr, "error: pthread_create was failed %d\n", __LINE__);
    exit(-1);
  }

  pthread_t thread_distance;
  if (pthread_create(&thread_distance, NULL, distance_calculate, NULL) != 0) {
    fprintf(stderr, "error: pthread_create was failed %d\n", __LINE__);
    exit(-1);
  }

  pthread_t thread_input;
  if (pthread_create(&thread_input, NULL, user_input, NULL) != 0) {
    fprintf(stderr, "error: pthread_create was failed %d\n", __LINE__);
    exit(-1);
  }

  pthread_detach(thread_distance);
  pthread_join(thread_sound, NULL);
  pthread_join(thread_light, NULL);
  pthread_detach(thread_input);

  // while(1) {
  //   if(who_exited == SOUND) {
  //     if(read(light_data, light_time, 256) == -1) {
  //       printf("%s", "read error\n");
  //       exit(0);
  //     }
  //     if(!strcmp(light_time, "a\n")) continue;
  //     printf("light exited\n");
  //     break;
  //   } else {
  //     if(read(sound_data, sound_time, 256) == -1) {
  //       printf("%s", "read error\n");
  //       exit(0);
  //     }
  //     if(!strcmp(sound_time, "b\n")) continue;
  //     printf("sound exited\n");
  //     break;
  //   }
  // }

  return 0;
}
