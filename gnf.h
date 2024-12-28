/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_gnf.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nlaerema <nlaerema@student.42lehavre.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/05 10:58:17 by nlaerema          #+#    #+#             */
/*   Updated: 2023/11/30 22:20:55 by nlaerema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_GNF_H
#define FT_GNF_H

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

typedef struct
{
	void *data;
	ssize_t size;
} GNF_Buffer_t;

int gnf(int fd, void **data, ssize_t *size);
void gnf_clear(int fd);
void gnf_separator(uint8_t newSeparator);

#endif
