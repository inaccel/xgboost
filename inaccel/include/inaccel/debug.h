#ifndef DEBUG_H_
#define DEBUG_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
  #define DEBUG_PRINT(...) fprintf( stderr, __VA_ARGS__ )
  #define DEBUG_PERROR(...) perror( __VA_ARGS__ )
#else
  #define DEBUG_PRINT(...) do{ } while ( 0 )
  #define DEBUG_PERROR(...) do{ } while ( 0 )
#endif

#define INACCEL_SUCCESS (0)
#define INACCEL_FAILURE (-1)

#endif /* DEBUG_H_ */
