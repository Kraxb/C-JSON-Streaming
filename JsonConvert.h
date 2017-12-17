#ifndef __JSON_CONVERT__
#define __JSON_CONVERT__

#include "jsmn.h"
#include "serialization.h"


int JsonSerialize(void *obj, const SerializationInfo_t info, char* buf, unsigned int buflen);
int JsonDeserialize(char *str, jsmntok_t *t, int tcnt, void *obj,const SerializationInfo_t info);

#endif