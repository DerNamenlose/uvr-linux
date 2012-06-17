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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "communication.h"
#include "parsing.h"
#include "logging.h"

/**
 * send a command to the device
 * 
 * \param conn the connection to the device
 * \param command the command to send
 * \return 0 on success, -1 else. errno will be set accordingly
 */
int sendCommand(struct USBConnection *conn, unsigned char command)
{
    if (conn != NULL) {
        if (write(conn->fd, &command, 1) != 1) {
            log_output(LOG_ERR, "Could not write to device. %s\n", strerror(errno));
            return -1;
        }
        return 0;
    }
    log_output(LOG_ERR, "Write to device failed. Connection is invalid.\n");
    return -1;
}

/**
 * cleanup the USB connection.
 * 
 * \param conn The connection to clean up. This must point to a pointer obtained by initUSBConnection()
 * \note Do <em>NOT</em> use the connection afterwards. The pointer will be invalid.
 */
void cleanupUSBConnection(struct USBConnection *conn)
{
    if (conn != NULL) {
        if (conn->_success) {
            tcsetattr(conn->fd, TCSANOW, &(conn->_savedattrs));
        }
        if (conn->device != NULL) {
            free(conn->device);
        }
        if (conn->fd > 0) {
            close(conn->fd);
        }
        free(conn);
    }
}

/**
 * open the connection to a D-LOGG USB device on the given path
 * 
 * \param device the device path where the D-LOGG is to be found
 * \return a pointer to a USBConnection structure containing the necessary information to talk to the device. NULL, if anything failed
 *         when initializing the connection. errno is set accordingly.
 */
struct USBConnection *initUSBConnection(char const * const device)
{
    struct USBConnection *conn;
    conn = malloc(sizeof(struct USBConnection));
    if (conn != NULL) {
        conn->fd = -1;
        conn->_success = 0;
        conn->device = malloc(strlen(device)+1);
        if (conn->device != 0) {
            strcpy(conn->device, device);
            conn->fd = open(device, O_NOCTTY | O_RDWR);
            if (conn->fd >= 0) {
                log_output(LOG_DEBUG, "Successfully opened USB device\n");
                // the opening has been successful
                // -> setup the serial connection (D-LOGG is a serial connector)
                if (tcgetattr(conn->fd, &(conn->_savedattrs)) == 0) {
                    memset(&(conn->_newattrs), 0, sizeof(struct termios));
                    conn->_newattrs.c_cflag     = B115200 | CS8 | CLOCAL | CREAD;
#ifdef CRTSCTS
                    conn->_newattrs.c_cflag    |= CRTSCTS;
#endif
#ifdef CNEW_RTSCTS
                    conn->_newattrs.c_cflag    |= CNEW_RTSCTS;
#endif
                    conn->_newattrs.c_iflag     = IGNPAR;
                    conn->_newattrs.c_oflag     = 0;
                    conn->_newattrs.c_lflag     = 0;
                    conn->_newattrs.c_cc[VTIME] = 0;   /* read block infinitely */
                    conn->_newattrs.c_cc[VMIN]  = 1;   /* minimum 1 character to be read */
                    tcflush(conn->fd, TCIFLUSH);
                    if (tcsetattr(conn->fd, TCSANOW, &(conn->_newattrs)) == 0) {
                        log_output(LOG_DEBUG, "Initializing device.\n");
                        if (sendCommand(conn, GET_MODE) == 0) {
                            if (read(conn->fd, &(conn->uvr_mode), 1) == 1) {
                                conn->_success = 1;  // initialization done
                            }
                            else {
                                log_output(LOG_ERR, "Could not read device reply. %s\n", strerror(errno));
                            }
                        }
                    }
                    else {
                        log_output(LOG_ERR, "Could not setup USB device. %s\n", strerror(errno));
                    }
                }
                else {
                    log_output(LOG_ERR, "Could not get attributes of serial interface. %s\n", strerror(errno));
                }
            }
        }
    }
    else {
        log_output(LOG_ERR, "Could not allocate memory. %s\n", strerror(errno));
    }
    if (conn != NULL && !conn->_success) {
        /* somewhere along the way we couldn't successfully initialize -> clean up */
        log_output(LOG_ERR, "Could not open USB device %s. %s\n", device, strerror(errno));
        cleanupUSBConnection(conn);
        conn = NULL;
    }
    log_output(LOG_DEBUG, "Returning connection %p\n", conn);
    return conn;
}

/**
 * read a set of data into the buffer. This function reads as long as the buffer
 * is not filled to the amount needed or an error occurs.
 * 
 * \return the number of bytes read on success, <0 else. errno will be set accordingly
 */
int readBuffer(struct USBConnection *conn, unsigned char *buffer)
{
    int ret = 0;
    int numBytes = 0;
    int bytesToRead = 115;
    while (bytesToRead != numBytes) {
        ret = read(conn->fd, buffer+numBytes, bytesToRead-numBytes); // 115 would be if two UVR1611 are connected
        if (ret <= 0) {
            return ret;
        }
        numBytes += ret;
        switch (buffer[0]) {
            case GET_CURRENT_DATA:
                // this means that we don't have new data
                log_output(LOG_DEBUG, "No new data currently. (read %d bytes)\n", ret);
                errno = EAGAIN;
                return -1;
            case UVR1611:
                switch (conn->uvr_mode) {
                    case 0xA8:
                        if (bytesToRead == 115) {
                            bytesToRead = 57; // 57 bytes are to be read in this mode
                        }
                        break;
                    default:
                        log_output(LOG_ERR, "Unsupported mode %x\n", conn->uvr_mode);
                        errno = EINVAL;
                        return -1;
                }
                break;
            default:
                log_output(LOG_ERR, "Unsupported device %x\n", buffer[0]);
                errno = EINVAL;
                return -1;
        }
    }
    return numBytes;
}

/**
 * read the current data values from the device
 * 
 * \return a pointer to the sensor list on success, NULL otherwise. errno will be set accordingly.
 * \note if the sensor list is no longer needed, release it using freeSensorList()
 */
struct SystemState *readCurrentData(struct USBConnection *conn)
{
    int ret;
    unsigned char databuffer[116]; // according to the specification there might be 115 bytes max
    if (conn == 0 || !conn->_success) {
        errno = EINVAL;
        return NULL;
    }
    sendCommand(conn, GET_CURRENT_DATA);
    // depending on the number of bytes read, different results are to be expected
    ret = readBuffer(conn, databuffer);
    if (ret > 0) {
        log_output(LOG_DEBUG, "Read buffer of size: %d\n", ret);
        switch(databuffer[0]) {
            case UVR1611:
                return parseUVR1611(databuffer);
            default:
                return NULL;
                break;
        }
    }
    return NULL;
}
