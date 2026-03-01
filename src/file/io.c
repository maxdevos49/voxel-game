#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"

int readall(FILE *in, char **dataptr, size_t *sizeptr) {
	assert(in != NULL);
	assert(dataptr != NULL);
	assert(sizeptr != NULL);

	if (in == NULL || dataptr == NULL || sizeptr == NULL) {
		return READALL_INVALID;
	}

	if (ferror(in)) {
		return READALL_ERROR;
	}

	char *data = NULL, *temp = NULL;
	size_t size = 0;
	size_t capacity = 0;

	while (1) {
		if (size + READALL_CHUNK + 1 > capacity) {
			capacity = size + READALL_CHUNK + 1;

			// Integer overflow check
			if (capacity <= size) {
				free(data);
				data = NULL;
				return READALL_TOOMUCH;
			}

			temp = realloc(data, capacity);
			if (temp == NULL) {
				perror("realloc");
				free(data);
				data = NULL;
				return READALL_NOMEM;
			}

			data = temp;
			temp = NULL;
		}

		size_t bytes = fread(data + size, 1, READALL_CHUNK, in);
		if (bytes == 0) {
			break;
		}

		size += bytes;
	}

	if (ferror(in)) {
		perror("fread");
		free(data);
		data = NULL;
		return READALL_ERROR;
	}

	// Shrinkwrap allocated buffer
	temp = realloc(data, size + 1);
	if (temp == NULL) {
		perror("realloc");
		free(data);
		data = NULL;
		return READALL_NOMEM;
	}

	data = temp;
	temp = NULL;
	data[size] = '\0';

	*dataptr = data;
	*sizeptr = size;

	return READALL_OK;
}
