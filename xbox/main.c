#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "xbox.h"

#define MSECS(a) (a.tv_sec * 1000 + a.tv_usec / 1000)

int main() {
    int xbox_inputs = open_controllers();
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

    while(1) {
        if(msecs == 0)
            gettimeofday(&tvb, NULL);

        e = get_event();
        if(e.player != -1)
            //printf("Player: %d Event: %d, Value: %d\n", e.player, e.event, e.data);
            num_events++;
        if(e.event == BUTTON_B && e.data == 1) {
            break;
        }
        gettimeofday(&tve, NULL);
        msecs = MSECS(tve) - MSECS(tvb);

        if(msecs >= 1000) {
            printf("%c[2K\r", 27);
            printf("%d Events received in %dms avgeraging %f events per second.", num_events, msecs, (float)num_events/msecs*1000);
            fflush(stdout);
            msecs = 0;
            num_events = 0;
        }
    }
    return 0;
}
