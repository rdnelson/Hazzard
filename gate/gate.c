#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>

#include <sys/time.h>

#define EXP_CONST 0.5
#define OFFSET 99

#define TO_TIME(freq) ((int)(1/(double)(freq) * 1000000))
#define TO_FREQ(time) ((int)((1/(double)(time)) * 1000000))
#define IS_IN_RANGE(val, target, offset) ((val >= (target - offset)) && val <= (target + offset))

// runningDiff:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.

static volatile int runningDiff[8];
static struct timeval currentTime[8];
static struct timeval lastTime[8];
static int freq;
static volatile int count[4];
static struct timeval lastLap[4];

long long
timeval_diff(struct timeval *end_time,
        struct timeval *start_time
        );

double getFreq(int gate){
    gettimeofday(&currentTime[gate], NULL);
    long long timeDiff = timeval_diff(&currentTime[gate], &lastTime[gate]);
    lastTime[gate] = currentTime[gate];
    if (TO_FREQ(timeDiff) < 10) {
        return 0;
    }
    runningDiff[gate] = runningDiff[gate] + EXP_CONST * (timeDiff - runningDiff[gate]);
    if (runningDiff[gate] == 0)
        return 0;
    return (1/(double)runningDiff[gate]) * 1000000;
}


void myInterrupt0 (void) {
    int i;
    freq = (int)getFreq(0);
    if (IS_IN_RANGE(freq, 100, OFFSET)) {
        if (count[0] >= 0) count[0] += 2;
    }
    else if (IS_IN_RANGE(freq, 500, OFFSET)) {
        if (count[1] >= 0) count[1] += 2;
    }
    for (i = 0; i < 2; i++)
        if (count[i] > 5) {
            if (timeval_diff(&currentTime[0], &lastLap[i]) > 1000000)
                printf("Got one!\n");
            gettimeofday(&lastLap[i], NULL);
        }
        if (count[i] > 0) count[i]--; 
}

int main (void)
{
    int lastFreq;
    int pin;
    int myCounter [8];

    for (pin = 0 ; pin < 8 ; ++pin) {
        runningDiff [pin] = myCounter [pin] = 0 ;
        gettimeofday(&lastTime[pin], NULL);
    }

    wiringPiSetup () ;

    freq = lastFreq = 0;
    wiringPiISR (0, INT_EDGE_RISING, &myInterrupt0);
    pullUpDnControl (0, PUD_UP);
    pullUpDnControl (2, PUD_UP);

    for (;;)
    {
        //if (freq == 0)
        //    continue;
        if (count[0] > count[2]) count[2] = count[0];
        lastFreq = freq;
        //printf("Freq: %d; 1000=%d; 2000=%d; max=%d\n", freq, count[0], count[1], count[2]);
        /*
        if (IS_IN_RANGE(freq, 1000, OFFSET))
            printf("1000!\n");
        else if (IS_IN_RANGE(freq, 2000, OFFSET))
            printf("2000!******************************************\n");
        else if (IS_IN_RANGE(freq, 3000, OFFSET))
            printf("3000!******************************************\n");
        */
        fflush(stdout);
    }

    return 0;
}

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
