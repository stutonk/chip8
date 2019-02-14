#include <stddef.h>
#include <sys/time.h>
#include "timer.h"

static uint8_t Delay = 0;
static uint8_t Sound = 0;
static struct timeval Prevtime = {0};
static struct timeval Currtime = {0};

static inline long timediff(struct timeval *, struct timeval *);

void timer_init()
{
    gettimeofday(&Prevtime, NULL);
}

void timer_update()
{
    gettimeofday(&Currtime, NULL);
    if (timediff(&Prevtime, &Currtime) >= 16666) {
        Delay -= (Delay) ? 1 : 0;
        Sound -= (Sound) ? 1 : 0;
        Prevtime.tv_usec = Currtime.tv_usec;
        Prevtime.tv_sec = Currtime.tv_sec;
    }
}

void timer_set_delay(uint8_t val)
{
    Delay = val;
}

uint8_t timer_get_delay()
{
    return Delay;
}

void timer_set_sound(uint8_t val)
{
    Sound = val;
}

uint8_t timer_get_sound()
{
    return Sound;
}

static inline long timediff(struct timeval *before, struct timeval *after)
{
    long diff = 0;
    diff = after->tv_sec * 1000000 + after->tv_usec;
    diff -= before->tv_sec * 1000000 + before->tv_usec;
    return diff;
}