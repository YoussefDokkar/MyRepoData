#ifndef HEALTHMONITOR_H
#define HEALTHMONITOR_H



void ACL_initialize();
void SysUartInit();
void BLE_Init();

void GetForce(float x,float y,float z);
void GetTempValue();
void sendFloatBLE(float value);

//functions needed by ACL PMOD and BLE PMOD
void EnableCaches();
void DisableCaches();

#endif // HEALTHMONITOR_H