/*
 * DataManager.c
 *
 *  Created on: Jun 3, 2026
 *      Author: Kelvin Novais
 */

#include "DataManager.h"

#if defined(USE_DATA_MANAGER)
/******************************************************************************/
// INCLUDES
#include <string.h>

#define BIGGEST_ELEMENT \
  ((sizeof(Mapping) > sizeof(Mapped) ? sizeof(Mapping) \
                                     : sizeof(Mapped))) // bytes

#define POOL_SIZE (BIGGEST_ELEMENT * POOL_N_POINTS)     // bytes


/******************************************************************************/
// COMPILER MESSAGES/ASSERTS
static_assert(sizeof(Mapping) % 4 == 0,
              "Mapping struct is not a multiple of 4 bytes!");

static_assert(sizeof(Mapped) % 4 == 0,
              "Mapped struct is not a multiple of 4 bytes!");

#pragma message("Using DataManager")

/******************************************************************************/
// VARIABLES
// Private
static unsigned char pool_data[POOL_SIZE] = {0};
static PoolMode      mode                 = POOL_MODE_MAPPED;
static size_t        count                = 0;
static size_t        capacity             = 0;


size_t pool_get_count() { return count; }

size_t pool_get_capacity() {
  if(mode == POOL_MODE_MAPPED)
    return (capacity = sizeof(pool_data) / sizeof(Mapped));
  else
    return (capacity = sizeof(pool_data) / sizeof(Mapping));
}

void pool_reset_to(PoolMode new_mode) {
  mode  = new_mode;
  count = 0;
}

PoolStatus pool_push_mapping(const Mapping *const m) {
  if(mode != POOL_MODE_MAPPING) return POOL_STATUS_INVALID_MODE;

  if(count >= capacity) return POOL_STATUS_FULL;

  unsigned char *dst = pool_data + count * sizeof(Mapping);
  // Use memcpy to avoid alignment issues
  memcpy(dst, m, sizeof(Mapping));
  count++;

  return POOL_STATUS_OK;
}

PoolStatus pool_push_mapped(const Mapped *const m) {
  if(mode != POOL_MODE_MAPPED) return POOL_STATUS_INVALID_MODE;

  if(count >= capacity) return POOL_STATUS_FULL;

  unsigned char *dst = pool_data + count * sizeof(Mapped);
  memcpy(dst, m, sizeof(Mapped));
  count++;

  return POOL_STATUS_OK;
}

PoolStatus pool_get_mapping_data(size_t idx, Mapping *out) {
  if(mode != POOL_MODE_MAPPING) return POOL_STATUS_INVALID_MODE;

  if(idx >= count) return POOL_STATUS_INVALID_INDEX;

  const unsigned char *src = pool_data + idx * sizeof(Mapping);
  memcpy(out, src, sizeof(Mapping));
  return POOL_STATUS_OK;
}

PoolStatus pool_get_mapped_data(size_t idx, Mapped *out) {
  if(mode != POOL_MODE_MAPPED) return POOL_STATUS_INVALID_MODE;

  if(idx >= count) return POOL_STATUS_INVALID_INDEX;

  const unsigned char *src = pool_data + idx * sizeof(Mapped);

  memcpy(out, src, sizeof(Mapped));
  return POOL_STATUS_OK;
}


#endif /* USE_DATA_MANAGER */

       /*
        * Helpful information:
        *
        * https://www.youtube.com/watch?v=OKjOZBaKlOc
        * https://www.youtube.com/watch?v=aROgtACPjjg
        */