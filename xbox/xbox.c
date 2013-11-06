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

#define INPUT_PATH "/dev/input"
#define EVENT_DEV  "event"
#define XBOX_NAME "Xbox"

static int is_event(const struct dirent *dir) {
    return strncmp(EVENT_DEV, dir->d_name, 5) == 0;
}

char** find_controllers() {
    // Variables for input identification
    int i = 0;
    int num_devices = 0;
    struct dirent **names;
    char fname[64];
    int fd = -1;
    char name[256] = "???";

    // Variables for return value
    int num_controllers = 0;
    char **controllers = NULL;
    char **tmp_controllers = NULL;

    // Find all "event" devices
    num_devices = scandir(INPUT_PATH, &names, is_event, alphasort);
    for(i = 0; i < num_devices; i++) {
        // Construct the filename
        snprintf(fname, sizeof(fname), "%s/%s", INPUT_PATH, names[i]->d_name);

        // Open the event file
        fd = open(fname, O_RDONLY);
        if (fd < 0) {
            // Open failed, keep trying the others
            continue;
        }

        // Get the device name
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        // Is it an Xbox controller?
        if(strstr(name, XBOX_NAME)) {
            // Found a controller
            num_controllers++;

            // Is it the first we've found?
            if (controllers == NULL) {
                // Allocate a new return array
                controllers = (char**)malloc(sizeof(char*));
            } else {
                // Increase the size of the return array
                tmp_controllers = (char**)realloc(controllers, sizeof(char*) * num_controllers);
                if(tmp_controllers == NULL) {
                    fprintf(stderr, "Failed to reallocate memory.\n");
                    return NULL;
                }
                // Only update the return array if the realloc was successful
                controllers = tmp_controllers;
            }

            // Copy the event filename into the return array
            controllers[num_controllers - 1] = (char*)malloc(sizeof(char) * 256);
            strncpy(controllers[num_controllers - 1], fname, 256);
        }

        // Close the file and free the directory entry
        close(fd);
        free(names[i]);
    }

    // Add a final NULL value to the return array to signal the end
    tmp_controllers = realloc(controllers, sizeof(char*) * (num_controllers + 1));
    if(tmp_controllers == NULL) {
        fprintf(stderr, "Failed to reallocate memory.\n");
        return NULL;
    }
    controllers = tmp_controllers;

    controllers[num_controllers] = NULL;
    return controllers;
}

int main() {
    char **xbox_inputs = find_controllers();
    while(xbox_inputs) {
        printf("%s\n", *xbox_inputs);
        free(*xbox_inputs);
        xbox_inputs++;
    }
    return 0;
}
