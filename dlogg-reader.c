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


#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "communication.h"

/**
 * print a sensor value to stdout
 */
void printSensor(struct SensorValue *sensor)
{
    printf("S%d: ", sensor->sensor);
    switch (sensor->sensorType) {
        case UNUSED:
            printf("---");
            break;
        case DIGITAL:
            printf(sensor->value.enabled ? "on" : "off");
            break;
        case TEMPERATURE:
            printf("%.1f °C", sensor->value.temperature);
            break;
        case FLOW:
            printf("%d l/h", sensor->value.flow);
            break;
        default:
            printf("UNKNOWN");
            break;
    }
}

int main(int argc, char *argv[]) {
    struct USBConnection *connection;
    if (argc != 2) {
        printf("Usage: %s <USB device>\n", argv[0]);
        return -1;
    }
    connection = initUSBConnection(argv[1]);
    if (connection != NULL) {
        int i;
        fprintf(stderr, "Connection initialization successful. UVR mode 0x%X\n", (unsigned int)connection->uvr_mode);
        for (i = 0; i < 30; ++i) {
            struct SensorListNode * result;
            result = readCurrentData(connection);
            struct SensorListNode *it;
            it = result;
            while (it != NULL) {
                printSensor(&(it->sensor));
                printf("\n");
                it = it->next;
            }
            freeSensorList(result);
            sleep(3);
        }
    }
    else {
        fprintf(stderr, "Could not initialize connection to UVR. %s\n", strerror(errno));
        cleanupUSBConnection(connection);
        return -1;
    }
    cleanupUSBConnection(connection);
    return 0;
}