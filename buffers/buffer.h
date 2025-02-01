#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

/*!
 * @brief Write to circular buffer
 *
 * writes a value to the specified circular buffer and
 * updates index. resets index to 0 when reaching buffer size.
 *
 * @param buffer - pointer to the circular buffer
 * @param index - pointer to the current write index
 * @param value - value to write into the buffer
 * @param size - size of the buffer
 */
void buffer_write(volatile float *buffer, volatile uint16_t* index, float value, uint16_t size);

/*!
 * @brief Read from circular buffer
 *
 * reads a value from the specified circular buffer at the given index position.
 * the index is wrapped around using the buffer size to maintain circular behavior.
 *
 * @param buffer - pointer to the circular buffer
 * @param index - read position in the buffer
 * @param size - size of the buffer
 * @return float - value read from the buffer at the specified position
 */
float buffer_read(volatile float *buffer, uint16_t index, uint16_t size);

#endif // BUFFER_H
