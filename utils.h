#ifndef __UTILS_H__
#define __UTILS_H__



char * strUpper(char *str);
char * strLower(char *str);

void genRandomString(char *pchBuffer, unsigned int uiLength);

int convHexToDec(char chData);
void convHexStringToDec(char *pchBuffer, char *pchData, unsigned int uiLength);
void convDecToHexString(char *pchBuffer, char *pchData, unsigned int uiLength);
signed char HexToBin(char chr);
short BinToHex(char chr);


int validate_email(char *addr);
char * fileNameWithExtension(char *path);
char * fileNameExtension(char *path);
int CountChar(char *str, char c);


char *strsep1(char *str,char div);
int findchar(char *str,int start,char chr);

#endif  // __UTILS_H__
