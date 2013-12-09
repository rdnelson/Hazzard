#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "xbox.h"

#define MSECS(a) (a.tv_sec * 1000 + a.tv_usec / 1000)

int main() {
    // Open the controllers
    int xbox_inputs = open_controllers();

    // Only useful if controllers are found
    if(xbox_inputs == 0) {
        printf("Error: No XBox controllers found. Try running as root.");
        return 1;
    }
    printf("Found %d controllers.\n", xbox_inputs);

    struct player_event e;
    struct timeval tvb;
    struct timeval tve;
    int msecs = 0;
    int num_events = 0;

    // Loop forever
    while(1) {

        // Get beginning of current second
        if(msecs == 0)
            gettimeofday(&tvb, NULL);

        e = get_event();

        // Is the event valid?
        if(e.player != -1)
            num_events++;

        // Quit if B is pressed
        if(e.event == BUTTON_B && e.data == 1) {
            break;
        }

        // Is the current second over?
        gettimeofday(&tve, NULL);
        msecs = MSECS(tve) - MSECS(tvb);

        if(msecs >= 1000) {
            // clear and return to beginning of line and log stats
            printf("%c[2K\r", 27);
            printf("%d Events received in %dms avgeraging %f events per second.", num_events, msecs, (float)num_events/msecs*1000);
            fflush(stdout);
            msecs = 0;
            num_events = 0;
        }
    }
    return 0;
}
