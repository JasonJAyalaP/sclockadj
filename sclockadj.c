#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

/* receive time adjustment, negative or positive, in nanoseconds */
/* long long 64bit is at least −9,223,372,036,854,775,807 to +9,223,372,036,854,775,807 */
/* exits program upon failure */
void change_time_by_nanoseconds(long long add_ns)
{
    struct timespec tps; /* tv_sec; tv_nsec */
    if (clock_gettime(0, &tps) == -1) /* get current time in seconds since epoch + nanoseconds offset */
    {
        perror("Failed to get current time!");
        exit(EXIT_FAILURE);
    }
    /* combine seconds and nanoseconds offset in order to add nanoseconds*/
    long long ns_since_epoch =
        (long long)(tps.tv_sec) * 1000000000 + /* convert seconds to nanoseconds */
        (long long)(tps.tv_nsec);              /* add offset */
    long long new_ns_since_epoch = ns_since_epoch + add_ns;
    /* separate new nanoseconds since epoch into seconds and nanoseconds offset */
    long new_s = new_ns_since_epoch / 1000000000;  /* truncates into seconds */
    long new_ns = new_ns_since_epoch % 1000000000; /* nanoseconds remainder */
    /* set old struct with new values */
    tps.tv_sec = new_s;
    tps.tv_nsec = new_ns;
    /* set time with new values */
    if (clock_settime(0, &tps) == -1)
    {
        perror("Failed to change clock!");
        exit(EXIT_FAILURE);
    }
}

/* intended to be used only by sdwdate with sane inputs */
int main(int argc, char *argv[])
{
    if (argc < 1) exit(EXIT_FAILURE);
    long long ns_time_change = atoll(argv[1]); /* convert argv string into long long */
    if (ns_time_change == 0) return 1; /* exit if atoll fails */
    /* since nanosecond jump is fixed, we can count the number of complete jumps. */
    /* llabs for negative numbers */
    long long number_of_full_jumps = llabs(ns_time_change) / 5000000;  /* times we'll move clock by 5,000,000 ns at a time */
    long long last_jump_nanoseconds = llabs(ns_time_change) % 5000000; /* then add remaining < 5,000,000 ns */

    if (ns_time_change > 0) /* positive nanosecond change */
    {
        for (unsigned i = 0; i < number_of_full_jumps; ++i) 
        {
            sleep(1);  /* a 1 second wait imitates ntpdate */
            change_time_by_nanoseconds(5000000); /* 5,000,000 ns imitates ntpdate */
        }
        sleep(1);
        change_time_by_nanoseconds(last_jump_nanoseconds);
    }
    else  /* negative nanosecond change */
    {
        for (unsigned i = 0; i < number_of_full_jumps; ++i) 
        {
            sleep(1);
            change_time_by_nanoseconds(-5000000);
        }
        sleep(1);
        change_time_by_nanoseconds(-last_jump_nanoseconds); /* negative of absolute value imitates Euclidean modulo */
    }
    return EXIT_SUCCESS;
}
