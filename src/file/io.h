#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stddef.h>

#ifndef READALL_CHUNK
#define READALL_CHUNK 262144
#endif

#define READALL_OK		0  /* Success */
#define READALL_INVALID -1 /* Invalid parameters */
#define READALL_ERROR	-2 /* Stream error */
#define READALL_TOOMUCH -3 /* To much input */
#define READALL_NOMEM	-4 /* Out of memory */

int readall(FILE *in, char **dataptr, size_t* sizeptr);

#endif
