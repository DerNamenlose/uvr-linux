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