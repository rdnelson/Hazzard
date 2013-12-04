#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "xbox.h"

#define INPUT_PATH "/dev/input"
#define EVENT_DEV  "event"
#define XBOX_NAME "Xbox"
#define MAX_PLAYERS 4

#define INPUT(a, r) { \
    if((r = a) == -1) { \
        if(errno != EAGAIN) { \
            fprintf(stderr, "Error: call to `%s` on line %d failed with error %s (%d).\n", #a, __LINE__, strerror(errno), errno); \
            return retval; \
        } \
    } \
}

#define INPUT_THREAD(a, r) { \
    if((r = a) == -1) { \
        if(errno != EAGAIN) { \
            fprintf(stderr, "Error: call to `%s` on line %d failed with error %s (%d).\n", #a, __LINE__, strerror(errno), errno); \
            pthread_exit(NULL); \
        } \
    } \
}

#define SYSCALL(a, r) { \
    if((r = a) == -1) { \
        fprintf(stderr, "Error: call to `%s` on line %d failed with error %s (%d).\n", #a, __LINE__, strerror(errno), errno); \
        return; \
    } \
}

#define SYSCALL_THREAD(a, r) { \
    if((r = a) == -1) { \
        fprintf(stderr, "Error: call to `%s` on line %d failed with error %s (%d).\n", #a, __LINE__, strerror(errno), errno); \
        pthread_exit(NULL); \
    } \
}

typedef void (*read_callback)(struct player_event);

struct callback_info {
    int fd;
    read_callback cb;
    unsigned int player;
};

int current_player;
int num_players;
int num_controllers;
int player_fds[MAX_PLAYERS];
pthread_t threads[MAX_PLAYERS];

static int is_event(const struct dirent *dir) {
    return strncmp(EVENT_DEV, dir->d_name, 5) == 0;
}

int open_controllers() {
    // Variables for input identification
    int i = 0;
    int num_devices = 0;
    struct dirent **names;
    char fname[64];
    int fd = -1;
    char name[256] = "???";

    // Close any existing controller fds and reset the player count
    for(i = 0; i < MAX_PLAYERS; i++) {
        close(player_fds[i]);
    }
    num_players = 0;
    num_controllers = 0;

    // Find all "event" devices
    num_devices = scandir(INPUT_PATH, &names, is_event, alphasort);
    for(i = 0; i < num_devices; i++) {
        // Construct the filename
        snprintf(fname, sizeof(fname), "%s/%s", INPUT_PATH, names[i]->d_name);

        // Open the event file
        fd = open(fname, O_RDWR | O_NONBLOCK);
        if (fd < 0) {
            // Open failed, keep trying the others
            continue;
        }

        // Get the device name
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        // Is it an Xbox controller?
        if(strstr(name, XBOX_NAME)) {
            // Found a controller
            player_fds[num_controllers] = fd;
            num_controllers++;
        } else {
            // Close the file only if it isn't a controller
            close(fd);
        }
        // Free the directory entry
        free(names[i]);
    }

    return num_controllers;
}

void *thread_read(void* void_info) {
    struct callback_info* info = (struct callback_info*)void_info;

    int flags;
    int read_ret;
    struct player_event retval;
    struct input_event inp;

    SYSCALL_THREAD(fcntl(info->fd, F_GETFL, 0), flags);

    flags &= ~O_NONBLOCK;

    SYSCALL_THREAD(fcntl(info->fd, F_SETFL, flags), flags);

    while(1) {
        INPUT_THREAD(read(info->fd, &inp, sizeof(inp)), read_ret);
        if(read_ret == sizeof(inp) && inp.type != 0) {
            // Got some input
            retval.player = info->player;
            retval.event = inp.code;
            retval.data = inp.value;
            info->cb(retval);
        }
    }
}

void add_callback(unsigned int player, read_callback callback) {
    if (player >= MAX_PLAYERS)
        return;

    if (callback != NULL) {

        struct callback_info* info = (struct callback_info*)malloc(sizeof(struct callback_info));
        info->fd = player_fds[player];
        info->cb = callback;
        info->player = player;

        int retval;
        SYSCALL(pthread_create(threads + player, NULL, thread_read, (void*)info), retval);
        player_fds[player] = -2;
    }

}

struct player_event get_event() {
    struct input_event inp;
    struct player_event retval;

    retval.player = -1;
    retval.data = -1;

    // Default to error state
    retval.event = -2;

    int read_ret;
    int end_player = (current_player + MAX_PLAYERS - 1) % MAX_PLAYERS;

    for(; current_player != end_player; current_player = (current_player + 1) % MAX_PLAYERS) {

        // Check for installed callback
        if (player_fds[current_player] == -2) {
            continue;
        }

        INPUT(read(player_fds[current_player], &inp, sizeof(inp)), read_ret);
        if(read_ret == sizeof(inp) && inp.type != 0) {
            // Got some input
            retval.player = current_player;
            retval.event = inp.code;
            retval.data = inp.value;
            current_player = (current_player + 1) % MAX_PLAYERS;
            return retval;
        }
    }

    // No error, set event to no packet read.
    retval.event = -1;
    return retval;
}
