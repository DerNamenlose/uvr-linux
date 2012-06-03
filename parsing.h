#ifndef PARSING_H
#define PARSING_H

#include "datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * parse an input entry in the given buffer
 */
struct SensorListNode *parseInput(unsigned char *buffer);


/**
 * parse the buffer from a UVR1611
 * 
 * \return NULL on error, a sensor node else
 */
struct SensorListNode *parseUVR1611(unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif /* PARSING_H */