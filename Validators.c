#include "Validators.h"
#include <stdlib.h>




int FloatAngleFloatValidator(void*context ,void *arg,  char *str, int len){
  float* valptr = (float*)arg;
  if(*valptr>360 || *valptr<0) return -1;
  return  0;
}

int LongLatFloatValidator(void*context ,void *arg,  char *str, int len){
 float* valptr = (float*)arg;
  if(*valptr>180 || *valptr<-180) return -1;
  return  0;
}

int WindSensorTypeUIntValidator(void*context ,void *arg,  char *str, int len){
  unsigned int* valptr = ( unsigned int*)arg;
  if(*valptr<0 || *valptr>1) return -1;
  return  0;
}

int SensorInputUCharValidator(void*context ,void *arg,  char *str, int len){
  unsigned char* valptr = ( unsigned char*)arg;
  if(*valptr>8) return -1;
  return  0;
}

int TimeZoneFloatValidator(void*context ,void *arg,  char *str, int len){
   float* valptr = (  float*)arg;
  if(*valptr<-11 || *valptr>14) return -1;
  return  0;
}


