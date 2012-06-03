#include <stdlib.h>

#include "datatypes.h"

/**
 * clean up a sensor list
 */
void freeSensorList(struct SensorListNode *list)
{
    struct SensorListNode *ptr;
    ptr = list;
    while (ptr != NULL) {
        struct SensorListNode *tmp;
        tmp = ptr;
        ptr = ptr->next;
        free(tmp);
    }
}

struct SensorListNode *createSensorListNode()
{
    struct SensorListNode *tmp;
    tmp = malloc(sizeof(struct SensorListNode));
    if (tmp != NULL) {
        tmp->next = NULL;
    }
    return tmp;
}
