#ifndef __VALUE_VALIDATORS__
#define __VALUE_VALIDATORS__


int FloatAngleFloatValidator(void*context ,void *arg,  char *str, int len);
int LongLatFloatValidator(void*context ,void *arg,  char *str, int len);
int WindSensorTypeUIntValidator(void*context ,void *arg,  char *str, int len);
int SensorInputUCharValidator(void*context ,void *arg,  char *str, int len);
int TimeZoneFloatValidator(void*context ,void *arg,  char *str, int len);

#endif