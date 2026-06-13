/*
 * ErrorHandler.h
 *
 *  Created on: Jun 3, 2026
 *      Author: Kelvin Novais
 */

#ifndef UTILS_ERRORHANDLER_ERRORHANDLER_H_
#define UTILS_ERRORHANDLER_ERRORHANDLER_H_

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

EXTERN_C void emergency_stop();

EXTERN_C void on_hard_fault();
EXTERN_C void on_memory_management_fault();
EXTERN_C void on_bus_fault();
EXTERN_C void on_usage_fault();

#undef EXTERN_C

#endif /* UTILS_ERRORHANDLER_ERRORHANDLER_H_ */

// Prokofiev: Romeo and Juliet Suite
