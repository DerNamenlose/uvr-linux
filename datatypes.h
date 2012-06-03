#ifndef DATATYPES_H
#define DATATYPES_H

#include <termios.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The different commands for the device
 */
#define GET_MODE          0x81u
#define GET_CURRENT_DATA  0xABu

/**
 * identiers of the different UVRs
 */
#define UVR1611 0x80

/**
 * structure representing a USB connection.
 */
struct USBConnection
{
    char *device;
    int fd;
    struct termios _savedattrs;
    struct termios _newattrs;
    unsigned char uvr_mode;
    char _success;
};

/**
 * different types of sensors
 */
#define UNUSED 0
#define DIGITAL 1
#define TEMPERATURE 2
#define FLOW 3
#define RADIATION 6
#define ROOM_TEMPERATURE 7

struct SensorValue
{
    unsigned char sensor;
    union {
        float temperature;
        int   enabled;
        int   flow;
        int   radiation;
    } value;
    int sensorType;
};

/**
 * a list node of a sensor list
 */
struct SensorListNode
{
    struct SensorValue sensor;
    struct SensorListNode *next;
};

/**
 * clean up a sensor list
 */
void freeSensorList(struct SensorListNode *list);

/**
 * create a new SensorListNode
 */
struct SensorListNode *createSensorListNode();

#ifdef __cplusplus
}
#endif

#endif /* DATATYPES_H */