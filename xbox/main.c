#include <stdlib.h>
#include <stdio.h>

#include "xbox.h"

int main() {
    int xbox_inputs = open_controllers();
    if(xbox_inputs == 0) {
        printf("Error: No XBox controllers found. Try running as root.");
        return 1;
    }
    printf("Found %d controllers.\n", xbox_inputs);

    struct player_event e;
    while(1) {
        e = get_event();
        if(e.player != -1)
            printf("Player: %d Event: %d, Value: %d\n", e.player, e.event, e.data);
        if(e.event == BUTTON_B && e.data == 1) {
            break;
        }
    }
    return 0;
}
