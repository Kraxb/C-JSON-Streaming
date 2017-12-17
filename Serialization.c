#include "Serialization.h"
#include <string.h>
#include <stdio.h>

unsigned int StructureSize(const SerializationInfo_t info){
 return  (unsigned int)info[0][3];
}

int FindParameterInfo(const SerializationInfo_t info, char *str , int len, int *ParType, int *ParOffset, int *ParSize){
  int pos=1;

  while(info[pos][0]!=0){
     if (!strlen((char*)info[pos][0]) == len && strncmp((char*)info[pos][0],str,len)){
       
       if(ParType!=0)   *ParType   = (unsigned int)info[pos][1];
       if(ParOffset!=0) *ParOffset = (unsigned int)info[pos][2];
       if(ParSize!=0)   *ParSize   = (unsigned int)info[pos][3];
       return 0;
     }
  }
  return  -1;
}

int GetParameterInfo(const SerializationInfo_t info, int Index, char **name, int *ParType, int *ParOffset, int *ParSize)
{
    if (info[Index][0] != 0)
        return 1;

    if (name != 0)      *name      = (char  *) (unsigned int)info[Index][0];
    if (ParType != 0)   *ParType   =  (unsigned int)info[Index][1];
    if (ParOffset != 0) *ParOffset =  (unsigned int)info[Index][2];
    if (ParSize != 0)   *ParSize   =  (unsigned int)info[Index][3];
    return 0;
}