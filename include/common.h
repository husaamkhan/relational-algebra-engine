#pragma once

#include <stdlib.h>
#include <stdio.h>

/* ==================================================
 * LOGGING
 * ================================================== */

#define LOG(fmt, ...)     printf(fmt "\n", ##__VA_ARGS__)
#define LOG_ERR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
    #define LOG_DBG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define LOG_DBG(fmt, ...) ((void)0)
#endif

/* ==================================================
 * UTILITY FUNCTIONS
 * ================================================== */

/*
 * Rounds a value up to the next multiple of the given alignment.
 *
 * Input:
 *   alignment - Required alignment.
 *   value     - Value to align.
 *
 * Output:
 *   None.
 *
 * Returns:
 *   The value rounded up to the next multiple of alignment.
 */
static inline size_t align_up(size_t alignment, size_t value)
{
	size_t r = alignment - (value % alignment); 
	if (r != 0)
	{
		value += r;
	}

	return value;
}

/*
 * Reads the contents of a file into a newly allocated buffer.
 *
 * Input:
 *   in - File stream to read from.
 *
 * Output:
 *   size_out - Set to the number of bytes read.
 *
 * Returns:
 *   A pointer to the allocated file contents, or NULL if reading fails.
 */
static inline char *read_file(FILE *in, size_t *size_out)
{
	fseek(in, 0, SEEK_END);
	*size_out = ftell(in);
	fseek(in, 0, SEEK_SET);

	char *buffer = malloc(*size_out);

	if (!fread(buffer, 1, *size_out, in))
	{
		LOG_ERR("Couldn't read file");
		free(buffer);
		return NULL;
	}

	return buffer;
}

/* ==================================================
 * DATA TYPES
 * ================================================== */

typedef struct
{
	char *base;
	size_t used;
	size_t capacity;
	size_t alignment;
} Arena;

/* ==================================================
 * ARENA
 * ================================================== */

/*
 * Creates an empty arena.
 *
 * Input:
 *   initial_capacity - Initial capacity of the arena in bytes.
 *
 * Output:
 *   None.
 *
 * Returns:
 *   A newly initialized Arena.
 */
static inline Arena arena_create(size_t initial_capacity)
{
	return (Arena) {
		.base 		= NULL,
		.used 		= 0,
		.capacity 	= initial_capacity,
		.alignment 	= 1
	};
}


/*
 * Allocates memory from an arena.
 *
 * Input:
 *   arena     - Arena to allocate from.
 *   size      - Number of bytes to allocate.
 *   alignment - Required alignment of the allocation.
 *
 * Output:
 *   The arena's used space is advanced by the allocation size.
 *   The arena may be resized if it does not have enough capacity.
 *
 * Returns:
 *   A pointer to the allocated memory.
 */
static inline void *arena_alloc(Arena *arena, size_t size, size_t alignment)
{
	if (arena->used + size > arena->capacity)
	{
		arena->capacity *= 2;
		arena->base = realloc(arena->base, arena->capacity);

		if (arena->base == NULL)
		{
			LOG_ERR("Arena realloc failed!");
			exit(EXIT_FAILURE);
		}
	}

	arena->used += size;
	arena->used = align_up(alignment, arena->used);
	void *ptr = arena->base + arena->used;
	return ptr;
}
