/*
 * min_max.h
 *
 *  Created on: 2 wrz 2017
 *      Author: kamil
 */

#ifndef SRC_SOURCE_MIN_MAX_H_
#define SRC_SOURCE_MIN_MAX_H_


#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif

#ifdef __GNUC__

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#else

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#endif


#endif /* SRC_SOURCE_MIN_MAX_H_ */
