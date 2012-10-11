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
#include <stdlib.h>
#include <errno.h>

#include "parsing.h"
#include "logging.h"

#define GETBIT(byte, bit) ((byte & (0x01 << bit)) >> bit)

struct ValueListNode *parseInput(unsigned char *buffer)
{
    struct ValueListNode *node;
    int value;
    node = createValueListNode();
    if (node != NULL) {
        node->value.valueType = (buffer[1] & 0x70) >> 4;  // bits 6, 5, and 4 of the high byte indicate the sensor type
        value = buffer[1] & 0x0F;
        value <<= 8;
        value += buffer[0];
	log_output(LOG_DEBUG, "Binary value: %x\n", value);
	if (value & 0x0800) {
		// negative temperature
		value = -(0x1000 - value);
	}
        switch (node->value.valueType) {
            case UNUSED:
                break;
            case DIGITAL:
                if ((buffer[1] & 0x80) != 0) {
                    node->value.value.enabled = 1;
                }
                else {
                    node->value.value.enabled = 0;
                }
                break;
            case TEMPERATURE:
                node->value.value.temperature = ((float)value) / 10.0;
                break;
            case FLOW:
                node->value.value.flow = value * 4;
                break;
            default:
                log_output(LOG_ERR, "Unsupported sensor type so far\n");
                free(node);
                node = NULL;
                break;
        }
    }
    return node;
}

int parseInputs(struct SystemState *state, unsigned char *buffer, unsigned int number)
{
    log_output(LOG_DEBUG, "Parsing %d inputs\n", number);
    if (state != NULL && buffer != NULL) {
        unsigned int i;
        struct ValueListNode *head;
        struct ValueListNode *last;
        head = last = NULL;
        for (i = 0; i < number; ++i) { 
            struct ValueListNode *node;
            node = parseInput(buffer+2*i); // every input takes 2 bytes
            if (node != NULL) {
                node->value.valueID = i+1;  // inputs are 1-based
                if (head == NULL) {
                    head = node;
                }
                if (last != NULL) {
                    last->next = node;
                }
                last = node;
            }
            else {
                log_output(LOG_ERR, "Could not create new list node instance\n");
                return -1;
            }
        }
        state->inputs = head;
    }
    log_output(LOG_DEBUG, "Done parsing inputs\n");
    return 0;
}

/**
 * parse the output information from the buffer
 * 
 * \param state the system state where to put the outputs
 * \param buffer the buffer where the binary encoding is located
 * \param number the number of outputs to parse
 */
int parseOutputs(struct SystemState *state, unsigned char *buffer, unsigned int number)
{
    log_output(LOG_DEBUG, "Parsing %u outputs\n", number);
    if (state != NULL && buffer != NULL) {
        struct ValueListNode *currentLast = NULL;
        unsigned int i;
        for (i = 0; i < number; ++i) {
            unsigned char currentByte;
            struct ValueListNode *node;
            log_output(LOG_DEBUG, "Parsing input %u\n", i);
            node = createValueListNode();
            if (node == NULL) {
                log_output(LOG_ERR, "Could not create new list node instance\n");
                return -1;
            }
            currentByte = buffer[(int)(i / 8)]; // get the correct byte in the buffer containing our output bit
            node->value.valueType = DIGITAL;
            node->value.valueID = i+1; // outputs are 1-based
            node->value.value.enabled = GETBIT(currentByte, i % 8);
            if (state->outputs == NULL) {
                state->outputs = node;
            }
            else {
                currentLast->next = node;
            }
            currentLast = node;
        }
    }
    log_output(LOG_DEBUG, "Done parsing outputs\n");
    return 0;
}

/**
 * parse the rotation entries in the buffer
 */
int parseRotations(struct SystemState *state, unsigned char *buffer, unsigned int number)
{
    // TODO implement
    return 0;
}

/**
 * parse the heat registers
 */
int parseHeat(struct SystemState *state, unsigned char *buffer, unsigned int number)
{
    log_output(LOG_DEBUG, "Parsing %d heat registers\n", number);
    if (state != NULL && buffer != NULL)
    {
        struct ValueListNode *currentLast = NULL;
        unsigned int i;
        for (i = 0; i < number; ++i) {
            if (GETBIT(buffer[0], i)) {
                // only parse the register if the corresponding counter is enabled
                int value = 0;
                struct ValueListNode *node;
                node = createValueListNode();
                if (node == NULL) {
                    log_output(LOG_ERR, "Could not create new list node instance\n");
                    return -1;
                }
                // current value
                value = ((int)(buffer[i*4+1+3])) << 16;
                value += ((int)buffer[i*4+1+2]) << 8;
                value += buffer[i*4+1+1];
                if (buffer[i*4+1+3] > 127) {
                    value = -(0x01000000 - value);
                }
                value *= 10;
                if (buffer[i*4+1+3] > 127) {
                    value -= ((int)buffer[i*4+1]) * 10 / 256;
                }
                else {
                    value += ((int)buffer[i*4+1]) * 10 / 256;
                }
                node->value.valueID = i+1;
                node->value.valueType = HEAT;
                node->value.value.heat.current = ((float)value) / 100;
                // the total value
                value = (((unsigned int)buffer[i*4+1+7]) << 8) + buffer[i*4+1+6];
                node->value.value.heat.total = value * 1000; // the high bytes give the value in MWh, we save kWh
                value = (((unsigned int)buffer[i*4+1+5]) << 8) + buffer[i*4+1+4];
                node->value.value.heat.total += ((float)value) / 10;
                if (state->heatRegisters == NULL) {
                    state->heatRegisters = node;
                }
                else {
                    currentLast->next = node;
                }
                currentLast = node;
            }
        }
    }
    log_output(LOG_DEBUG, "Done parsing heat registers\n");
    return 0;
}

/**
 * parse the buffer from a UVR1611
 * 
 * \return NULL on error, a system state structure else
 */
struct SystemState *parseUVR1611(unsigned char *buffer)
{
    log_output(LOG_DEBUG, "Parsing message from UVR1611\n");
    struct SystemState *state;
    state = initSystemState();
    if (state != NULL) {
        if (parseInputs(state, buffer+1, 16) != 0) {
            log_output(LOG_ERR, "Could not parse input list.\n");
            freeSystemState(state);
            state = NULL;
        }
    }
    if (state != NULL) {
        if (parseOutputs(state, buffer+1+32, 13) != 0) {
            log_output(LOG_ERR, "Could not parse input list.\n");
            freeSystemState(state);
            state = NULL;
        }
    }
    if (state != NULL) {
        if (parseHeat(state, buffer+1+38, 2) != 0) {
            log_output(LOG_ERR, "Could not parse input list.\n");
            freeSystemState(state);
            state = NULL;
        }
    }
    return state;
}
