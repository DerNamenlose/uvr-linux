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

#include "datatypes.h"

struct SystemState *initSystemState()
{
    struct SystemState *ptr;
    ptr = malloc(sizeof(struct SystemState));
    if (ptr) {
        ptr->inputs = NULL;
        ptr->outputs = NULL;
    }
}

/**
 * clean up a system state object
 */
void freeSystemState(struct SystemState *state)
{
    if (state) {
        if (state->inputs) {
            struct ValueListNode *ptr;
            ptr = state->inputs;
            while (ptr != NULL) {
                struct ValueListNode *tmp;
                tmp = ptr;
                ptr = ptr->next;
                free(tmp);
            }
        }
        free(state);
    }
}

struct ValueListNode *createValueListNode()
{
    struct ValueListNode *tmp;
    tmp = malloc(sizeof(struct ValueListNode));
    if (tmp != NULL) {
        tmp->next = NULL;
    }
    return tmp;
}

