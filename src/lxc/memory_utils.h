/* SPDX-License-Identifier: LGPL-2.1+ */

#ifndef __LXC_MEMORY_UTILS_H
#define __LXC_MEMORY_UTILS_H

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"
#include "error_utils.h"

#define define_cleanup_function(type, cleaner)           \
	static inline void cleaner##_function(type *ptr) \
	{                                                \
		if (*ptr)                                \
			cleaner(*ptr);                   \
	}

#define call_cleaner(cleaner) \
	__attribute__((__cleanup__(cleaner##_function))) __attribute__((unused))

#define close_prot_errno_disarm(fd) \
	if (fd >= 0) {              \
		int _e_ = errno;    \
		close(fd);          \
		errno = _e_;        \
		fd = -EBADF;        \
	}

#define close_prot_errno_move(fd, new_fd) \
	if (fd >= 0) {                       \
		int _e_ = errno;             \
		close(fd);                   \
		errno = _e_;                 \
		fd = new_fd;                 \
		new_fd = -EBADF;	     \
	}

static inline void close_prot_errno_disarm_function(int *fd)
{
       close_prot_errno_disarm(*fd);
}
#define __do_close call_cleaner(close_prot_errno_disarm)

define_cleanup_function(FILE *, fclose);
#define __do_fclose call_cleaner(fclose)

define_cleanup_function(DIR *, closedir);
#define __do_closedir call_cleaner(closedir)

#define free_disarm(ptr)                    \
	({                                  \
		if (!IS_ERR_OR_NULL(ptr)) { \
			free(ptr);          \
			ptr = NULL;         \
		}                           \
	})

static inline void free_disarm_function(void *ptr)
{
	free_disarm(*(void **)ptr);
}
#define __do_free call_cleaner(free_disarm)

static inline void free_string_list(char **list)
{
	if (list && !IS_ERR(list)) {
		for (int i = 0; list[i]; i++)
			free(list[i]);
		free_disarm(list);
	}
}
define_cleanup_function(char **, free_string_list);
#define __do_free_string_list call_cleaner(free_string_list)

static inline void *memdup(const void *data, size_t len)
{
	void *copy = NULL;

	copy = len ? malloc(len) : NULL;
	return copy ? memcpy(copy, data, len) : NULL;
}

#define zalloc(__size__) (calloc(1, __size__))

#define free_move_ptr(a, b)          \
	({                           \
		free(a);             \
		(a) = move_ptr((b)); \
	})

#define close_move_fd(a, b)         \
	({                          \
		close(a);           \
		(a) = move_fd((b)); \
	})

#define close_equal(a, b)             \
	({                            \
		if (a >= 0 && a != b) \
			close(a);     \
		if (b >= 0)           \
			close(b);     \
		a = b = -EBADF;       \
	})

#define free_equal(a, b)         \
	({                       \
		if (a != b)      \
			free(a); \
		free(b);         \
		a = b = NULL;    \
	})

#endif /* __LXC_MEMORY_UTILS_H */
