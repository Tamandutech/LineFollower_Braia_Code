/*
 * application.h
 *
 *  Created on: Nov 3, 2025
 *      Author: Kelvin Novais
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void setup(void);
EXTERNC void loop(void);

#endif /* APPLICATION_H_ */
