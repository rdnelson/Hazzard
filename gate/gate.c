#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>

#include <sys/time.h>

#define EXP_CONST 0.5
#define OFFSET 99

//Define macros, names self explanitory
#define TO_TIME(freq) ((int)(1/(double)(freq) * 1000000))
#define TO_FREQ(time) ((int)((1/(double)(time)) * 1000000))
#define IS_IN_RANGE(val, target, offset) ((val >= (target - offset)) && val <= (target + offset))

#define MIN_CUTTOFF 10
#define NUM_GATES 1
#define NUM_PLAYERS 2

// runningDiff:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.

//typedef for Python callback
typedef void (*callback_t)(int);

//Global variables
static volatile int runningDiff[NUM_GATES];
static struct timeval currentTime[NUM_GATES];
static struct timeval lastTime[NUM_GATES];
static int freq;
static volatile int count[NUM_PLAYERS];
static struct timeval lastLap[NUM_PLAYERS];
static callback_t pythonCallback;

//Calculate the difference between two time values
long long
timeval_diff(struct timeval *end_time,
        struct timeval *start_time
        );

//Add Python callback
void add_callback(callback_t callback);

//Get frequency and update running totals
double getFreq(int gate){
    gettimeofday(&currentTime[gate], NULL);
    long long timeDiff = timeval_diff(&currentTime[gate], &lastTime[gate]);
    lastTime[gate] = currentTime[gate];
    // High pass to ignore first trigger when a car passes, and catch only
    // actual edges of the blink
    if (TO_FREQ(timeDiff) < MIN_CUTTOFF) {
        return 0;
    }
    //Running average calulation, 
    runningDiff[gate] = runningDiff[gate] + EXP_CONST * (timeDiff - runningDiff[gate]);
    //Don't divide by 0
    if (runningDiff[gate] == 0)
        return 0;
    return TO_FREQ(runningDiff[gate]);
}


void myInterrupt0 (void) {
    int i;
    //Update frequency
    freq = (int)getFreq(0);

    //Is this car one?
    if (IS_IN_RANGE(freq, 100, OFFSET)) {
        if (count[0] >= 0) count[0] += 2;
    }
    //Is this car two?
    else if (IS_IN_RANGE(freq, 500, OFFSET)) {
        if (count[1] >= 0) count[1] += 2;
    }
    //Have any of the cars triggered a lap?
    for (i = 0; i < 2; i++)
        if (count[i] > 5) {
            if (timeval_diff(&currentTime[0], &lastLap[i]) > 1000000)
                pythonCallback(i);
	    count[i] = 0;
            gettimeofday(&lastLap[i], NULL);
        }
        if (count[i] > 0) count[i]--; 
}

//Init the ISR, and register the callback
int init (callback_t callback)
{
    int pin;
    int myCounter [NUM_GATES];

    for (pin = 0 ; pin < NUM_GATES ; ++pin) {
        runningDiff [pin] = myCounter [pin] = 0 ;
        gettimeofday(&lastTime[pin], NULL);
    }

    wiringPiSetup () ;

    add_callback(callback);

    wiringPiISR (0, INT_EDGE_RISING, &myInterrupt0);
    pullUpDnControl (0, PUD_UP);
    pullUpDnControl (2, PUD_UP);

    return 0;
}

//Calculate the time different in miliseconds between two events
    long long
timeval_diff(struct timeval *end_time,
        struct timeval *start_time
        )
{
    struct timeval difference;

    difference.tv_sec =end_time->tv_sec -start_time->tv_sec ;
    difference.tv_usec=end_time->tv_usec-start_time->tv_usec;

    while(difference.tv_usec<0)
    {
        difference.tv_usec+=1000000;
        difference.tv_sec -=1;
    }

    return 1000000LL*difference.tv_sec+
        difference.tv_usec;

} /* timeval_diff() */

//Add a python callback
void add_callback(callback_t callback) {
    if (callback != NULL) {
        pythonCallback = callback;
    }
}

