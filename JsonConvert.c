#include "JsonConvert.h"
#include "Utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>

typedef JsonResType (*JsonValueDeserializerFptr_t)(JsonConvertHandle *,void*  ,int, ValueValidatorFptr );
typedef JsonResType (*JsonArrayDeserializerFptr_t)(JsonConvertHandle *,void*  ,int,int, ValueValidatorFptr );
typedef int         (*JsonValueSerializerFptr_t)(JsonConvertHandle *, void*,  int);
typedef int         (*JsonArraySerializerFptr_t)(JsonConvertHandle *, void*,  int,int);

JsonResType JsonArraySkip(JsonConvertHandle* hnd);
JsonResType JsonObjectSkip(JsonConvertHandle* hnd);




JsonResType JsonInternalFlush(JsonConvertHandle *hnd, JsonResType LastResult)
{
    int repeat=0;

    if(LastResult<JSON_OK) return  LastResult;
    hnd->pos += LastResult;
    
    if (hnd->StreamX != NULL && hnd->pos >= hnd->buflen)
    {
        if (LastResult > hnd->buflen) return JSON_BUFFER_ERROR; //buffer to small protection
        if (hnd->pos > hnd->buflen)
        {
            hnd->pos -= LastResult; //fix position
            repeat = 1;
        }
        LastResult = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
        if (LastResult != JSON_OK) return JSON_STREAM_WRITE_ERROR;
        hnd->pos = 0;
    }

    if(repeat) return  JSON_REPEAT_LAST;
    return JSON_OK;
}

JsonResType JsonReadStream(JsonConvertHandle* hnd){
  int btoread = hnd->buflen;
  int bread;
  int res,pos=0;
 

  if(hnd->StreamX==NULL)
   return  JSON_STREAM_END;
  if(hnd->len==hnd->buflen && hnd->pos==0) return  JSON_STREAM_BUFFER_FULL;
  if(hnd->pos>hnd->len )hnd->pos=hnd->len;

  //shift bytes to beginning of buffer
  for(int i=hnd->pos; i<hnd->len; i++){
    hnd->buf[pos++] = hnd->buf[i];
  }
  
  //calculate bytes to read/length
  hnd->len = (hnd->len - hnd->pos); 
  btoread  =  hnd->buflen - hnd->len;
  hnd->pos = 0;

  //read stream
  res = hnd->StreamX(hnd->StreamXArg, &hnd->buf[hnd->len], btoread, &bread );
  if(res || bread>btoread) return JSON_STREAM_READ_ERROR;
  if(btoread!=0 && bread==0)
    return JSON_STREAM_END; 

  hnd->len += bread;
  return JSON_OK;
}



JsonResType JsonTokenType(JsonConvertHandle* hnd){
  int res;

  do{
    for(;hnd->pos < hnd->len; hnd->pos++){
      switch(hnd->buf[hnd->pos]){
          case '\0': return JSON_NULL_CHARACTER;       
          case  '\t' : case '\r' : case '\n': case ' ':
            continue;
          case '[' : return JSON_TKN_ARRAY_OPEN;
          case ']' : return JSON_TKN_ARRAY_CLOSE;
          case '{' : return JSON_TKN_OBJECT_OPEN;
          case '}' : return JSON_TKN_OBJECT_CLOSE;
          case ':' : return JSON_TKN_VALUE_SEPARATOR;
          case ',' : return JSON_TKN_ITEM_SEPARATOR;
          case '\"': return JSON_TKN_STRING;
          default  : return JSON_TKN_PRIMITIVE;
      } 
    } 
  }while(!(res=JsonReadStream(hnd)));
  
  return res;
}


#define JSON_NO_ESCAPE    0
#define JSON_ESCAPE_START 1


JsonResType JsonTokenPosition(JsonConvertHandle* hnd, int *start, int *len){
  int res;
  int type = JsonTokenType(hnd);  
  int escape = JSON_NO_ESCAPE;

  if(type <= 0) return type;

  do{
    int pos = hnd->pos;
    *start=pos;

    for(;pos < hnd->len; pos++){
     
      switch(type){
        case JSON_TKN_ARRAY_OPEN:
        case JSON_TKN_ARRAY_CLOSE:
        case JSON_TKN_OBJECT_OPEN:
        case JSON_TKN_OBJECT_CLOSE:
        case JSON_TKN_VALUE_SEPARATOR:
        case JSON_TKN_ITEM_SEPARATOR:
          *len=pos - *start +1;
          return type;
        case JSON_TKN_STRING:
            if(escape){
               if(escape==1){
                 switch(hnd->buf[pos]){
                      case '\"': case '/' : case '\\' : case 'b' :
		      case 'f' : case 'r' : case 'n'  : case 't' :
			break;
                      case 'u':
                        escape++;
                        break;


                      default:
                        return  JSON_TOKEN_ERROR;
                 }
               }else{//4 hex chars
                  if(!((hnd->buf[pos] >= '0' && hnd->buf[pos] <= '9') || 
		       (hnd->buf[pos] >= 'A' && hnd->buf[pos] <= 'F') ||
		       (hnd->buf[pos] >= 'a' && hnd->buf[pos] <= 'f'))){  
							return JSON_TOKEN_ERROR;
                  }
                  escape++;
                  if(escape==6) escape=0;
               }
            }else{

            switch(hnd->buf[pos]){
                case '\0': return JSON_NULL_CHARACTER;       
                case '\t' : case '\r' : case '\n' : return JSON_TOKEN_ERROR;   
                case '\\': 
                  escape = JSON_ESCAPE_START;
                  break;
                case '\"': {
                  if(*start==pos)continue;
                  *start= hnd->pos+1;
                  *len=pos - *start;
                  return type;
                }
            }
            break;
            }
        case JSON_TKN_PRIMITIVE:
          switch(hnd->buf[pos]){
               case '\0': return JSON_NULL_CHARACTER;       
               case ':' : 
                  return JSON_TOKEN_ERROR;
               case ',' : case '}': case ']': case '\t' : case '\r' : case '\n':
                  *len=pos - *start;
                  return type; 
          }
          if (hnd->buf[pos] < 32 || hnd->buf[pos] >= 127) 
              return JSON_TOKEN_ERROR;
	 continue;
              
        default: 
          return JSON_TOKEN_ERROR;
      }
    } 
  }while(!(res=JsonReadStream(hnd)));
  
  return res;
}




