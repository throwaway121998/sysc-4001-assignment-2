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
    int B[SZ];
    int is_alive;
};


#endif /* STATS_h */
