#ifndef __SERIALIZATION__
#define __SERIALIZATION__

#include <stddef.h>

typedef   void *SerializationInfo_t[][4];

#define GENERATE_SERIALIZABLE(NAME, TYPE, SIZE, CLASSTYPE) {#NAME, (void*)TYPE, (void*)offsetof(CLASSTYPE,NAME), (void*)SIZE},
#define SERIALIZATION_START(NAME,NAMESPACE, STRUCTTYPE) {NAME,NAMESPACE "_",(void*)0,(void*)sizeof(STRUCTTYPE)}
#define SERIALIZATION_END() {(void*)0,(void*)0,(void*)0,(void*)0}

#define PASTER(x, y) x##_##y
#define GENERATE_ENUM(NAME, TYPE, SIZE, NAMESPACE) PASTER(NAMESPACE, NAME),
#define GENERATE_SER_GENERIC(NAME, TYPE, SIZE, CLASSTYPE) GENERATE_##TYPE(NAME, SIZE)


typedef enum{
   SER_TYPE_CHAR,
   SER_TYPE_UCHAR ,
   SER_TYPE_SHORT, 
   SER_TYPE_USHORT, 
   SER_TYPE_INT, 
   SER_TYPE_UINT, 
   SER_TYPE_FLOAT, 
   SER_TYPE_DOUBLE, 
   SER_TYPE_STRING,
 
   SER_TYPE_BOOL, 
   SER_TYPE_PASSWORD,  
   SER_TYPE_TIME, 
   SER_TYPE_DATE, 
   SER_TYPE_DATETIME,
   SER_TYPE_IPADDR,  
   SER_TYPE_PRIM_COUNT, //count of primitive types


   SER_TYPE_CHAR_ARRAY=SER_TYPE_PRIM_COUNT, 
   SER_TYPE_UCHAR_ARRAY, 
   SER_TYPE_SHORT_ARRAY, 
   SER_TYPE_USHORT_ARRAY, 
   SER_TYPE_INT_ARRAY, 
   SER_TYPE_UINT_ARRAY, 
   SER_TYPE_FLOAT_ARRAY, 
   SER_TYPE_DOUBLE_ARRAY, 
   SER_TYPE_TOTAL_COUNT//count of all types
 

}SerType_t;


#define GENERATE_SER_TYPE_CHAR(NAME, SIZE) signed char NAME;
#define GENERATE_SER_TYPE_UCHAR(NAME, SIZE) unsigned char NAME;
#define GENERATE_SER_TYPE_INT(NAME, SIZE) signed int NAME;
#define GENERATE_SER_TYPE_UINT(NAME, SIZE) unsigned int NAME;
#define GENERATE_SER_TYPE_SHORT(NAME, SIZE) signed short  NAME;
#define GENERATE_SER_TYPE_USHORT(NAME, SIZE) unsigned short NAME;
#define GENERATE_SER_TYPE_FLOAT(NAME, SIZE) float NAME;
#define GENERATE_SER_TYPE_DOUBLE(NAME, SIZE) double NAME;

#define GENERATE_SER_TYPE_CHAR_ARRAY(NAME, SIZE) signed char NAME[SIZE];
#define GENERATE_SER_TYPE_UCHAR_ARRAY(NAME, SIZE) unsigned char NAME[SIZE];
#define GENERATE_SER_TYPE_INT_ARRAY(NAME, SIZE) signed int NAME[SIZE];
#define GENERATE_SER_TYPE_UINT_ARRAY(NAME, SIZE) unsigned int NAME[SIZE];
#define GENERATE_SER_TYPE_SHORT_ARRAY(NAME, SIZE) signed short  NAME[SIZE];
#define GENERATE_SER_TYPE_USHORT_ARRAY(NAME, SIZE) unsigned short  NAME[SIZE];
#define GENERATE_SER_TYPE_FLOAT_ARRAY(NAME, SIZE) float NAME[SIZE];
#define GENERATE_SER_TYPE_DOUBLE_ARRAY(NAME, SIZE) double NAME[SIZE];

#define GENERATE_SER_TYPE_PASSWORD(NAME, SIZE) char NAME[SIZE];
#define GENERATE_SER_TYPE_STRING(NAME, SIZE) char NAME[SIZE];
#define GENERATE_SER_TYPE_BOOL(NAME, SIZE) char NAME;



int FindParameterInfo(const SerializationInfo_t info, char *str , int len, int *ParType, int *ParOffset, int *ParSize);
int GetParameterInfo(const SerializationInfo_t info, int Index, char **name, int *ParType, int *ParOffset, int *ParSize);
unsigned int StructureSize(const SerializationInfo_t info);
#endif