JsonResType JsonObjectSkip(JsonConvertHandle* hnd){
    int start,len;
    JsonResType type;
  
    type = JsonTokenType(hnd);
    if(type<0) return type ;
    if(type!=JSON_TKN_OBJECT_OPEN) return type;
    hnd->pos++;

    do
    {
        type = JsonTokenPosition(hnd, &start, &len);
        if(type<0) return type ;  
        if(type==JSON_TKN_OBJECT_CLOSE){
          hnd->pos++;
          return  JSON_OK;
        }
        if(type!=JSON_TKN_STRING) return JSON_TOKEN_ERROR;
            
        hnd->pos += len+2;
        type = JsonTokenType(hnd);
        if(type<0) return type ;
        if(type != JSON_TKN_VALUE_SEPARATOR) return type;
        hnd->pos++;
        
        type = JsonTokenPosition(hnd, &start, &len);
        if(type<0) return type ;
        
        
        switch(type){
          case JSON_TKN_OBJECT_OPEN:
            if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
            hnd->NestCount++;
            type = JsonObjectSkip(hnd);
            hnd->NestCount--;
            if(type<0) return  type;
            break;

          case JSON_TKN_ARRAY_OPEN:
            if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
            hnd->NestCount++;
            type = JsonArraySkip(hnd);
            hnd->NestCount++;
            if(type<0) return  type;
            break;

          case JSON_TKN_PRIMITIVE:
            hnd->pos+=len;
            break;
    
          case JSON_TKN_STRING:
            hnd->pos+=len+2;
            break;
    
          default:
            return JSON_TOKEN_ERROR;
        }
        

        type = JsonTokenType(hnd);
        if(type<0) return type ;
        if(type!=JSON_TKN_ITEM_SEPARATOR && type!=JSON_TKN_OBJECT_CLOSE )return  JSON_TOKEN_ERROR;
        if(type==JSON_TKN_ITEM_SEPARATOR) hnd->pos++;  
    }while(1);
}

JsonResType JsonArraySkip(JsonConvertHandle* hnd){
    int start,len;
    JsonResType type;
   
    type = JsonTokenType(hnd);
    if(type<0) return type ;
    if(type!=JSON_TKN_ARRAY_OPEN) return type;
    hnd->pos++;

    do
    {
        type = JsonTokenPosition(hnd, &start, &len);
        if(type<0) return type ;  
        if(type==JSON_TKN_ARRAY_CLOSE){
          hnd->pos++;
          return  JSON_OK;
        }
        if(type!=JSON_TKN_STRING) return JSON_TOKEN_ERROR;
        
       
        switch(type){
          case JSON_TKN_OBJECT_OPEN:
             if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
             hnd->NestCount++;
             type = JsonObjectSkip(hnd);
             hnd->NestCount--;
             if(type<0) return  type;
             break;
    
          case JSON_TKN_ARRAY_OPEN:
             if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
             hnd->NestCount++;
             type= JsonArraySkip(hnd);
             hnd->NestCount--;
             if(type<0) return  type;
             break;

          case JSON_TKN_PRIMITIVE:
            hnd->pos+=len;
            break;
    
          case JSON_TKN_STRING:
            hnd->pos+=len+2;
            break;
    
          default:
            return JSON_TOKEN_ERROR;
        }
        

        type = JsonTokenType(hnd);
        if(type<0) return type ;
        if(type!=JSON_TKN_ITEM_SEPARATOR && type!=JSON_TKN_ARRAY_CLOSE )return  JSON_TOKEN_ERROR;
        if(type==JSON_TKN_ITEM_SEPARATOR) hnd->pos++;  
    }while(1);
}





JsonResType DeserializeUInt(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;   

    unsigned int value;
    unsigned int * valptr=(unsigned int *)obj;

    value = strtoul ( &hnd->buf[start], NULL, 10);
    if(value==ULONG_MAX) return  JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }

    *valptr=value;
    
    return 0;
}

JsonResType DeserializeInt(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;

    int value;
    int * valptr=(int *)obj;

    value = strtol (&hnd->buf[start], NULL, 10);
    if(value==LONG_MAX) return  JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
   
   *valptr=value;
    
    return 0;
}

JsonResType DeserializeFloat(JsonConvertHandle *hnd, void*obj,  int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;

    float value;
    float * valptr=(float *)obj;

    value = strtof (&hnd->buf[start], NULL);
    if(value== FLT_MAX) return  JSON_INVALID_VALUE;

    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}

JsonResType DeserializeDouble(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;

    double value;
    double * valptr=(double *)obj;

    value = strtod (&hnd->buf[start], NULL);
    if(value==DBL_MAX) return  JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }

    *valptr=value;
    
    return 0;
}

JsonResType DeserializeString(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;

    char  * valptr=(char *)obj;
    
    if(type==JSON_TKN_PRIMITIVE && len==4 && !strncmp(&hnd->buf[start],"null",4)){  
        memset(valptr,0,size);
        hnd->pos+=len;
        return 0;
    }

    if(type!=JSON_TKN_STRING) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len+2;
    
    if(len >= size) return  JSON_INVALID_VALUE;//length error
    
    if(Validator!=NULL){
      if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }

    memcpy(valptr,&hnd->buf[start],len);
    valptr[len]=0;
    
    return 0;
}

JsonResType DeserializeChar(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;


    int value;
    char * valptr=(char *)obj;

    value = strtol (&hnd->buf[start], NULL, 10);
    if(value>SCHAR_MAX || value<SCHAR_MIN) return  JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}

