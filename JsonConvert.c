#include "JsonConvert.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

typedef int (*JsonValueDeserializerFptr_t)(char *, jsmntok_t *,void* ,int ,int );
typedef int (*JsonValueSerializerFptr_t)(char*, unsigned int , void*, int , int );

int DeserializeUInt(char *str, jsmntok_t *t, void*obj,  int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    unsigned int value;
    unsigned int * valptr=(unsigned int *)(((unsigned int)obj) + offset);

    value = strtoul ( &str[t->start], NULL, 10);
    if(value==ULONG_MAX) return  -1;
    *valptr=value;
    
    return 0;
}

int DeserializeInt(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    int * valptr=(int *)(((unsigned int)obj) + offset);

    value = strtol (&str[t->start], NULL, 10);
    if(value==LONG_MAX) return  -1;
    *valptr=value;
    
    return 0;
}

int DeserializeFloat(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    float value;
    float * valptr=(float *)(((unsigned int)obj) + offset);

    value = strtof (&str[t->start], NULL);
    if(value== FLT_MAX) return  -1;
    *valptr=value;
    
    return 0;
}

int DeserializeDouble(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    double value;
    double * valptr=(double *)(((unsigned int)obj) + offset);

    value = strtod (&str[t->start], NULL);
    if(value==DBL_MAX) return  -1;
    *valptr=value;
    
    return 0;
}

int DeserializeString(char *str, jsmntok_t *t, void*obj, int offset, int size){
    int vlen= t->end - t->start;
    char  * valptr=(char *)(((unsigned int)obj) + offset);
    
    if(t->type==JSMN_PRIMITIVE && vlen==4 && !strncmp(&str[t->start],"null",4)){  
        memset(valptr,0,size);
        return 0;
    }

    if(t->type!=JSMN_STRING) return  -1;//invalid type
    if(vlen >= size) return  -2;//length error
    
    memcpy(valptr,&str[t->start],vlen);
    valptr[vlen]=0;
    
    return 0;
}

int DeserializeChar(char *str, jsmntok_t *t, void*obj,  int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    char * valptr=(char *)(((unsigned int)obj) + offset);

    value = strtol (&str[t->start], NULL, 10);
    if(value>SCHAR_MAX || value<SCHAR_MIN) return  -3;
    *valptr=value;
    
    return 0;
}

int DeserializeUChar(char *str, jsmntok_t *t, void*obj,  int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    unsigned char * valptr=(unsigned char *)(((unsigned int)obj) + offset);

    value = strtol (&str[t->start], NULL, 10);
    if(value>CHAR_MAX || value<CHAR_MIN) return  -3;
    *valptr=value;
    
    return 0;
}

int DeserializeBool(char *str, jsmntok_t *t, void*obj,  int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    char * valptr=(char *)(((unsigned int)obj) + offset);
    

    if(vlen==4 && !strncmp(&str[t->start],"true",4)){
      *valptr=1;
    }else if(vlen==5 && !strncmp(&str[t->start],"false",5)){
      *valptr=0;
    }else{
        return  -3;
    }
    
    return 0;
}

int DeserializeShort(char *str, jsmntok_t *t, void*obj,  int offset, int size){
   if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    short * valptr=(short *)(((unsigned int)obj) + offset);

    value = strtol (&str[t->start], NULL, 10);
    if(value>SHRT_MAX || value<SHRT_MIN) return  -3;
    *valptr=value;
    
    return 0;
}

int DeserializeUShort(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_PRIMITIVE) return  -1;//invalid type
    
    int vlen= t->end - t->start;
    int value;
    short * valptr=(short *)(((unsigned int)obj) + offset);

    value = strtol (&str[t->start], NULL, 10);
    if(value>USHRT_MAX || value<0) return  -3;
    *valptr=value;
    
    return 0;
}


int DeserializeCharArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    char * valptr=(char *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeChar(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeUCharArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    unsigned char * valptr=(unsigned char *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeUChar(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeShortArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    short * valptr=(short *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeShort(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeUShortArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    unsigned short * valptr=(unsigned short *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeUShort(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeFloatArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    float * valptr=(float *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeFloat(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeIntArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    int * valptr=(int *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeInt(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeUIntArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    unsigned int * valptr=( unsigned int *)(((unsigned int)obj) + offset);
    int res;


    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeUInt(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}

int DeserializeDoubleArray(char *str, jsmntok_t *t, void*obj, int offset, int size){
    if(t->type!=JSMN_ARRAY) return  -1;//invalid type
    
    double * valptr=(double *)(((unsigned int)obj) + offset);
    int res;

    if(t->size>size) return  -5;//excess elements 
    for(int i=0;i<t->size;i++){
      res = DeserializeDouble(str,&t[i+1],&valptr[i],0,0);
      if(res) return  res;
    }
    
    return 0;
}





static const JsonValueDeserializerFptr_t jsonValueDeserializers[SER_TYPE_TOTAL_COUNT]={
  DeserializeChar,
  DeserializeUChar,
  DeserializeShort,
  DeserializeUShort,
  DeserializeInt,
  DeserializeUInt,
  DeserializeFloat,
  DeserializeDouble,
  DeserializeString,

  DeserializeBool,
  DeserializeString,//password
  NULL,
  NULL,
  NULL,
  NULL,
  
  DeserializeCharArray,
  DeserializeUCharArray,
  DeserializeShortArray,
  DeserializeUShortArray,
  DeserializeIntArray,
  DeserializeUIntArray,
  DeserializeFloatArray,
  DeserializeDoubleArray,

};







int SerializeUInt(char*buf, unsigned int buflen, void*obj, int offset, int size){ 
    unsigned int * valptr=(unsigned int *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%u",*valptr);
    return len;
}

int SerializeInt(char*buf, unsigned int buflen, void*obj, int offset, int size){
    int * valptr=(int *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%d",*valptr);
    return len;
}

int SerializeFloat(char*buf, unsigned int buflen, void*obj, int offset, int size){
    float * valptr=(float *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%f",*valptr);
    return len;
}

int SerializeDouble(char*buf, unsigned int buflen, void*obj, int offset, int size){
    double * valptr=(double *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%f",*valptr);
    return len;
}

int SerializeString(char*buf, unsigned int buflen, void*obj, int offset, int size){
    char * valptr=(char *)(((unsigned int)obj) + offset);
    valptr[size-1]=0;
   
    int len;
    if(valptr[0]==0){
       len  = snprintf(buf,buflen,"null");
    }else{
        len = snprintf(buf,buflen,"\"%s\"",valptr);
    }
    
    return len;
}

int SerializeChar(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed char * valptr=(char *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%d", *valptr);
    return len;
}

int SerializeUChar(char*buf, unsigned int buflen, void*obj, int offset, int size){
    unsigned char * valptr=(unsigned char *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%u",*valptr);
    return len;
}

int SerializeBool(char*buf, unsigned int buflen, void*obj, int offset, int size){
    char * valptr=(char *)(((unsigned int)obj) + offset);  
    int len;
    if(*valptr){
      len= snprintf(buf,buflen,"true");
    }else{
      len= snprintf(buf,buflen,"false");
    }
    
    return len;
}

int SerializeShort(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed short * valptr=(short *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%d",*valptr);
    return len;
}

int SerializeUShort(char*buf, unsigned int buflen, void*obj, int offset, int size){
    unsigned short * valptr=(unsigned short *)(((unsigned int)obj) + offset);
    int len = snprintf(buf,buflen,"%u",*valptr);
    return len;
}


int SerializePassword(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed int len = snprintf(buf,buflen,"null");
    return len;
}



int SerializeCharArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed char * valptr=(char *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

    for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeChar(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
}

int SerializeUCharArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    unsigned char * valptr=(unsigned char *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

   for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeUChar(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
}

int SerializeShortArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed short * valptr=(short *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

     for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeShort(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;

}

int SerializeUShortArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    unsigned short * valptr=(unsigned short *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

     for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeUShort(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
 
}

int SerializeFloatArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    float * valptr=(float *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

    for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeFloat(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
}
int SerializeDoubleArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    double * valptr=(double *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

   for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeDouble(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
}
int SerializeIntArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    signed int * valptr=(int *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

  for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeInt(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
 
}
int SerializeUIntArray(char*buf, unsigned int buflen, void*obj, int offset, int size){
    unsigned int * valptr=(unsigned int  *)(((unsigned int)obj) + offset);    
    int len;
    int TotalLen=0;

   for(int i=0; i<size; i++){
      if(i!=0){
        TotalLen+=len  = snprintf(&buf[TotalLen],buflen-TotalLen,",");
        if(len<0) return -1;
      }

      TotalLen+=len = SerializeInt(&buf[TotalLen],buflen-TotalLen,&valptr[i],0,0);
      if(len<0) return -1;
    }

    return TotalLen;
}


static const JsonValueSerializerFptr_t jsonArrayValueSerializer[SER_TYPE_TOTAL_COUNT-SER_TYPE_PRIM_COUNT]={
  SerializeCharArray,
  SerializeUCharArray,
  SerializeShortArray,
  SerializeUShortArray,
  SerializeIntArray,
  SerializeUIntArray,
  SerializeFloatArray,
  SerializeDoubleArray
};


int SerializeArray(char*buf, unsigned int buflen, void*obj, int offset, int size, int ArrayType){
    int TotalLen=0;
    int len;

    if(jsonArrayValueSerializer[ArrayType]==NULL) return  -10;//serializer not supported

    buf+=len  = snprintf(buf,buflen,"[");
    if(len<0) return -1;
    TotalLen+=len;

    buf+=len=jsonArrayValueSerializer[ArrayType](buf,buflen-TotalLen,obj,offset,size);
    if(len<0) return  len;
    TotalLen+=len;

    buf+=len  = snprintf(buf,buflen,"]");
    if(len<0) return -1;
    TotalLen+=len;

    return  TotalLen;
}


static const JsonValueSerializerFptr_t jsonValueSerializer[SER_TYPE_PRIM_COUNT]={
  SerializeChar,
  SerializeUChar,
  SerializeShort,
  SerializeUShort,
  SerializeInt,
  SerializeUInt,
  SerializeFloat,
  SerializeDouble,
  SerializeString,
  
  SerializeBool,
  SerializePassword,
  NULL,
  NULL,
  NULL,
  NULL,
};








int JsonDeserialize(char *str, jsmntok_t *t, int tcnt, void *obj, const SerializationInfo_t info)
{
    int vlen, res;

    int type;
    int offset;
    int size;
    //memset(obj, 0, StructureSize(info));

    /* Assume the top-level element is an object */
    if (tcnt < 1 || t[0].type != JSMN_OBJECT)
    {
        return 1;
    }

    if (t[0].size == 0)
        return 0;
    
    int tid=1;
    for (int i = 0; i < t[0].size; i++)
    {
        if(t[tid].type!=JSMN_STRING) return -2;//not valid parameter should name be string

        vlen = t[tid].end - t[tid].start;
        if (!FindParameterInfo(info, &str[t[tid].start], vlen, &type, &offset, &size))
        {
            if(jsonValueDeserializers[type]==0) return  -10;//value deserializer not supported

            res = jsonValueDeserializers[type](str, &t[tid+1], obj, offset, size);
            if (res)return res;
        }

        tid += 1 + t[tid].size + t[tid+1].size;
        
    }
    return 0;
}

int JsonSerialize(void *obj, const SerializationInfo_t info, char* buf, unsigned int buflen)
{
    char *name;
    int type;
    int offset;
    int size;

    int res;
    int TotalLen=0;
  
    TotalLen+=res = snprintf(&buf[TotalLen], buflen-TotalLen, "{");
    if(res<0) return -1; //buffer full

    for (int i = 1; ; i++)
    {
        if(GetParameterInfo(info,i,&name,&type,&offset,&size)) break;//end of parameter list info

        if(i>1){
          TotalLen+=res = snprintf(&buf[TotalLen], buflen-TotalLen, ",");
          if(res<0) return -1; //buffer full
    
        }

        TotalLen+=res = snprintf(&buf[TotalLen], buflen-TotalLen, "\"%s\":", name);
        if(res<0) return -1; //buffer full
   

        if(type<SER_TYPE_PRIM_COUNT){
              if(jsonValueSerializer[type] == NULL) return -10;
              TotalLen+=res = jsonValueSerializer[type](&buf[TotalLen], buflen-TotalLen, obj, offset, size);
             
        }else{
              type-=SER_TYPE_PRIM_COUNT;
              TotalLen+= res = SerializeArray(&buf[TotalLen], buflen-TotalLen, obj, offset, size,type);
        }

       
        if(res<0) return res;//exception
       
    }

    TotalLen+=res = snprintf(&buf[TotalLen], buflen-TotalLen, "}");
    if(res<0) return -1; //buffer full

    return TotalLen;
}
