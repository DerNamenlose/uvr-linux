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

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "communication.h"
#include "logging.h"
#include <stdlib.h>

void daemonize()
{
    pid_t pid = fork();
    if (pid > 0) {
        // we're in the parent process -> exit
        exit(0);
    }
    // standard UNIX daemon setup
    umask(0);
    pid_t sid = setsid();
    if (sid < 0) {
        fprintf(stderr, "Could not get new session id");
        exit(-1);
    }
    if ((chdir("/")) < 0) {
        fprintf(stderr, "Could not change directory to /");
        exit(-1);
    }
    log_output(LOG_DEBUG, "Sucessfully daemonized reader.\n");
    // close STDIO
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

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

void executeProgram(char *program, struct SensorListNode *sensors)
{
    pid_t child;
    child = fork();
    if (child == 0) {
        // we're in the child
        struct SensorListNode *si = sensors;
        char valuebuf[100];
        int counter = 0;
        while (si != NULL) {
            char varbuf[100];
            snprintf(varbuf, 100, "UVR_INPUT_%d_TYPE", si->sensor.sensor);
            switch(si->sensor.sensorType) {
                case UNUSED:
                    snprintf(valuebuf, 100, "UNUSED");
                    break;
                case DIGITAL:
                    snprintf(valuebuf, 100, "DIGITAL");
                    break;
                case TEMPERATURE:
                    snprintf(valuebuf, 100, "TEMPERATURE");
                    break;
                case FLOW:
                    snprintf(valuebuf, 100, "FLOW");
                    break;
            }
            setenv(varbuf, valuebuf, 1);
            snprintf(varbuf, 100, "UVR_INPUT_%d_VALUE", si->sensor.sensor);
            switch(si->sensor.sensorType) {
                case UNUSED:
                    snprintf(valuebuf, 100, "UNUSED");
                    break;
                case DIGITAL:
                    snprintf(valuebuf, 100, si->sensor.value.enabled ? "1" : "0");
                    break;
                case TEMPERATURE:
                    snprintf(valuebuf, 100, "%.1f", si->sensor.value.temperature);
                    break;
                case FLOW:
                    snprintf(valuebuf, 100, "%d", si->sensor.value.flow);
                    break;
            }
            setenv(varbuf, valuebuf, 1);
            counter++;
            si = si->next;
        }
        snprintf(valuebuf, 100, "%d", counter);
        setenv("UVR_INPUTS", valuebuf, 1);
        log_output(LOG_DEBUG, "Executing %s\n", program);
        system(program);
        log_output(LOG_DEBUG, "%s finished\n", program);
        exit(0);
    }
    else {
        waitpid(child, NULL, 0); // sleep until the child returns
        log_output(LOG_DEBUG, "Child returned\n");
    }
}

void printUsage(char *command)
{
    fprintf(stderr, "Usage: %s [-s <program>] [-d <delay>] [-c <count>] <USB device>\n", command);
    fprintf(stderr, "  -s    Execute the program given as a parameter and\n");
    fprintf(stderr, "        hand it the values in the environment instead\n");
    fprintf(stderr, "        of printing them to stdout. The values are handed\n");
    fprintf(stderr, "        over in form of environment variables named\n");
    fprintf(stderr, "        UVR_INPUT_<number>_<type>, with <number> being\n");        
    fprintf(stderr, "        the sensor number and <type> being either\n");        
    fprintf(stderr, "        VALUE or TYPE. The _TYPE-variable contains one\n");        
    fprintf(stderr, "        of UNUSED, DIGITAL, TEMPERATURE or FLOW, corresponding\n");
    fprintf(stderr, "        to the respective sensor types. Digital sensors may have a value\n");
    fprintf(stderr, "        of 0 or 1, temperature sensors contain the temperature in °C,\n");
    fprintf(stderr, "        flow sensor values are in l/h. The number of inputs is contained\n");
    fprintf(stderr, "        in UVR_INPUTS.\n");        
    fprintf(stderr, "  -d    Set the delay between the value updates in seconds. (default: 10)\n");
    fprintf(stderr, "  -c    Set the repetition counter. A repetition counter of 0 means run infinitely. (default: 0)\n");
    fprintf(stderr, "  -D    Run the program as a daemon. The reader forks into the background and detaches from the terminal\n");
    fprintf(stderr, "        This implies -s as a daemon cannot make any output.\n");
    fprintf(stderr, "  -v    Enable debug output.\n");
}

int main(int argc, char *argv[]) {
    struct USBConnection *connection;
    int opt;
    int repeatCount = 0;
    char *script = NULL;
    int delay = 10;
    int daemon = 0;
    while ((opt = getopt(argc, argv, "s:d:c:Dv")) != -1) {
        switch (opt) {
            case 's':
                script = optarg;
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'c':
                repeatCount = atoi(optarg);
                break;
            case 'D':
                daemon = 1;
                break;
            case 'v':
                enable_debug();
                break;
            default:
                printUsage(argv[0]);
                return -1;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Missing USB device parameter.\n");
        printUsage(argv[0]);
        return -1;
    }
    if (daemon && script == NULL) {
        fprintf(stderr, "Missing script parameter. Running the program as a daemon implies -s.\n");
        return -1;
    }
    if (daemon) {
        initlog(1);
        daemonize();
    }
    else {
        initlog(0);
    }
    log_output(LOG_DEBUG, "Opening USB device\n");
    connection = initUSBConnection(argv[optind]);
    if (connection != NULL) {
        int i;
        int increment = 1;
        log_output(LOG_INFO, "Connection initialization successful. UVR mode 0x%X\n", (unsigned int)connection->uvr_mode);
        if (repeatCount == 0) {
            increment = 0;
            repeatCount = 1; // prepare the values in a way that the loop below runs infinitely
        }
        for (i = 0; i < repeatCount; i+=increment) {
            struct SensorListNode * result;
            result = readCurrentData(connection);
            if (script != NULL) {
                executeProgram(script, result);
            }
            else {
                struct SensorListNode *it;
                it = result;
                while (it != NULL) {
                    printSensor(&(it->sensor));
                    printf("\n");
                    it = it->next;
                }
            }
            freeSensorList(result);
            sleep(delay);
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