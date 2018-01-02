#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include "utils.h"

/**
 * Initializes the Link Layer settings.
 *
 * @param porta The Serial Port's port.
 * @param baudRate The transmission velocity.
 * @param timeout The timer's value.
 * @param numTransmissions Number of tries in case of failure.
 * @return OK if everything went as expected, ERROR otherwise.
 */
int initLinkLayer(int porta, int baudRate, uint timeout, uint numTransmissions);

/**
 * Opens the Serial Port with the LinkLayer settings. See function initLinkLayer.
 *
 * @return The filedescriptor corresponding to the Serial Port.
 */
int openSerialPort();

/**
 * Establish the connection between the two computers, according to the protocol.
 *
 * @param type The type of conection, either a receiver or a transmitter.
 * @return negative if error ocurred, the Serial Port's filedescriptor otherwise.
 */
int llopen(ConnectionType type);

/**
 * Finish the connection between the two computers, according to the protocol.
 *
 * @param fd The Serial Port's filedescriptor.
 * @return OK if everything went as expected, negative value otherwise.
 */
int llclose(int fd);

/**
 * Write information through the Serial Port.
 *
 * @param fd The Serial Port's filedescriptor.
 * @param buffer The adress of the buffer containing the message to be written.
 * @param length The buffer's length.
 * @retun Number of written characters, negative value if an error ocurred.
 */
int llwrite(int fd, uchar ** buffer, int length);

/**
 * Read information through the Serial Port.
 *
 * @param fd The Serial Port's filedescriptor.
 * @param buffer The adress of the buffer that will contain the read message.
 * @retun Number of read characters, negative value if an error ocurred.
 */
int llread(int fd, uchar ** buffer);

#endif
