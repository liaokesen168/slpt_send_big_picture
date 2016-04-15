#ifndef __FRIZZ_DEBUG_H__
#define __FRIZZ_DEBUG_H__

/*!< debug message on or off */

//#define DEBUG_FRIZZ

#ifdef DEBUG_FRIZZ
 #define DEBUG_PRINT printf
 #define kprint(fmt,args...) printf("\033[1;31m" fmt "\033[0m \n", ##args);
#else
 #define DEBUG_PRINT 1 ? (void) 0 : printf
 #define kprint(fmt,args...) printf("\033[1;31m" fmt "\033[0m \n", ##args);
#endif

#endif
