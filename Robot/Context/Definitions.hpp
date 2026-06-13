/*
 * Definitions.hpp
 *
 *  Created on: May 17, 2026
 *      Author: Kelvin Novais
 */

#ifndef CONTEXT_DEFINITIONS_HPP_
#define CONTEXT_DEFINITIONS_HPP_

#include <cstdint>

enum Side : uint8_t {
  Left = 0,
  Right,

  N_SIDES_
};

enum Axis : uint8_t {
  X = 0,
  Y,
  Z,

  N_AXES_
};

enum RotationAxis : uint8_t {
  Pitch = 0,
  Row,
  Yaw,

  N_ROTATION_AXES_
};

/*
    Axis
                                  Z
                                  ⋮
                                  ⋮      Y
                                  ⋮   ⋰
                                  ⋮ ⋰
                                   O …………………………… X
                                ⋰ 
                              ⋰


    Rotational Axis
                                 Yaw
                                  ⤻
                                  ⋮
                                  ⋮      Row
                                  ⋮   ⋰
                                  ⋮ ⋰
                                   O …………………………… Pitch
                                ⋰
                              ⋰

*/

#endif /* CONTEXT_DEFINITIONS_HPP_ */
