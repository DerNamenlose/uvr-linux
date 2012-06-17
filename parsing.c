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
        switch (node->value.valueType) {
            case UNUSED:
                break;
            case DIGITAL:
                if (buffer[1] & 0x80 != 0) {
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
    int i;
    struct ValueListNode *head;
    struct ValueListNode *last;
    head = last = NULL;
    for (i = 0; i < number; ++i) { 
        struct ValueListNode *node;
        node = parseInput(buffer+2*i+1); // every input takes 2 bytes and the first one is at offset 1
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
            return -1;
        }
    }
    state->inputs = head;
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
    if (state != NULL && buffer != NULL) {
        struct ValueListNode *currentLast = NULL;
        int i;
        for (i = 0; i < number; ++i) {
            unsigned char currentByte;
            struct ValueListNode *node;
            node = createValueListNode();
            if (node == NULL) {
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
    return 0;
}

/**
 * parse the buffer from a UVR1611
 * 
 * \return NULL on error, a system state structure else
 */
struct SystemState *parseUVR1611(unsigned char *buffer)
{
    struct SystemState *state;
    state = initSystemState();
    if (state != NULL) {
        if (parseInputs(state, buffer, 16) != 0) {
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
    return state;
}
