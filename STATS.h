//
//  STATS.h
//  STATS
//
//  Created by Kyle Horne on 2019-03-18.
//  Copyright © 2019 Kyle Horne. All rights reserved.
//

#ifndef STATS_h
#define STATS_h

/* The size of the array. */
#define SZ 5

/* The shared data structure amongst the processes. */
struct shared_data {
    int mutex[SZ];
    int     B[SZ];
    int  is_alive;
};

#include <sys/types.h>
#include <sys/sem.h>

#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
! defined(__sgi) && ! defined(__APPLE__)

union semun {
    int                 val;
    struct semid_ds*    buf;
    unsigned short*     array;
    #if defined(__linux__)
        struct seminfo* __buf;
    #endif
};

#endif

#endif /* STATS_h */
