#include <stdlib.h>
#include <stdio.h>

#include "xbox.h"

int main() {
    int xbox_inputs = open_controllers(0);
    if(xbox_inputs == 0) {
        printf("Error: No XBox controllers found. Try running as root.");
        return 1;
    }
    printf("Found %d controllers.\n", xbox_inputs);
    return 0;
}
