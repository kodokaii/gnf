#pragma once

#define GNF_MIN_READ_SIZE 4096
#define GNF_FD_MAX 1024

#define GNF_EOF 1
#define GNF_READ_ERROR 2
#define GNF_ALLOC_ERROR 3
#define GNF_FD_ERROR 4

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
	uint8_t	*data;
	ssize_t	size;
} GNFBuffer;

int gnf(int fd, uint8_t **data, ssize_t *size);
void gnf_clear(int fd);
void gnf_separator(uint8_t new_separator);