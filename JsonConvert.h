#ifndef __JSON_CONVERT__
#define __JSON_CONVERT__

#include "serialization.h"

typedef int(*JsonConvertStreamCallback)(void*arg, char *buf,int len, int *bread);


typedef  enum { 
   JSON_OK                  = 0,
   JSON_REPEAT_LAST         = 1,//serializer buffer was full
   JSON_TKN_VALUE_SEPARATOR = 2, 
   JSON_TKN_ITEM_SEPARATOR  = 3,
   JSON_TKN_OBJECT_OPEN     = 4,
   JSON_TKN_OBJECT_CLOSE    = 5,
   JSON_TKN_ARRAY_OPEN      = 6,
   JSON_TKN_ARRAY_CLOSE     = 7,
   JSON_TKN_PRIMITIVE       = 8,
   JSON_TKN_NUMBER          = 9,
   JSON_TKN_STRING          = 10,


   JSON_STREAM_END         = -1,
   JSON_STREAM_READ_ERROR  = -2,
   JSON_STREAM_WRITE_ERROR = -3,
   JSON_STREAM_BUFFER_FULL = -4,
   JSON_BUFFER_ERROR       = -5,
   JSON_BODY_ERROR         = -6,
   JSON_TOKEN_ERROR        = -7,
   JSON_NULL_CHARACTER     = -8,
   JSON_INVALID_TYPE       = -9,
   JSON_INVALID_VALUE      = -10,
   JSON_NOT_SUPPORTED      = -11,
   JSON_MAX_NEST           = -12,
   JSON_VALIDATION_ERROR   = -13
}JsonResType;

#define JSON_OPT_IGNORE_VALIDATION_ERR (1<<0)
#define JSON_OPT_NO_STREAM_ON_EXIT     (1<<1)

typedef  struct{
  JsonConvertStreamCallback     StreamX;
  void *                        StreamXArg;

  char *buf;
  int   buflen;
  int   len;
  int   pos;

  unsigned int MaxNest;
  unsigned int NestCount;

  unsigned int Options;
}JsonConvertHandle;


void JsonInitHandle(JsonConvertHandle *hnd, void *buf, int buflen, int datalen, unsigned int Options);
void JsonHandleRegisterCallback(JsonConvertHandle *hnd, JsonConvertStreamCallback StreamRead, void *arg);

JsonResType JsonSerializeObjectArray(JsonConvertHandle *hnd, void*obj, int ArraySize, const SerializationInfo_t info);
JsonResType JsonSerializeObjectPtrArray(JsonConvertHandle *hnd, void*obj, int ArraySize, const SerializationInfo_t info);
JsonResType JsonSerialize(void *obj, const SerializationInfo_t info, JsonConvertHandle* hnd);

JsonResType JsonDeserialize(JsonConvertHandle* hnd,  void *obj, const SerializationInfo_t info);
JsonResType JsonDeserializeObjectArray(JsonConvertHandle* hnd,  void *obj,int size, const SerializationInfo_t info);
JsonResType JsonWrite(JsonConvertHandle *hnd, char *buf, unsigned int len);
JsonResType JsonWriteString(JsonConvertHandle *hnd, char *str);
JsonResType JsonFlush(JsonConvertHandle* hnd);


//private 
JsonResType JsonDeserializeObject(JsonConvertHandle* hnd,  void *obj, const SerializationInfo_t info);

#endif
