#ifndef __SERIALIZATION__
#define __SERIALIZATION__

#include <stddef.h>
#include "Validators.h"
#include "DateTime.h"


typedef  int(*ValueValidatorFptr)(void*context ,void *arg,  char *str, int len); 

typedef const  struct{
  const char        *Name;
  const unsigned int NameLength;
  const char        *Namespace;
  const char         Type;
  const unsigned int Offset;
  const unsigned int Size;
  const unsigned int ElementSize;
  const void       * SInfo;
  const char         Direction;
  const ValueValidatorFptr Validator;
} SerializationItem;

typedef const  SerializationItem SerializationInfo_t[];

#define SER_DIR_RW 0
#define SER_DIR_R  1
#define SER_DIR_W  2

#define member_size(type, member)         sizeof(((const type *)0)->member)
#define array_member_size(type, member)   sizeof(((const type *)0)->member[0])
#define array_length(type, member)        sizeof(((const type *)0)->member) / sizeof(((const type *)0)->member[0])


#define SERIALIZATION_START(NAME,NAMESPACE, STRUCTTYPE){.Type=0, .Name=NAME, .NameLength=sizeof(#NAME)-1,  .Namespace=NAMESPACE "_", .Size=sizeof(STRUCTTYPE)}
#define SERIALIZATION_END() {}

#define GENERATE_SER_OBJECT_ARRAY(NAME, CLASSTYPE, SERINFO)                   {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_OBJECT_ARRRY,     .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME), .SInfo=SERINFO},
#define GENERATE_SER_OBJECT_PTR_ARRAY(NAME, CLASSTYPE, SERINFO)               {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_OBJECT_PTR_ARRAY, .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME), .SInfo=SERINFO},
#define GENERATE_SER_OBJECT(NAME, CLASSTYPE, SERINFO)                         {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_OBJECT,           .Offset=offsetof(CLASSTYPE,NAME), .Size=sizeof(CLASSTYPE), .SInfo=SERINFO},
#define GENERATE_SER_OBJECT_PTR(NAME, CLASSTYPE, SERINFO)                     {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_OBJECT_PTR,       .Offset=offsetof(CLASSTYPE,NAME), .Size=sizeof(CLASSTYPE), .SInfo=SERINFO},
#define GENERATE_SERIALIZABLE_STRING(NAME, CLASSTYPE)                         {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_STRING,           .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME)},
#define GENERATE_SERIALIZABLE_STRING_D(NAME, CLASSTYPE,DIR)                   {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=SER_TYPE_STRING,           .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME), .Direction=DIR },
#define GENERATE_SERIALIZABLE(NAME, TYPE,  CLASSTYPE)                         {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=member_size(CLASSTYPE,NAME)},
#define GENERATE_SERIALIZABLE_D(NAME, TYPE, CLASSTYPE,DIR)                    {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=member_size(CLASSTYPE,NAME), .Direction=DIR},
#define GENERATE_SERIALIZABLE_DN(NAME, ALTNAME, TYPE,  CLASSTYPE,DIR)         {.Name=ALTNAME, .NameLength=sizeof(ALTNAME)-1,  .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=member_size(CLASSTYPE,NAME), .Direction=DIR},
#define GENERATE_SERIALIZABLE_N(NAME,  ALTNAME, TYPE,  CLASSTYPE)             {.Name=ALTNAME, .NameLength=sizeof(ALTNAME)-1,  .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=member_size(CLASSTYPE,NAME)},
#define GENERATE_VALIDATE_SERIALIZABLE(NAME, TYPE,  CLASSTYPE,VALIDATOR)      {.Name=#NAME,  .NameLength=sizeof(#NAME)-1,     .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=member_size(CLASSTYPE,NAME), .Validator=VALIDATOR},
#define GENERATE_SERIALIZABLE_ARRAY(NAME, TYPE,  CLASSTYPE)                   {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME) ,.ElementSize=array_member_size(CLASSTYPE,NAME)},
#define GENERATE_SERIALIZABLE_ARRAY_D(NAME, TYPE,  CLASSTYPE, DIR)                   {.Name=#NAME, .NameLength=sizeof(#NAME)-1,      .Type=TYPE,                      .Offset=offsetof(CLASSTYPE,NAME), .Size=array_length(CLASSTYPE,NAME) ,.ElementSize=array_member_size(CLASSTYPE,NAME), .Direction=DIR},



typedef enum{
   SER_TYPE_UNSIGNED,
   SER_TYPE_SIGNED,
   SER_TYPE_FLOAT, 

   SER_TYPE_STRING,
   SER_TYPE_BOOL, 
   SER_TYPE_PASSWORD,  
   SER_TYPE_TIME, 
   SER_TYPE_DATE, 
   SER_TYPE_DATETIME,
   SER_TYPE_TIMESPAN,
   SER_TYPE_EUI_48,
   SER_TYPE_EUI_64,
   SER_TYPE_HEX,
   SER_TYPE_RHEX,
   SER_TYPE_PRIM_COUNT, //count of primitive types

   SER_TYPE_UNSIGNED_ARRAY=SER_TYPE_PRIM_COUNT, 
   SER_TYPE_SIGNED_ARRAY, 
   SER_TYPE_FLOAT_ARRAY, 
   SER_TYPE_BOOL_ARRAY,
   SER_TYPE_HEX_ARRAY,
   SER_TYPE_RHEX_ARRAY,
   SER_TYPE_ARRAY_COUNT,//count of all types
  
 
   SER_TYPE_OBJECT = SER_TYPE_ARRAY_COUNT,
   SER_TYPE_OBJECT_PTR, //serialization only
   SER_TYPE_OBJECT_ARRRY,
   SER_TYPE_OBJECT_PTR_ARRAY,//serialization only
   SER_TYPE_STRING_PTR,
   SER_TYPE_TOTAL_COUNT,
}SerType_t;


int FindParameterInfo(const SerializationInfo_t info, char *str, unsigned int len, unsigned int dir);
unsigned int StructureSize(const SerializationInfo_t info);

#endif
