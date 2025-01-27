#include "buffer.h"

void buffer_write(volatile float *buffer, volatile uint16_t* index, float value, uint16_t size)
{
	/* write value to current index */
	buffer[*index] = value;

	/* increment index */
	(*index)++;

	/* reset index if we reach the end */
	if (*index >= size)
	{
		*index = 0;
	}
}

float buffer_read(volatile float *buffer, uint16_t index, uint16_t size)
{
  /* ensure index is inside bounds */
  index = index % size;

  /* return value at index */
  return buffer[index];
}
