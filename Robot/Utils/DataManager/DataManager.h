/*
 * DataManager.h
 *
 *  Created on: Jun 3, 2026
 *      Author: Kelvin Novais
 */


#ifndef UTILS_DATAMANAGER_DATAMANAGER_H_
#define UTILS_DATAMANAGER_DATAMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif


// #define USE_DATA_MANAGER
#if defined(USE_DATA_MANAGER)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Number of points should be pre-allocated
#define POOL_N_POINTS 3000

/******************************************************************************\
IMPORTANT - Keep memory aligned

We are using STM32, which means it can read 32 bits in a cycle
32 bits = 4 bytes = 1 word

Make the struct ALWAYS be a multiple of a word (halfword, word, doubleword)
\******************************************************************************/
typedef struct __attribute__((packed)) _Mapping {
  // FLOATS
  /*
   * 32 bits each float
   */
  // WORD 1
  float x;

  // WORD 2
  float y;

  // WORD 3
  float omega;


  // WORD 4
  /*
   * Can hold up to:       2³⁰ - 1 = 1,073,741,823 µs
   * which is              1,073.7 s
   * which is              17.9 min
   */
  uint32_t timestamp   : 30;
  uint32_t isLeftMark  : 1;
  uint32_t isRightMark : 1;


  // WORD 5
  /*
   * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
   */
  uint32_t leftEncoder : 22;
  /*
   * Must hold up to LastRotatableColor
   */
  uint32_t colorIndex  : 3;
  uint32_t             : 7;


  // WORD 6
  /*
   * Must hold up to TRACK_MAX_PULSES: 2²² - 1 = 4,194,303 pulses
   */
  uint32_t rightEncoder : 22;
  uint32_t              : 10;

} Mapping;

typedef struct __attribute__((packed)) _Mapped {
  // WORD 1
  uint32_t encoder : 22;
  uint32_t motor   : 10;

  // WORD 2
  uint32_t vacuum     : 10;
  uint32_t colorIndex : 3;
  uint32_t            : 19;
} Mapped;

typedef enum _PoolMode { POOL_MODE_MAPPING = 0, POOL_MODE_MAPPED } PoolMode;

typedef enum _PoolStatus {
  POOL_STATUS_OK = 0,
  POOl_STATUS_ERROR,

  POOL_STATUS_FULL,
  POOL_STATUS_INVALID_MODE,
  POOL_STATUS_INVALID_INDEX,

  POOL_STATUS_UNKNOWN
} PoolStatus;

void pool_reset_to(PoolMode new_mode);

size_t pool_get_count();
size_t pool_get_capacity();

PoolStatus pool_push_mapping(const Mapping *const mapping);
PoolStatus pool_push_mapped(const Mapped *const mapped);

////////////////////////////////////////////////////////////////////////////////
PoolStatus pool_get_mapping_data(size_t index, Mapping *destination);
PoolStatus pool_get_mapped_data(size_t index, Mapped *destination);

// OR

// const Mapped *pool_get_mapped_ptr(size_t index);
// const Mapped *pool_get_mapping_ptr(size_t index);
////////////////////////////////////////////////////////////////////////////////

#endif /* USE_DATA_MANAGER */

#ifdef __cplusplus
}
#endif

#endif /* UTILS_DATAMANAGER_DATAMANAGER_H_ */

// Franz Schubert: Violin Sonata in D Major, D. 384: III. Allegro vivace
