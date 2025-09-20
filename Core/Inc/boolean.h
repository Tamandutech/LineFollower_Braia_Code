// The bool type from <stdbool.h> is not supported.
// See: https://community.st.com/t5/stm32cubemonitor-mcus/cubemonitor-doesn-t-understand-bool-types/td-p/304985

/*
 * boolean.h
 *
 *  Created on: Sep 17, 2025
 *      Author: Kelvin Novais
 */

#ifndef INC_BOOLEAN_H_
#define INC_BOOLEAN_H_

typedef enum _boolean {
	false,
	true
} boolean;

#endif /* INC_BOOLEAN_H_ */
