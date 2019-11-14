#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>



signed char HexToBin(char chr){
  if(chr>='0'  && chr<='9' ) return chr-0x30;
  if(chr>='a'  && chr<='f')  return chr-0x57;
  if(chr>='A'  && chr<='F')   return chr-0x37;
  return  -1;
}

short BinToHex(char chr){
   char val;
   short res=0;
   val = chr&0xf;  

   if(val<10) res|='0'+val;
   else       res|='A'+(val-10);
   
   val = (chr>>4)&0xf;  

   if(val<10) res|=('0'+val)<<8;
   else       res|=('A'+(val-10))<<8;

   return  res;
}


int convHexToDec(char chData)
{
    switch (chData)
    {
    case '0':
        return (0);
    case '1':
        return (1);
    case '2':
        return (2);
    case '3':
        return (3);
    case '4':
        return (4);
    case '5':
        return (5);
    case '6':
        return (6);
    case '7':
        return (7);
    case '8':
        return (8);
    case '9':
        return (9);
    case 'a':
    case 'A':
        return (10);
    case 'b':
    case 'B':
        return (11);
    case 'c':
    case 'C':
        return (12);
    case 'd':
    case 'D':
        return (13);
    case 'e':
    case 'E':
        return (14);
    case 'f':
    case 'F':
        return (15);

    default:
        return (0);
    }
}

void convHexStringToDec(char *pchBuffer, char *pchData, unsigned int uiLength)
{
    unsigned int uiCount;

    memset(pchBuffer, 0, uiLength / 2);

    for (uiCount = 0; uiCount < uiLength; uiCount += 2)
    {
        *pchBuffer = convHexToDec(pchData[uiCount]) << 4;
        *pchBuffer += convHexToDec(pchData[uiCount + 1]) & 0xff;

        pchBuffer += 1;
    }
}

void convDecToHexString(char *pchBuffer, char *pchData, unsigned int uiLength)
{
    unsigned int uiCount;

    memset(pchBuffer, 0, uiLength);
    uiLength /= 2;

    for (uiCount = 0; uiCount < uiLength; uiCount++)
    {
        sprintf(&pchBuffer[strlen(pchBuffer)], "%02x", pchData[uiCount]);
    }
}


int validate_email(char *addr)
{
    int len = strlen(addr);

    int i, prev;
    if (len == 0)
        return 0;
    if (len < 6)
        return 0;

    for (i = 0; i < len; i++)
        if (addr[i] == '@')
            break;

    if ((i == 0) || (i >= len))
        return 0; //no char befora @
    prev = i + 1;

    for (; i < len; i++)
        if (addr[i] == '.')
            break;

    if ((i == prev) || (i >= len))
        return 0; //no char after @

    if ((len - (i + 1)) < 2)
        return 0;

    return 1;
}

char * fileNameWithExtension(char *path)
{
  for(int i = strlen(path)-1; i>=0; i--){
    if(path[i] == '\\')return &path[i+1];
  }
  return  path;
}

char * fileNameExtension(char *path)
{
  for(int i = strlen(path)-1; i>=0; i--){
    if(path[i] == '.')return &path[i];
  }
  return  &path[strlen(path)];
}

char * strUpper(char *str){
char *tmp=str;
while (*str!=0){ *str= toupper(*str);str++;}
return tmp;
}

char * strLower(char *str){
char *tmp=str;
while (*str!=0){ *str= tolower(*str);str++;}
return tmp;
}

int CountChar(char *str, char c)
{
    int res = 0;
    for (int i = 0; str[i]!=0; i++)
    {
        if (!strncasecmp(&str[i], &c, 1))res++;
    }

    return res;
}





char *strsep1(char *str,char div)
{
 if( str== (char *)0)return  (char *)0;
 for(int i=0;(i<strlen(str))&&(i<512);i++){
   if(str[i]==div){
     str[i]=0;
     return &str[i+1];
   }
 }
 return (char *)0;
}

int findchar(char *str,int start,char chr)
{
 if( str== (char *)0)return -1;
 for(int i=start;i<strlen(str);i++){
   if(str[i]==chr){
     return (i - start)+1; 
   }
 }
  return 0;
}