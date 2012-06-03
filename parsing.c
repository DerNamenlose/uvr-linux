#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "parsing.h"

struct SensorListNode *parseInput(unsigned char *buffer)
{
    struct SensorListNode *node;
    int value;
    node = createSensorListNode();
    if (node != NULL) {
        node->sensor.sensorType = (buffer[1] & 0x70) >> 4;  // bits 6, 5, and 4 of the high byte indicate the sensor type
        value = buffer[1] & 0x0F;
        value <<= 8;
        value += buffer[0];
        switch (node->sensor.sensorType) {
            case UNUSED:
                break;
            case DIGITAL:
                if (buffer[1] & 0x80 != 0) {
                    node->sensor.value.enabled = 1;
                }
                else {
                    node->sensor.value.enabled = 0;
                }
                break;
            case TEMPERATURE:
                node->sensor.value.temperature = ((float)value) / 10.0;
                break;
            case FLOW:
                node->sensor.value.flow = value * 4;
                break;
            default:
                fprintf(stderr, "Unsupported sensor type so far\n");
                free(node);
                node = NULL;
                break;
        }
    }
    return node;
}


/**
 * parse the buffer from a UVR1611
 * 
 * \return NULL on error, a sensor node else
 */
struct SensorListNode *parseUVR1611(unsigned char *buffer)
{
    int i;
    struct SensorListNode *head;
    struct SensorListNode *last;
    head = last = NULL;
    for (i = 0; i < 16; ++i) { // UVR1611 has 16 inputs
        struct SensorListNode *node;
        node = parseInput(buffer+2*i+1); // every input takes 2 bytes and the first one is at offset 1
        if (node != NULL) {
            node->sensor.sensor = i+1;
            if (head == NULL) {
                head = node;
            }
            if (last != NULL) {
                last->next = node;
            }
            last = node;
        }
    }
    // TODO parse outputs
    return head;
}
