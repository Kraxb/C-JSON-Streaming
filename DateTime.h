#ifndef __DATETIME__
#define __DATETIME__


typedef struct {
  unsigned char  ucSec;
  unsigned char  ucMin;
  unsigned char  ucHour;
  unsigned char  ucDay;
  unsigned char  ucMonth;
  unsigned short usYear;
}dateTime_t;

#define timespan_t unsigned int




#endif