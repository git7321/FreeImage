/*
 * jinclude.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2022-2023, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file exists to provide a single place to fix any problems with
 * including the wrong system include files.  (Common problems are taken
 * care of by the standard jconfig symbols, but on really weird systems
 * you may have to edit this file.)
 *
 * NOTE: this file is NOT intended to be included by applications using the
 * JPEG library.  Most applications need only include jpeglib.h.
 */

#ifndef __JINCLUDE_H__
#define __JINCLUDE_H__

#include "jconfig.h"
#include "jconfigint.h"
#define JCONFIG_INCLUDED

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#endif /* JINCLUDE_H */
