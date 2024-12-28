#include "gnf.h"

static GNF_Buffer_t storages[GNF_FD_MAX];
static uint8_t separator = '\n';

static void *_memdup(const void *src, size_t size)
{
	void *dst;

	dst = malloc(size);
	if (dst)
		dst = memcpy(dst, src, size);
	return (dst);
}

static ssize_t _updateSize(GNF_Buffer_t *buffer, GNF_Buffer_t *field)
{
	ssize_t size;

	size = 0;
	if (buffer->data)
	{
		while (size < buffer->size && ((uint8_t *)buffer->data)[size] != separator)
			size++;
		if (size != buffer->size)
			buffer->size = size + 1;
		field->size += buffer->size;
	}
	return (size);
}

static int _readField(int fd, GNF_Buffer_t *field, GNF_Buffer_t *storage, ssize_t readSize)
{
	GNF_Buffer_t buffer;

	buffer = *storage;
	if (_updateSize(&buffer, field) == readSize)
	{
		readSize *= 2;
		if (readSize < GNF_MIN_READ_SIZE)
			readSize = GNF_MIN_READ_SIZE;
		storage->data = malloc(readSize);
		if (storage->data == NULL)
			return (GNF_ALLOC_ERROR);
		storage->size = read(fd, storage->data, readSize);
		if (storage->size < 0)
			return (free(buffer.data), GNF_READ_ERROR);
		int error = _readField(fd, field, storage, readSize);
		if (error)
			return (free(buffer.data), error);
	}
	else
	{
		storage->size -= buffer.size;
		storage->data = _memdup(buffer.data + buffer.size, storage->size);
		if (storage->data == NULL)
			return (free(buffer.data), GNF_ALLOC_ERROR);
		if (field->size == 0)
			return (free(buffer.data), GNF_EOF);
		field->data = malloc(field->size + 1);
		if (field->data == NULL)
			return (free(buffer.data), GNF_ALLOC_ERROR);
		((uint8_t *)field->data)[field->size] = '\0';
		field->data += field->size;
	}
	field->data = memcpy(field->data - buffer.size, buffer.data, buffer.size);
	free(buffer.data);
	return (EXIT_SUCCESS);
}

void gnf_separator(uint8_t newSeparator)
{
	separator = newSeparator;
}

void gnf_clear(int fd)
{
	if (0 <= fd && fd < GNF_FD_MAX)
	{
		free(storages[fd].data);
		storages[fd].data = NULL;
		storages[fd].size = 0;
	}
}

int gnf(int fd, void **field, ssize_t *size)
{
	GNF_Buffer_t fieldBuffer = {NULL, 0};

	if (fd < 0 || GNF_FD_MAX <= fd)
		return (GNF_FD_ERROR);
	int error = _readField(fd, &fieldBuffer, &storages[fd], storages[fd].size);
	if (error)
		return (gnf_clear(fd), error);
	if (field)
		*field = fieldBuffer.data;
	else
		free(fieldBuffer.data);
	if (size)
		*size = fieldBuffer.size;
	return (EXIT_SUCCESS);
}