JsonResType DeserializeUChar(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;


    int value;
    unsigned char * valptr=(unsigned char *)obj;

    value = strtol (&hnd->buf[start], NULL, 10);
    if(value>CHAR_MAX || value<CHAR_MIN) return  JSON_INVALID_VALUE;
   
    if(Validator!=NULL){
      if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}

JsonResType DeserializeBool(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;


    int value;
    char * valptr=(char *)obj;
    

    if(len==4 && !strncmp(&hnd->buf[start],"true",4)){
      *valptr=1;
    }else if(len==5 && !strncmp(&hnd->buf[start],"false",5)){
      *valptr=0;
    }else{
        return  JSON_INVALID_VALUE;
    }
    
    return 0;
}

JsonResType DeserializeShort(JsonConvertHandle *hnd, void*obj,  int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;


    int value;
    short * valptr=(short *)obj;

    value = strtol (&hnd->buf[start], NULL, 10);
    if(value>SHRT_MAX || value<SHRT_MIN) return JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}

JsonResType DeserializeUShort(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
    if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;//invalid type
    hnd->pos+=len;

    int value;
    short * valptr=(short *)obj;

    value = strtol (&hnd->buf[start], NULL, 10);
    if(value>USHRT_MAX || value<0) return  JSON_INVALID_VALUE;
    
    if(Validator!=NULL){
      if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}


JsonResType DeserializeUnsigned(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
   switch(size){
     case 1 :
      return  DeserializeUChar(hnd,obj,size,Validator);
     case 2:
      return  DeserializeUShort(hnd,obj,size,Validator);
     case 4:
      return  DeserializeUInt(hnd,obj,size,Validator);
     case 8:
      break;
   }
   return  JSON_NOT_SUPPORTED;
}

JsonResType DeserializeSigned(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
   switch(size){
     case 1 :
      return  DeserializeChar(hnd,obj,size,Validator);
     case 2:
      return  DeserializeShort(hnd,obj,size,Validator);
     case 4:
      return  DeserializeInt(hnd,obj,size,Validator);
     case 8:
      break;
   }
   return  JSON_NOT_SUPPORTED;
}


JsonResType DeserializeFloating(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
   switch(size){
     
     case 4:
      return  DeserializeFloat(hnd,obj,size,Validator);
     case 8:
      return  DeserializeDouble(hnd,obj,size,Validator);
   }
   return  JSON_NOT_SUPPORTED;
}


int atoin(char *val,unsigned int len){
   int i;
   int ret = 0;
   for(i = 0; i < len; i++)
   {
        if(val[i]<'0' || val[i]>'9') break;
        ret = (ret * 10) + (val[i] - '0');
   }
   return ret;
}

int JSONISODateTimeConvert(dateTime_t *dt, char *value,int len){
  //example "2012-04-23T18:25:43.511Z" 
  int val;
  if(len!=24) return -1;

  for(int i=0;i<len;i++){
    switch(value[i]){
      case '-':
        if(i==4){//year
          dt->usYear = atoin(value,4);
          continue;

        }else if(i==7){//month
          val = atoin(&value[5],2);
          if(val<1 || val>12) return  -1;
          dt->ucMonth=val;
          continue;
        }
        return  -1;

      case 'T':
        if(i==10){//day
          val = atoin(&value[8],2);
          if(val<1 || val>31) return -1;
          dt->ucDay=val;
          continue;
        }
        return  -1;

      case ':':
        if(i==13){//hour
            val = atoin(&value[11],2);
            if(val<0 || val>23) return -1;
            dt->ucHour=val;
            continue;
        }else if(i==16){//min
            val = atoin(&value[14],2);
            if(val<0 || val>59) return -1;
            dt->ucMin=val;
            continue;
        }
        return  -1;

      case '.':
         if(i==19){//seconds
            val = atoin(&value[17],2);
            if(val<0 || val>59) return -1;
            dt->ucSec=val;
            continue;
         }
         return  -1;

      case 'Z':
        if(i==23){//milliseconds
          return  0;
        }
        return  -1;
    }
     
    if(value[i]<'0' || value[i]>'9') return  -1;
  
  }
  return -1;
}

int JSONTimespanConvert(timespan_t *timespan, char *value,int len){
  //example "2012-04-23T18:25:43.511Z" 
  timespan_t val=0;
  int tmp;
  if(len<8) return -1;
  
  int part=0;
  int lastpos=0;
  int nlen;
  
  for(int i=0;i<len;i++){
    nlen=i-lastpos;

    switch(value[i]){
      case '.':
        if(part==0 && nlen<=5){//days
          tmp = atoin(&value[lastpos], nlen);
          val += tmp*24*60*60;
          lastpos=i+1;
          part++;
          continue;
        }else if ( part==4 && nlen<=7){//milliseconds
          tmp = atoin(&value[lastpos], nlen);
          lastpos=i+1;
          part++;
        }
        return  -1;

      case ':':
        if(part==0)part=1;
        if(part==1 && nlen==2){//Hours
          tmp = atoin(&value[lastpos], nlen);
          if(tmp<0 || tmp>23) return -1;
          val += tmp*60*60;
          lastpos=i+1;
          part++;
          continue;
        }else if(part==2 && nlen==2){//minutes
          tmp = atoin(&value[lastpos], nlen);
          if(tmp<0 || tmp>59) return -1;
          val += tmp*60;
          lastpos=i+1;
          part++;
          continue;
        }else if(part==3 && nlen==2){//seconds
          tmp = atoin(&value[lastpos], nlen);
           if(tmp<0 || tmp>59) return -1;
          val += tmp;
          lastpos=i+1;
          part++;
          continue;
        }
        return  -1;
    }    
    if(value[i]<'0' || value[i]>'9') return  -1;
  }

  //no milliseconds get minutes

  if(part == 3){
    nlen = len - lastpos;    
    if(nlen != 2) return  -1;

    tmp = atoin(&value[lastpos], nlen);
    if(tmp<0 || tmp>59) return -1;
    val += tmp;
  }

  *timespan = val;
  return 0;
}


JsonResType DeserializeTime(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
   
    
    dateTime_t value;
    memset(&value,0,sizeof(dateTime_t));
    dateTime_t * valptr=(dateTime_t *)obj;


    if(type==JSON_TKN_PRIMITIVE){
       if(!(len==4 && !strncmp(&hnd->buf[start],"null",4))){
         return  JSON_INVALID_TYPE;
       }
       hnd->pos+=len;
    }else if(type==JSON_TKN_STRING){
       if(JSONISODateTimeConvert(&value, &hnd->buf[start], len)) return JSON_INVALID_VALUE; 
       value.ucDay=1;
       value.ucMonth=1;
       value.usYear=1970;
       if(!DateTimeIsValid(&value))return JSON_INVALID_VALUE; 

        hnd->pos+=len+2;
    }else{
        return  JSON_INVALID_TYPE;//invalid type
    }

    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    memcpy(valptr, &value,sizeof(dateTime_t));
    
    return 0;
}

JsonResType DeserializeDate(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
   
    
    dateTime_t value;
    memset(&value,0,sizeof(dateTime_t));
    dateTime_t * valptr=(dateTime_t *)obj;


    if(type==JSON_TKN_PRIMITIVE){
       if(!(len==4 && !strncmp(&hnd->buf[start],"null",4))){
         return  JSON_INVALID_TYPE;
       }
       hnd->pos+=len;
    }else if(type==JSON_TKN_STRING){
       if(JSONISODateTimeConvert(&value, &hnd->buf[start], len)) return JSON_INVALID_VALUE; 
       value.ucHour=0;
       value.ucMin=0;
       value.ucSec=0;
       if(!DateTimeIsValid(&value))return JSON_INVALID_VALUE; 

        hnd->pos+=len+2;
    }else{
        return  JSON_INVALID_TYPE;//invalid type
    }

    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    memcpy(valptr, &value,sizeof(dateTime_t));
    
    return 0;
}
 
JsonResType DeserializeDateTime(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
   
    
    dateTime_t value;
    memset(&value,0,sizeof(dateTime_t));
    dateTime_t * valptr=(dateTime_t *)obj;


    if(type==JSON_TKN_PRIMITIVE){
       if(!(len==4 && !strncmp(&hnd->buf[start],"null",4))){
         return  JSON_INVALID_TYPE;
       }
       hnd->pos+=len;
    }else if(type==JSON_TKN_STRING){
       if(JSONISODateTimeConvert(&value, &hnd->buf[start], len)) return JSON_INVALID_VALUE; 
       if(!DateTimeIsValid(&value))return JSON_INVALID_VALUE; 

        hnd->pos+=len+2;
    }else{
        return  JSON_INVALID_TYPE;//invalid type
    }

    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    memcpy(valptr, &value, sizeof(dateTime_t));
    
    return 0;
}

JsonResType DeserializeTimespan(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < 0 ) return type;
   
    
    timespan_t value;
    value=0;
    timespan_t * valptr=(timespan_t *)obj;


    if(type == JSON_TKN_STRING){
       if(JSONTimespanConvert(&value, &hnd->buf[start], len)) return JSON_INVALID_VALUE; 

        hnd->pos+=len+2;
    }else{
        return  JSON_INVALID_TYPE;//invalid type
    }

    
    if(Validator!=NULL){
      if(Validator(hnd,&value, &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
    }
    
    *valptr=value;
    
    return 0;
}



JsonResType DeserializeEUI48(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
  if(size!=6)return  JSON_INVALID_TYPE;
  
  int start,len;
  int type = JsonTokenPosition(hnd, &start, &len);
  if( type < 0 ) return type;
  
  int  eui48pos=0;
  char eui48[6];

  char  * valptr=(char *)obj;
  char   chr;
  
  if(type==JSON_TKN_PRIMITIVE && len==4 && !strncmp(&hnd->buf[start],"null",4)){  
      memset(valptr,0,size);
      hnd->pos+=len;
      return 0;
  }
 
  if(type!=JSON_TKN_STRING) return  JSON_INVALID_TYPE;//invalid type
  hnd->pos+=len+2;
  
  if(len != 17) return  JSON_INVALID_VALUE;//length error
  
  if(Validator!=NULL){
    if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
  }
  
  for(int i=0;i<=18;i++){
    chr = hnd->buf[start+i];
    if((i%2)==0){
      if(i!=18  && chr!=':' ) return  JSON_INVALID_VALUE;
      eui48[eui48pos++]= (HexToBin(hnd->buf[start+i-2])<<4) | HexToBin(hnd->buf[start+i-1]);

    }else if(!((chr>='0'  && chr<='9' ) ||
             (chr>='a'  && chr<='f')||
             (chr>='A'  && chr<='F'))){
        return  JSON_INVALID_VALUE;
    }
  }

  memcpy(valptr, eui48 , 6);
  
  return 0;

}


JsonResType DeserializeEUI64(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
  if(size!=8)return  JSON_INVALID_TYPE;
  
  int start,len;
  int type = JsonTokenPosition(hnd, &start, &len);
  if( type < 0 ) return type;
  
  int  eui64pos=0;
  char eui64[8];

  char  * valptr=(char *)obj;
  char   chrl;
  char   chrh;

  if(type==JSON_TKN_PRIMITIVE && len==4 && !strncmp(&hnd->buf[start],"null",4)){  
      memset(valptr,0,size);
      hnd->pos+=len;
      return 0;
  }
 
  if(type!=JSON_TKN_STRING) return  JSON_INVALID_TYPE;//invalid type
  hnd->pos+=len+2;
  
  if(len != 23) return  JSON_INVALID_VALUE;//length error
  
  if(Validator!=NULL){
    if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
  }
  
  for(int i=0;i<=23;i++){
    if((i<23-2) && hnd->buf[start+i+2] != ':') return  JSON_INVALID_VALUE;
    chrl = HexToBin(hnd->buf[start+i+1]);
    chrh = HexToBin(hnd->buf[start+i]);
    i+=2;
    if(chrl==-1 || chrh==-1) return  JSON_INVALID_VALUE;
    
    eui64[eui64pos++]= (chrh<<4) | chrl;  
  }

  memcpy(valptr, eui64 , 8);
  
  return 0;

}

JsonResType DeserializeHex(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
  
  int start,len;
  int type = JsonTokenPosition(hnd, &start, &len);
  if( type < 0 ) return type;
  
  int  hexppos=0;

  char  * valptr=(char *)obj;
  char   chrl;
  char   chrh;
  
  if(type==JSON_TKN_PRIMITIVE && len==4 && !strncmp(&hnd->buf[start],"null",4)){  
      memset(valptr,0,size);
      hnd->pos+=len;
      return 0;
  }
 
  if(type!=JSON_TKN_STRING) return  JSON_INVALID_TYPE;//invalid type
  hnd->pos+=len+2;
  
  if(len != (size*2)) return  JSON_INVALID_VALUE;//length error
  
  if(Validator!=NULL){
    if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
  }
  
  for(int i=0;i<len;i+=2){
    chrl = HexToBin(hnd->buf[start+i+1]);
    chrh = HexToBin(hnd->buf[start+i]);

    if(chrl==-1 || chrh==-1){
        return  JSON_INVALID_VALUE;
    }

    valptr[hexppos++]= (chrh<<4) | chrl;
  }
  
  return 0;

}



JsonResType DeserializeRHex(JsonConvertHandle *hnd, void*obj, int size, ValueValidatorFptr Validator){
  if(size!=8)return  JSON_INVALID_TYPE;
  
  int start,len;
  int type = JsonTokenPosition(hnd, &start, &len);
  if( type < 0 ) return type;
  
  int  hexppos=size-1;

  char  * valptr=(char *)obj;
  char   chr;
  
  if(type==JSON_TKN_PRIMITIVE && len==4 && !strncmp(&hnd->buf[start],"null",4)){  
      memset(valptr,0,size);
      hnd->pos+=len;
      return 0;
  }
 
  if(type!=JSON_TKN_STRING) return  JSON_INVALID_TYPE;//invalid type
  hnd->pos+=len+2;
  
  if(len != (size*2)) return  JSON_INVALID_VALUE;//length error
  
  if(Validator!=NULL){
    if(Validator(hnd,&hnd->buf[start], &hnd->buf[start],len)) return JSON_VALIDATION_ERROR; 
  }
  
  for(int i=0;i<=(size*2);i++){
    chr = hnd->buf[start+i];
    
    if(!((chr>='0'  && chr<='9' ) ||
             (chr>='a'  && chr<='f')||
             (chr>='A'  && chr<='F'))){
        return  JSON_INVALID_VALUE;
    }
    
    if(i%1){
      valptr[hexppos--]= (HexToBin(hnd->buf[start+i-1])<<4) | HexToBin(hnd->buf[start+i]);
    }
    
  }

  
  return 0;

}








JsonResType DeserializeUnsignedArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}

      res = DeserializeUnsigned(hnd, valptr,size,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

        type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}

JsonResType DeserializeSignedArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}

      res = DeserializeSigned(hnd, valptr,size,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

        type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}

JsonResType DeserializeFloatingArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}

      res = DeserializeFloat(hnd, valptr,size,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type!=JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

        type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}

