//
//  semun.h
//  STATS
//
//  Created by Kyle Horne on 2019-03-18.
//  Copyright © 2019 Kyle Horne. All rights reserved.
//

#ifndef semun_h
#define semun_h

#include <sys/types.h>
#include <sys/sem.h>

#if ! defined(__FreeBSD__) && ! defined(__OpenBSD__) && \
    ! defined(__sgi) && ! defined(__APPLE__)

union semun {
    int                  val;
    struct semid_ds *    buf;
    unsigned short *     array;
    #if defined(__linux__)
        struct seminfo * __buf;
    #endif
};

#endif

#endif /* semun_h */
