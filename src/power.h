#ifndef POWER_H_
#define POWER_H_
#include <stdint.h>

void pwrInit(void);
void sysSleep(void);
void sysShutdown(void);
uint16_t updateBattery(void);

#endif /* POWER_H_ */