JsonResType DeserializeBoolArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      
      res = DeserializeBool(hnd, valptr ,0,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type== JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type != JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}

JsonResType DeserializeHexArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      
      res = DeserializeHex(hnd, valptr ,0,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type== JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type != JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}

JsonResType DeserializeRHexArray(JsonConvertHandle *hnd, void*obj, int length, int size, ValueValidatorFptr Validator){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    
    char * valptr=(char *)obj;
    int res;

    for(int i=0;i<length;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      
      res = DeserializeRHex(hnd, valptr ,0,Validator);
      if(res) return  res;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type== JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;

      valptr+size;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type != JSON_TKN_PRIMITIVE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}




static const JsonValueDeserializerFptr_t jsonValueDeserializers[SER_TYPE_PRIM_COUNT]={
  DeserializeUnsigned,
  DeserializeSigned,
  DeserializeFloating,
  DeserializeString,

  DeserializeBool,
  DeserializeString,//password
  DeserializeTime,
  DeserializeDate,
  DeserializeDateTime,
  DeserializeTimespan,
  DeserializeEUI48,
  DeserializeEUI64,
  DeserializeHex,
  DeserializeRHex
};


static const JsonArrayDeserializerFptr_t jsonArrayDeserializers[SER_TYPE_ARRAY_COUNT - SER_TYPE_PRIM_COUNT]={
  DeserializeUnsignedArray,
  DeserializeSignedArray,
  DeserializeFloatingArray,
  DeserializeBoolArray,
  DeserializeHexArray,
  DeserializeRHexArray
};


int SerializeUInt(JsonConvertHandle *hnd, void*obj){ 
    unsigned int * valptr=(unsigned int *)obj;
    return  snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%u",*valptr);
}

int SerializeInt(JsonConvertHandle *hnd, void*obj){
    int * valptr=(int *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%d",*valptr);

}

int SerializeChar(JsonConvertHandle *hnd, void*obj){ 
    signed char * valptr=(char *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%d", *valptr);
}

int SerializeUChar(JsonConvertHandle *hnd, void*obj){ 
    unsigned char * valptr=(unsigned char *)obj;
    return  snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%u",*valptr);
}

int SerializeShort(JsonConvertHandle *hnd, void*obj){
    signed short * valptr=(short *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%d",*valptr);
}

int SerializeUShort(JsonConvertHandle *hnd, void*obj){
    unsigned short * valptr=(unsigned short *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%u",*valptr);
}


int SerializeUnsigned(JsonConvertHandle *hnd, void*obj, int size){
  switch(size){
    case 1:
      return  SerializeUChar(hnd, obj);
    case 2:
      return  SerializeUShort(hnd, obj);
    case 4:
      return  SerializeUInt(hnd, obj);
    case 8:
    break;
  }
  return  JSON_NOT_SUPPORTED;
}

int SerializeSigned(JsonConvertHandle *hnd, void*obj, int size){
   switch(size){
    case 1:
      return  SerializeChar(hnd, obj);
    case 2:
      return  SerializeShort(hnd, obj);
    case 4:
      return  SerializeInt(hnd, obj);
    case 8:
    break;
  }
  return  JSON_NOT_SUPPORTED;
}



int SerializeFloat(JsonConvertHandle *hnd, void*obj){
    float * valptr=(float *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%.3f",*valptr); 
}

int SerializeDouble(JsonConvertHandle *hnd, void*obj){

    double * valptr=(double *)obj;
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"%.3f",*valptr);
}

int SerializeFloating(JsonConvertHandle *hnd, void*obj, int size){
   switch(size){
    case 4:
      return  SerializeFloat(hnd, obj);
    case 8:
      return  SerializeDouble(hnd, obj);
  }
  return  JSON_NOT_SUPPORTED;
}


int SerializeString(JsonConvertHandle *hnd, void*obj,  int size){
    char * valptr=(char *)obj;
    valptr[size-1]=0;
   
    int len;
    if(valptr[0]==0){
       return JsonWriteString(hnd,"null");
    }else{
      return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%s\"",valptr);
    }
}



int SerializeBool(JsonConvertHandle *hnd, void*obj, int size){
    char * valptr=(char *)obj;  
    int len;
    if(*valptr){
        return JsonWriteString(hnd,"true");
    }else{
        return JsonWriteString(hnd,"false");
    }
}


int SerializePassword(JsonConvertHandle *hnd, void*obj, int size){
     return JsonWriteString(hnd,"null");
}


//example "2012-04-23T18:25:43.511Z" 
int SerializeTime(JsonConvertHandle *hnd, void*obj,  int size){
    dateTime_t * dt=(dateTime_t *)obj;
    if(DateTimeIsNULL(dt)){
       return JsonWriteString(hnd,"null");
    }
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%.4u-%.2u-%.2uT%.2u:%.2u:%.2u.000Z\"",1970,1,1,dt->ucHour,dt->ucMin,dt->ucSec);
}

int SerializeDate(JsonConvertHandle *hnd, void*obj,  int size){
    dateTime_t * dt=(dateTime_t *)obj;
    if(DateTimeIsNULL(dt)){
       return JsonWriteString(hnd,"null");
    }
    return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%.4u-%.2u-%.2uT%.2u:%.2u:%.2u.000Z\"",dt->usYear,dt->ucMonth,dt->ucDay);
}

int SerializeDateTime(JsonConvertHandle *hnd, void*obj, int size){
  dateTime_t * dt=(dateTime_t *)obj;
  if(DateTimeIsNULL(dt)){
    return JsonWriteString(hnd,"null");
  }
  return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%.4u-%.2u-%.2uT%.2u:%.2u:%.2u.000Z\"",dt->usYear,dt->ucMonth,dt->ucDay,dt->ucHour,dt->ucMin,dt->ucSec);
}


int SerializeTimeSpan(JsonConvertHandle *hnd, void*obj, int size){
  if(size!=sizeof(timespan_t)) return  JSON_NOT_SUPPORTED;
  timespan_t * timespan=(timespan_t *)obj;
  
  int days=TimespanDays(timespan);

  if(days==0){
      return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%.2u:%.2u:%.2u\"",TimespanHours(timespan),TimespanMinutes(timespan),TimespanSeconds(timespan));
  }else{
      return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%.2u.%.2u:%.2u:%.2u\"",days,TimespanHours(timespan),TimespanMinutes(timespan),TimespanSeconds(timespan));
  }
}



