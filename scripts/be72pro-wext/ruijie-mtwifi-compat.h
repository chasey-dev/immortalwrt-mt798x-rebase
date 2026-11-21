#ifndef RUIJIE_MTWIFI_COMPAT_H
#define RUIJIE_MTWIFI_COMPAT_H

#include <linux/string.h>

/* Linux 6.12 removed strlcpy. Preserve the old driver's return contract. */
static inline size_t ruijie_strlcpy(char *dest, const char *src, size_t size)
{
	size_t length = strlen(src);

	if (size) {
		size_t count = length < size - 1 ? length : size - 1;

		memcpy(dest, src, count);
		dest[count] = '\0';
	}

	return length;
}

#define strlcpy ruijie_strlcpy

#endif
