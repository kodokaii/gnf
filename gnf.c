#include "gnf.h"

static GNFBuffer g_storage[GNF_FD_MAX];
static uint8_t g_separator = '\n';

static int _read_field(int fd, GNFBuffer *field_output, GNFBuffer *overflow_output, ssize_t read_size);

static void *_memdup(const void *src, size_t size) {
	void *dst;

	dst = malloc(size);
	if (dst)
		dst = memcpy(dst, src, size);
	return (dst);
}

static void _free_buffer(GNFBuffer *buffer) {
	free(buffer->data);
	buffer->data = NULL;
	buffer->size = 0;
}

static ssize_t _find_separator(GNFBuffer *buffer) {
	ssize_t i;

	if (buffer->data) {
		for (i = 0; i < buffer->size; i++) {
			if (buffer->data[i] == g_separator)
				return (i);
		}
	}
	return (-1);
}

static void _update_field_size(GNFBuffer *field_output, ssize_t buffer_size, ssize_t separator_idx) {
	if (0 <= separator_idx)
		field_output->size += separator_idx + 1;
	else
		field_output->size += buffer_size;
}

static int _save_overflow(GNFBuffer *overflow_output, GNFBuffer *buffer, ssize_t separator_idx) {
	if (0 <= separator_idx) {
		overflow_output->size = buffer->size - (separator_idx + 1);
		overflow_output->data = _memdup(buffer->data + buffer->size - overflow_output->size, overflow_output->size);
		if (overflow_output->data == NULL)
			return (GNF_ALLOC_ERROR);
	} else {
		overflow_output->size = 0;
		overflow_output->data = NULL;
	}
	return (EXIT_SUCCESS);
}

static int _final_setup(GNFBuffer *field_output, GNFBuffer *overflow_output, GNFBuffer *buffer, ssize_t separator_idx) {
	int error;

	error = _save_overflow(overflow_output, buffer, separator_idx);
	if (error)
		return (error);

	// Remove overflow from the original buffer size
	buffer->size -= overflow_output->size;

	if (field_output->size == 0)
		return (GNF_EOF);

	field_output->data = malloc(field_output->size + 1);
	if (field_output->data == NULL)
		return (GNF_ALLOC_ERROR);
	field_output->data[field_output->size] = '\0';


	// Move pointer to the end for backward buffer copy
	field_output->data += field_output->size;
	return (EXIT_SUCCESS);
}

static ssize_t _new_read_size(ssize_t read_size) {
	read_size *= 2;
	if (read_size < GNF_MIN_READ_SIZE)
		read_size = GNF_MIN_READ_SIZE;
	return (read_size);
}

static int _continue_reading(int fd, GNFBuffer *field_output, GNFBuffer *overflow_output, ssize_t read_size) {
	read_size = _new_read_size(read_size);

	overflow_output->data = malloc(read_size);
	if (overflow_output->data == NULL)
		return (GNF_ALLOC_ERROR);

	overflow_output->size = read(fd, overflow_output->data, read_size);
	if (overflow_output->size < 0)
		return (GNF_READ_ERROR);

	return (_read_field(fd, field_output, overflow_output, read_size));
}

static int _read_field(int fd, GNFBuffer *field_output, GNFBuffer *overflow_output, ssize_t read_size) {
	GNFBuffer	buffer;
	int			error;
	ssize_t		separator_idx;

	buffer = *overflow_output;
	separator_idx = _find_separator(&buffer);
	_update_field_size(field_output, buffer.size, separator_idx);

	if (buffer.size == read_size && separator_idx < 0) {
		error = _continue_reading(fd, field_output, overflow_output, read_size);
	} else {
		error = _final_setup(field_output, overflow_output, &buffer, separator_idx);
	}
	if (error)
		return (_free_buffer(&buffer), error);

	// Copy saved buffers to the allocated field buffer and shift pointer back
	field_output->data = memcpy(field_output->data - buffer.size, buffer.data, buffer.size);
	_free_buffer(&buffer);

	return (EXIT_SUCCESS);
}

void gnf_separator(uint8_t new_separator) {
	g_separator = new_separator;
}

void gnf_clear(int fd) {
	if (0 <= fd && fd < GNF_FD_MAX)
		_free_buffer(&g_storage[fd]);
}

int gnf(int fd, uint8_t **field, ssize_t *size) {
	GNFBuffer field_output = {NULL, 0};

	if (fd < 0 || GNF_FD_MAX <= fd)
		return (GNF_FD_ERROR);

	int error = _read_field(fd, &field_output, &g_storage[fd], g_storage[fd].size);
	if (error)
		return (gnf_clear(fd), error);

	*field = field_output.data;
	if (size)
		*size = field_output.size;

	return (EXIT_SUCCESS);
}