int SerializeEUI48(JsonConvertHandle *hnd, void*obj, int size){
  char * eui48= (char *)obj;
  return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%02x:%02x:%02x:%02x:%02x:%02x\"",eui48[0],eui48[1],eui48[2],eui48[3],eui48[4],eui48[5]);
}

int SerializeEUI64(JsonConvertHandle *hnd, void*obj, int size){
  char * eui64= (char *)obj;
  return snprintf(&hnd->buf[hnd->pos],hnd->buflen + 1 - hnd->pos,"\"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\"",eui64[0],eui64[1],eui64[2],eui64[3],eui64[4],eui64[5],eui64[6],eui64[7]);
}



int SerializeHex(JsonConvertHandle *hnd, void*obj, int size){
  char * byte= (char *)obj;
  int bufsize = (hnd->buflen + 1 - hnd->pos);
  int needed= (size*2)+2;
  if(bufsize<needed) return  needed;
  
  int pos=hnd->pos;
  short shex;
  char *hex;

  hnd->buf[pos++]='\"';
  for(int i=0;i<size;i++){
    shex = BinToHex(byte[i]);
 
    hnd->buf[pos++]=shex>>8;
    hnd->buf[pos++]=shex;
  }  
  hnd->buf[pos++]='\"';
  return needed;
}

