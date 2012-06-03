#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "datatypes.h"

/**
 * send a command to the device
 * 
 * \param conn the connection to the device
 * \param command the command to send
 * \return 0 on success, -1 else. errno will be set accordingly
 */
int sendCommand(struct USBConnection *conn, unsigned char command);

/**
 * cleanup the USB connection.
 * 
 * \param conn The connection to clean up. This must point to a pointer obtained by initUSBConnection()
 * \note Do <em>NOT</em> use the connection afterwards. The pointer will be invalid.
 */
void cleanupUSBConnection(struct USBConnection *conn);

/**
 * open the connection to a D-LOGG USB device on the given path
 * 
 * \param device the device path where the D-LOGG is to be found
 * \return a pointer to a USBConnection structure containing the necessary information to talk to the device. NULL, if anything failed
 *         when initializing the connection. errno is set accordingly.
 */
struct USBConnection *initUSBConnection(char const * const device);


/**
 * read a set of data into the buffer. This function reads as long as the buffer
 * is not filled to the amount needed or an error occurs.
 * 
 * \return the number of bytes read on success, <0 else. errno will be set accordingly
 */
int readBuffer(struct USBConnection *conn, unsigned char *buffer);

/**
 * read the current data values from the device
 * 
 * \return a pointer to the sensor list on success, NULL otherwise. errno will be set accordingly.
 * \note if the sensor list is no longer needed, release it using freeSensorList()
 */
struct SensorListNode *readCurrentData(struct USBConnection *conn);

#ifdef __cplusplus
}
#endif

#endif /* COMMUNICATION_H */