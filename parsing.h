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


#ifndef PARSING_H
#define PARSING_H

#include "datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * parse an input entry in the given buffer
 */
struct ValueListNode *parseInput(unsigned char *buffer);


/**
 * parse the buffer from a UVR1611
 * 
 * \return NULL on error, a sensor node else
 */
struct SystemState *parseUVR1611(unsigned char *buffer);

#ifdef __cplusplus
}
#endif

#endif /* PARSING_H */