int SerializeRHex(JsonConvertHandle *hnd, void*obj, int size){
  char * byte= (char *)obj;
  int bufsize = (hnd->buflen + 1 - hnd->pos);
  int needed= (size*2)+2;
  if(bufsize<needed) return  needed;
  
  int pos=hnd->pos;
  short shex;
  char *hex;

  hnd->buf[pos++]='\"';
  for(int i=size-1;i>=0;i--){
    shex = BinToHex(byte[i]);
    hex = (char *)&shex;
    hnd->buf[pos++]=*hex++;
    hnd->buf[pos++]=*hex++;
  }  
  hnd->buf[pos++]='\"';
  return needed;
}





int SerializeUnsignedArray(JsonConvertHandle *hnd, void*obj, int Length,  int size){
    char * valptr=(char *)obj;    
    JsonResType res;

    for(int i=0; i<Length; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeUnsigned(hnd, valptr,size);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}

int SerializeSignedArray(JsonConvertHandle *hnd, void*obj, int Length,  int size){
    char * valptr=(char *)obj;    
    JsonResType res;

    for(int i=0; i<Length; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeSigned(hnd, valptr,size);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}


int SerializeFloatingArray(JsonConvertHandle *hnd, void*obj, int Length,  int size){
    char * valptr=(char *)obj;    
    JsonResType res;

    for(int i=0; i<Length; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeFloating(hnd, valptr,size);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}


int SerializeBoolArray(JsonConvertHandle *hnd, void*obj, int len, int size){
   char * valptr=(char  *)obj;    
   JsonResType res;

    for(int i=0; i<len; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeBool(hnd, valptr ,0);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}

int SerializeHexArray(JsonConvertHandle *hnd, void*obj, int len, int size){
   char * valptr=(char  *)obj;    
   JsonResType res;

    for(int i=0; i<len; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeHex(hnd, valptr ,size);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}


int SerializeRHexArray(JsonConvertHandle *hnd, void*obj, int len, int size){
   char * valptr=(char  *)obj;    
   JsonResType res;

    for(int i=0; i<len; i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }

      res = SerializeRHex(hnd, valptr ,size);
      res = JsonInternalFlush(hnd, res);
      if(res==JSON_REPEAT_LAST){
        i--;
        continue;
      }else if(res<JSON_OK) return  res;
      
      valptr+=size;
    }

    return JSON_OK;
}

static const JsonArraySerializerFptr_t jsonArrayValueSerializer[SER_TYPE_ARRAY_COUNT-SER_TYPE_PRIM_COUNT]={
  SerializeUnsignedArray,
  SerializeSignedArray,
  SerializeFloatingArray,
  SerializeBoolArray,
  SerializeHexArray,
  SerializeRHexArray
};


int SerializeArray(JsonConvertHandle *hnd, void*obj,int length, int size, int ArrayType){
    JsonResType res;
    ArrayType-=SER_TYPE_PRIM_COUNT;

    if(jsonArrayValueSerializer[ArrayType]==NULL) return  -10;//serializer not supported

    if((res = JsonWriteString(hnd,"["))<0) return  res;

    res = jsonArrayValueSerializer[ArrayType](hnd,obj,length,size);

    if(res<JSON_OK) return res;

    res=JsonWriteString(hnd,"]");

    return  res;
}


static const JsonValueSerializerFptr_t jsonValueSerializer[SER_TYPE_PRIM_COUNT]={
  SerializeUnsigned,
  SerializeSigned,
  SerializeFloating,
  SerializeString,
  
  SerializeBool,
  SerializePassword,
  SerializeTime,
  SerializeDate,
  SerializeDateTime,
  SerializeTimeSpan,
  SerializeEUI48,
  SerializeEUI64,
  SerializeHex,
  SerializeRHex
};





JsonResType JsonSerializeStringPtr(JsonConvertHandle *hnd, void*obj){
    JsonResType res;
    char *dptr=(char*)obj;

    if(dptr==NULL || *dptr==0) return JsonWriteString(hnd,"null");
    
    res = JsonWriteString(hnd, "\""); 
    if(res!=JSON_OK) return  res;

    res = JsonWriteString(hnd, dptr); 
    if(res!=JSON_OK) return  res;

    return   JsonWriteString(hnd, "\"");
}


JsonResType JsonSerializeObjectArray(JsonConvertHandle *hnd, void*obj, int ArraySize, const SerializationInfo_t info){
    JsonResType res;
    unsigned int objsize = StructureSize(info);

    if(info==NULL) return  -10;//serializer not supported

    if((res=JsonWriteString(hnd,"["))<0) return  res;

    for(int i=0;i<ArraySize;i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }
      
      res = JsonSerialize(obj, info, hnd);
      if(res<JSON_OK) return res;
      obj =(void*)( (unsigned int)obj + objsize);
    }

    if(res<JSON_OK) return res;

    res=JsonWriteString(hnd,"]");

    return  res;
}

JsonResType JsonSerializeObjectPtrArray(JsonConvertHandle *hnd, void*obj, int ArraySize, const SerializationInfo_t info){
    JsonResType res;


    if(info==NULL) return  -10;//serializer not supported

    if((res=JsonWriteString(hnd,"["))<0) return  res;

    for(int i=0;i<ArraySize;i++){
      if(i!=0){
        if((res=JsonWriteString(hnd,","))<0) return  res;
      }
      
      res = JsonSerialize(*((void**)obj), info, hnd);
      if(res<JSON_OK) return res;
      obj =(void*)( (unsigned int)obj + sizeof(void *));
    }

    if(res<JSON_OK) return res;

    res=JsonWriteString(hnd,"]");

    return  res;
}

JsonResType JsonDeserializeObjectArray(JsonConvertHandle* hnd,  void *obj,int size, const SerializationInfo_t info){
    int start,len;
    int type = JsonTokenPosition(hnd, &start, &len);
    if( type < JSON_OK ) return type;
    if(type!=JSON_TKN_ARRAY_OPEN) return  -1;//invalid type
    hnd->pos+=len;
    unsigned int objsize = StructureSize(info);
    

    int res;

    for(int i=0;i<size;i++){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}

      res = JsonDeserializeObject(hnd, obj, info);
      if(res) return  res;
      obj = (void*) ((unsigned int)obj + objsize);

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    for(;;){
      type = JsonTokenPosition(hnd, &start, &len);
      if( type < JSON_OK ) return type;
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      if(type!=JSON_TKN_OBJECT_OPEN) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type!=JSON_TKN_OBJECT_CLOSE) return  JSON_INVALID_TYPE;
      hnd->pos+=len;

      type = JsonTokenPosition(hnd, &start, &len);
      if(type == JSON_TKN_ARRAY_CLOSE){hnd->pos+=len; return JSON_OK;}
      else if(type == JSON_TKN_ITEM_SEPARATOR)hnd->pos+=len;
      else return  JSON_TOKEN_ERROR;
    }
    
    return JSON_OK;
}



JsonResType JsonDeserializeObject(JsonConvertHandle* hnd,  void *obj, const SerializationInfo_t info)
{
    int start,len, ResId;
    JsonResType type;
    void *child;
  
    type = JsonTokenType(hnd);
    if(type<0) return type ;
    if(type!=JSON_TKN_OBJECT_OPEN) return type;
    hnd->pos++;

    do
    {
        type = JsonTokenPosition(hnd, &start, &len);
        if(type<0) return type ;  
        if(type==JSON_TKN_OBJECT_CLOSE){
          hnd->pos++;
          return  JSON_OK;
        }
        if(type!=JSON_TKN_STRING) return JSON_TOKEN_ERROR;
       
        
        ResId = FindParameterInfo(info, &hnd->buf[start], len, SER_DIR_W);       

        hnd->pos += len+2;
        type = JsonTokenType(hnd);
        if(type<0) return type ;
        if(type != JSON_TKN_VALUE_SEPARATOR) return type;
        hnd->pos++;

        if (ResId>0)
        {
            type = JsonTokenType(hnd);
            if(type<0) return type ;
            
            
            if(info[ResId].Type == SER_TYPE_OBJECT){
                if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;

                child = (void *)((unsigned int)obj + info[ResId].Offset);
                
                hnd->NestCount++;
                ResId = JsonDeserializeObject(hnd, child, info[ResId].SInfo);
                hnd->NestCount--;
                if (ResId)return ResId;
            
            }else  if(info[ResId].Type == SER_TYPE_OBJECT_ARRRY){
                if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;

                child = (void *)((unsigned int)obj + info[ResId].Offset);
                
                hnd->NestCount++;
                ResId = JsonDeserializeObjectArray(hnd, child,info[ResId].Size, info[ResId].SInfo);
                hnd->NestCount--;
                if (ResId)return ResId;
            }else if(info[ResId].Type >=SER_TYPE_PRIM_COUNT && info[ResId].Type <SER_TYPE_ARRAY_COUNT){
              if(jsonArrayDeserializers[info[ResId].Type]==0) return  JSON_NOT_SUPPORTED;//value deserializer not supported
              
                ResId = jsonArrayDeserializers[info[ResId].Type-SER_TYPE_PRIM_COUNT](hnd, (void*)((unsigned int)obj + info[ResId].Offset),info[ResId].Size, info[ResId].ElementSize, info[ResId].Validator);

                if(ResId==JSON_VALIDATION_ERROR && hnd->Options & JSON_OPT_IGNORE_VALIDATION_ERR) ResId=JSON_OK;
                if (ResId)return ResId;

            }else{
                if(jsonValueDeserializers[info[ResId].Type]==0) return  JSON_NOT_SUPPORTED;//value deserializer not supported
            
                ResId = jsonValueDeserializers[info[ResId].Type](hnd, (void*)((unsigned int)obj + info[ResId].Offset), info[ResId].Size, info[ResId].Validator);

                if(ResId==JSON_VALIDATION_ERROR && hnd->Options & JSON_OPT_IGNORE_VALIDATION_ERR) ResId=JSON_OK;
                if (ResId)return ResId;
            }

        }else{
         //Skip not found elements        
            type = JsonTokenPosition(hnd,&start, &len);
            if(type<0) return  type;

            switch(type){
              case JSON_TKN_OBJECT_OPEN:
                if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
                hnd->NestCount++;
                type = JsonObjectSkip(hnd);
                hnd->NestCount--;
                if(type<0) return  type;
                break;
    
              case JSON_TKN_ARRAY_OPEN:
                if(hnd->NestCount>=hnd->MaxNest) return  JSON_MAX_NEST;
                hnd->NestCount++;
                type = JsonArraySkip(hnd);
                hnd->NestCount--;
                if(type<0) return  type;

              case JSON_TKN_PRIMITIVE:
                hnd->pos+=len;
                break;
    
              case JSON_TKN_STRING:
                hnd->pos+=len+2;
                break;
    
              default:
                return JSON_TOKEN_ERROR;
            }
        }

        type = JsonTokenType(hnd);
        if(type<0) return type ;
        if(type!=JSON_TKN_ITEM_SEPARATOR && type!=JSON_TKN_OBJECT_CLOSE )return  JSON_TOKEN_ERROR;
        if(type==JSON_TKN_ITEM_SEPARATOR) hnd->pos++;  
    }while(1);
}


void JsonInitHandle(JsonConvertHandle *hnd, void *buf, int buflen, int datalen,unsigned int Options){
  memset(hnd, 0, sizeof(JsonConvertHandle));
  hnd->buf=buf;
  hnd->buflen=buflen-1;
  hnd->len=datalen;
  hnd->MaxNest=10;
  hnd->Options=Options;
} 

void JsonHandleRegisterCallback(JsonConvertHandle *hnd, JsonConvertStreamCallback callback, void *arg){
  hnd->StreamX=callback;
  hnd->StreamXArg=arg;
}

JsonResType JsonDeserialize(JsonConvertHandle* hnd,  void *obj, const SerializationInfo_t info)
{
  int type,res;
  if(hnd->buf==NULL || hnd->buflen==0) return  JSON_BUFFER_ERROR;

  type=JsonTokenType(hnd);
  switch(type) {
    case JSON_TKN_OBJECT_OPEN:
      res = JsonDeserializeObject(hnd, obj, info);
      if(res)return res;
      type=JsonTokenType(hnd);
      if(type!=JSON_STREAM_END) return JSON_BODY_ERROR;
      break;

    case JSON_TKN_ARRAY_OPEN:
      return  JSON_NOT_SUPPORTED;
      /*res = JsonDeserializeArray(hnd, obj, info);
      if(res)return res;
      type=JsonTokenType(hnd);
      if(type!=JSON_STREAM_END) return JSON_BODY_ERROR;
      break;*/
    case JSON_TKN_PRIMITIVE:
       return  JSON_NOT_SUPPORTED;
      //null check

    default: return  JSON_NOT_SUPPORTED;
  }  
  return JSON_OK;
}

JsonResType JsonSerialize(void *obj, const SerializationInfo_t info, JsonConvertHandle *hnd)
{
    char *name;
    int type;
    void *objptr;

    int res;
    int repeat=0;

    //null value for nested object
    if(hnd->NestCount>0 && obj==NULL){
       res = JsonWrite(hnd, "null", 4);
       return res;
    }

    res = JsonWrite(hnd, "{", 1);
    if (res < 0) return res;

    if (obj != NULL){
        for (int i = 1;; i++)
        {
            if(info[i].Direction!=0 && info[i].Direction!=SER_DIR_R) continue; 
            
            if (info[i].Name == NULL)
                break; //end

            type = info[i].Type;
            objptr = (void *)((unsigned int)obj + info[i].Offset);

            name = strrchr(info[i].Name, '.');
            if (name == NULL){
                name = (char *)info[i].Name;
            }else{
                name++; //skip dot character
            }

            if(!repeat){
            if ( i > 1)
            {
                res =  JsonWriteString(hnd, ",\"");
                res += JsonWriteString(hnd, name);
                res += JsonWriteString(hnd, "\":");
                if (res < 0) return res; //exception
            }
            else
            {
                res = JsonWriteString(hnd, "\"");
                res = JsonWriteString(hnd, name);
                res = JsonWriteString(hnd, "\":");
                if (res < 0) return res; //exception
            }
            }
            repeat=0;

            if (hnd->pos >= (hnd->buflen))
            {
                res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
                if (res )return JSON_STREAM_WRITE_ERROR;
                hnd->pos = 0;
            } 

          
 
            

            if (type < SER_TYPE_PRIM_COUNT)
            { //primitive serialization
                if (jsonValueSerializer[type] == NULL)
                    return JSON_NOT_SUPPORTED;
                hnd->pos += res = jsonValueSerializer[type](hnd, objptr, info[i].Size);
            }
            else if (type < SER_TYPE_ARRAY_COUNT)
            { //array serialization
                 hnd->NestCount++;
                 res = SerializeArray(hnd, objptr, info[i].Size, info[i].ElementSize ,type);
                 hnd->NestCount--;
                 if(res<JSON_OK) return  res;
            }
            else if(type== SER_TYPE_OBJECT)
            { //object serialization
                hnd->NestCount++;
                res = JsonSerialize(objptr, (SerializationItem *)info[i].SInfo, hnd);
                hnd->NestCount--;
                if (res < 0)
                    return res; //exception
            } else if(type== SER_TYPE_OBJECT_PTR)
            { //object serialization
                hnd->NestCount++;
                res = JsonSerialize(*((void**)objptr), (SerializationItem *)info[i].SInfo, hnd);
                hnd->NestCount--;
                if (res < 0)
                    return res; //exception
            }else if(type== SER_TYPE_OBJECT_ARRRY){
                hnd->NestCount++;
                res = JsonSerializeObjectArray(hnd, objptr, info[i].Size, (SerializationItem *)info[i].SInfo);
                hnd->NestCount--;
                if (res < 0)
                    return res; //exception
            }else if(type== SER_TYPE_OBJECT_ARRRY){
                hnd->NestCount++;
                res = JsonSerializeObjectPtrArray(hnd, objptr, info[i].Size, (SerializationItem *)info[i].SInfo);
                hnd->NestCount--;
                if (res < 0)
                    return res; //exception
            }else if (type==SER_TYPE_STRING_PTR){
                res =  JsonSerializeStringPtr(hnd, *((void**)objptr));
                if (res < 0)
                    return res; //exception
            }else {
              return  JSON_NOT_SUPPORTED;
            }



            if (hnd->pos >= (hnd->buflen))
            {
                if (res > hnd->buflen)
                    return JSON_BUFFER_ERROR; //buffer to small protection
                if (hnd->pos > hnd->buflen){
                    hnd->pos -= res; //fix position
                    repeat=1;
                }
                res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
                if (res != 0)
                    return JSON_STREAM_WRITE_ERROR;
                hnd->pos = 0;
                i--;
                continue;
            }
        }
    }

    res = JsonWriteString(hnd, "}");
    if (res < 0)
        return res; //exception

    if (!(hnd->Options & JSON_OPT_NO_STREAM_ON_EXIT))
    {
        res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
        if (res != 0)
            return JSON_STREAM_WRITE_ERROR;
        hnd->pos = 0;
    }
    else
    {
        return 0;
    }

    if (hnd->pos > hnd->buflen)
        return JSON_BUFFER_ERROR; //exception

    return hnd->pos;
}

JsonResType JsonWrite(JsonConvertHandle *hnd, char *buf, unsigned int len)
{
    for (int i = 0; i < len; i++)
    {
        if (hnd->pos == (hnd->buflen))
        { 
           int res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
           if (res != 0)
               return JSON_STREAM_WRITE_ERROR;
           hnd->pos = 0; 
        }

        hnd->buf[hnd->pos++] = buf[i];
    }

    return JSON_OK;
}
JsonResType JsonWriteString(JsonConvertHandle *hnd, char *str)
{
    for (int i = 0; str[i]!=0; i++)
    {
        if (hnd->pos == (hnd->buflen))
        {
          int res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos, 0);
          if (res)return JSON_STREAM_WRITE_ERROR;
          hnd->pos = 0;
        }

        hnd->buf[hnd->pos++] = str[i];
    }

    return JSON_OK;
}

JsonResType JsonFlush(JsonConvertHandle* hnd){
    int res;
    
    
    if(  hnd->pos != 0){
       res = hnd->StreamX(hnd->StreamXArg, hnd->buf, hnd->pos,0);
       if (res) return JSON_STREAM_WRITE_ERROR;
       hnd->pos = 0;
    }
    
    res = hnd->StreamX(hnd->StreamXArg, 0, 0, 0);
    if (res) return JSON_STREAM_WRITE_ERROR;
   
    return JSON_OK;
}

