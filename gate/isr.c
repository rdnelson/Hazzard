#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>

#include <sys/time.h>

#define EXP_CONST 0.008

// runningDiff:
//	Global variable to count interrupts
//	Should be declared volatile to make sure the compiler doesn't cache it.

static volatile int runningDiff[8];
static struct timeval currentTime[8];
static struct timeval lastTime[8];
static int freq;
static int lastFreq;

long long
timeval_diff(struct timeval *difference,
        struct timeval *end_time,
        struct timeval *start_time
        );

double getFreq(int gate){
    gettimeofday(&currentTime[gate], NULL);
    long long timeDiff = timeval_diff(NULL, &currentTime[gate], &lastTime[gate]);
    lastTime[gate] = currentTime[gate];
    if (timeDiff > 1337)
        return 0;
    runningDiff[gate] = runningDiff[gate] + EXP_CONST * (timeDiff - runningDiff[gate]);
    if (runningDiff[gate] == 0)
        return 0;
    return (1/(double)runningDiff[gate]) * 1000000;
}


void myInterrupt0 (void) {
    lastFreq = freq;
    freq = (int)getFreq(0);
}

int main (void)
{
    int gotOne, pin;
    int myCounter [8];

    for (pin = 0 ; pin < 8 ; ++pin) {
        runningDiff [pin] = myCounter [pin] = 0 ;
        gettimeofday(&lastTime[pin], NULL);
    }

    wiringPiSetup () ;

    wiringPiISR (0, INT_EDGE_RISING, &myInterrupt0);
    pullUpDnControl (0, PUD_DOWN);
    pullUpDnControl (2, PUD_UP);

    for (;;)
    {
            printf("Freq: %d\n", freq);
        if (freq != lastFreq) {
            //printf("%c[2K\r", 27);
        }
    }

    return 0;
}

    long long
timeval_diff(struct timeval *difference,
        struct timeval *end_time,
        struct timeval *start_time
        )
{
    struct timeval temp_diff;

    if(difference==NULL)
    {
        difference=&temp_diff;
    }

    difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
    difference->tv_usec=end_time->tv_usec-start_time->tv_usec;

    while(difference->tv_usec<0)
    {
        difference->tv_usec+=1000000;
        difference->tv_sec -=1;
    }

    return 1000000LL*difference->tv_sec+
        difference->tv_usec;

} /* timeval_diff() */
