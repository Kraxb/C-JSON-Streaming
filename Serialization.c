#include "Serialization.h"
#include <string.h>
#include <stdio.h>

unsigned int StructureSize(const SerializationInfo_t info){
 return  info[0].Size;
}


int FindParameterInfo(const SerializationInfo_t info, char *str, unsigned int len, unsigned int dir){
  for(int i=1; ;i++){   
     if(info[i].Name==NULL) return -1;

     if (info[i].NameLength == len && !strncmp(info[i].Name, str, len)){ 
       if(info[i].Direction!=SER_DIR_RW && info[i].Direction!=dir) continue;
       return i;
     }
  }
  return  -1;
}
