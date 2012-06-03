/*
    UVR-Linux - a collection of programs to access data on 
    Technische Alternative UVR-type devices.
    Copyright (C) 2012  Markus Brueckner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


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