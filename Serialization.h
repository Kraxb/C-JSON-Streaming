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


#define GENERATE_SER_TYPE_CHAR(NAME, SIZE) char NAME;
#define GENERATE_SER_TYPE_UCHAR(NAME, SIZE) unsigned char NAME;
#define GENERATE_SER_TYPE_INT(NAME, SIZE) int NAME;
#define GENERATE_SER_TYPE_UINT(NAME, SIZE) unsigned int NAME;
#define GENERATE_SER_TYPE_SHORT(NAME, SIZE) short  NAME;
#define GENERATE_SER_TYPE_USHORT(NAME, SIZE) unsigned short NAME;
#define GENERATE_SER_TYPE_FLOAT(NAME, SIZE) float NAME;
#define GENERATE_SER_TYPE_DOUBLE(NAME, SIZE) double NAME;

#define GENERATE_SER_TYPE_CHAR_ARRAY(NAME, SIZE) char NAME[SIZE];
#define GENERATE_SER_TYPE_UCHAR_ARRAY(NAME, SIZE) unsigned char NAME[SIZE];
#define GENERATE_SER_TYPE_INT_ARRAY(NAME, SIZE) int NAME[SIZE];
#define GENERATE_SER_TYPE_UINT_ARRAY(NAME, SIZE) unsigned int NAME[SIZE];
#define GENERATE_SER_TYPE_SHORT_ARRAY(NAME, SIZE) short  NAME[SIZE];
#define GENERATE_SER_TYPE_USHORT_ARRAY(NAME, SIZE) unsigned short  NAME[SIZE];
#define GENERATE_SER_TYPE_FLOAT_ARRAY(NAME, SIZE) float NAME[SIZE];
#define GENERATE_SER_TYPE_DOUBLE_ARRAY(NAME, SIZE) double NAME[SIZE];

#define GENERATE_SER_TYPE_STRING(NAME, SIZE) char NAME[SIZE];
#define GENERATE_SER_TYPE_BOOL(NAME, SIZE) char NAME;

/*
#define SIZE_SER_TYPE_CHAR(SIZE) sizeof(char)
#define SIZE_SER_TYPE_UCHAR(SIZE) sizeof(unsigned char)
#define SIZE_SER_TYPE_DOUBLE(SIZE) sizeof(double)
#define SIZE_SER_TYPE_FLOAT(SIZE) sizeof(float)
#define SIZE_SER_TYPE_INT(SIZE) sizeof(int)
#define SIZE_SER_TYPE_UINT(SIZE) sizeof(unsigned int)
#define SIZE_SER_TYPE_SHORT(SIZE) sizeof(short )
#define SIZE_SER_TYPE_USHORT(SIZE) sizeof(unsigned short )

#define SIZE_SER_TYPE_CHAR_ARRAY(SIZE) sizeof(char[SIZE])
#define SIZE_SER_TYPE_UCHAR_ARRAY(SIZE) sizeof(unsigned char[SIZE])
#define SIZE_SER_TYPE_DOUBLE_ARRAY(SIZE) sizeof(double[SIZE])
#define SIZE_SER_TYPE_FLOAT_ARRAY(SIZE) sizeof(float[SIZE])
#define SIZE_SER_TYPE_INT_ARRAY(SIZE) sizeof(int[SIZE])
#define SIZE_SER_TYPE_UINT_ARRAY(SIZE) sizeof(unsigned int[SIZE])
#define SIZE_SER_TYPE_SHORT_ARRAY(SIZE) sizeof(short[SIZE])
#define SIZE_SER_TYPE_USHORT_ARRAY(SIZE) sizeof(unsigned short[SIZE])

#define SIZE_SER_TYPE_STRING(SIZE) sizeof(char[SIZE])
#define SIZE_SER_TYPE_BOOL(SIZE) sizeof(char)
*/


int FindParameterInfo(const SerializationInfo_t info, char *str , int len, int *ParType, int *ParOffset, int *ParSize);
int GetParameterInfo(const SerializationInfo_t info, int Index, char **name, int *ParType, int *ParOffset, int *ParSize);
unsigned int StructureSize(const SerializationInfo_t info);
#endif