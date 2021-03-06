## C JSON Streaming or buffered Serializer/Deserializer with very small RAM usage
 
 Code is written so that it supports buffered and streamed mode.
 
 You just have to define structure and model for that structure. 
 You can have any number of models for one structure.
 
 Serializatiion reads directly from structure members.
 Deserialization writes directly to structure members.
 
 It is possible to serialize/deserialize directly from and to buffer without callbacks.
 
 Deserializer also supports simple validation.
 
 Buffer must only be big enough to hold left or righ side of parameter plus some bytes for '": ' characters in JSON "left":"right"
 
 Size of simple structure members such as 'int' 'float' 'double' is detected automaticaly. 
 
 Each member also supports simple validation.
 
Size of member is detected automaticaly. For example, there is no need to specify if unsigned is 8bit 16bit or 32bit.
 
## SUPORTED TYPES
```
SER_TYPE_UNSIGNED 
SER_TYPE_SIGNED   
SER_TYPE_FLOAT
SER_TYPE_STRING
SER_TYPE_BOOL
SER_TYPE_PASSWORD
SER_TYPE_TIME
SER_TYPE_DATE
SER_TYPE_DATETIME
SER_TYPE_TIMESPAN
SER_TYPE_EUI_48
SER_TYPE_EUI_64
SER_TYPE_HEX
SER_TYPE_RHEX reversed hex representation bytes are swaped

SER_TYPE_UNSIGNED_ARRAY
SER_TYPE_SIGNED_ARRAY 
SER_TYPE_FLOAT_ARRAY 
SER_TYPE_BOOL_ARRAY
SER_TYPE_HEX_ARRAY
SER_TYPE_RHEX_ARRAY

SER_TYPE_OBJECT 
SER_TYPE_OBJECT_PTR serialization only
SER_TYPE_OBJECT_ARRRY
SER_TYPE_OBJECT_PTR_ARRAY serialization only
SER_TYPE_STRING_PTR
SER_TYPE_TOTAL_COUNT
```
  

## Member mapping

Serializer works with user preconfigured  SerializationInfo_t array of SerializationItem where member mapping is specified.

Struct members are mapped with following macros helpers
 _D is direction specification
 _DN is direction and custom json parameter name specification 

```
SERIALIZATION_START(NAME, NAMESPACE, STRUCTTYPE)
SERIALIZATION_END() 

GENERATE_SER_OBJECT_ARRAY(NAME, CLASSTYPE, SERINFO)
GENERATE_SER_OBJECT_PTR_ARRAY(NAME, CLASSTYPE, SERINFO)
GENERATE_SER_OBJECT(NAME, CLASSTYPE, SERINFO)
GENERATE_SER_OBJECT_PTR(NAME, CLASSTYPE, SERINFO)
GENERATE_SERIALIZABLE_STRING(NAME, CLASSTYPE)
GENERATE_SERIALIZABLE_STRING_D(NAME, CLASSTYPE,DIR)
GENERATE_SERIALIZABLE(NAME, TYPE,  CLASSTYPE)
GENERATE_SERIALIZABLE_D(NAME, TYPE, CLASSTYPE,DIR)
GENERATE_SERIALIZABLE_DN(NAME, ALTNAME, TYPE,  CLASSTYPE,DIR)
GENERATE_SERIALIZABLE_N(NAME,  ALTNAME, TYPE,  CLASSTYPE)
GENERATE_VALIDATE_SERIALIZABLE(NAME, TYPE,  CLASSTYPE,VALIDATOR)
GENERATE_SERIALIZABLE_ARRAY(NAME, TYPE,  CLASSTYPE)
GENERATE_SERIALIZABLE_ARRAY_D(NAME, TYPE,  CLASSTYPE, DIR)


```
## Code Example

```
typedef struct{
  float          Float; 
  timespan_t     Interval;
  int            Number;
}struct_t;


const SerializationInfo_t MODEL1_SERIALIZATION_INFO = {
    SERIALIZATION_START("MODEL1","M", struct_t),
    GENERATE_SERIALIZABLE(Float,        SER_TYPE_FLOAT,  struct_t)  
    GENERATE_SERIALIZABLE(Interval,     SER_TYPE_TIMESPAN,  struct_t)  
    GENERATE_SERIALIZABLE_D(Number,       SER_TYPE_SIGNED,    struct_t,SER_DIR_R)  
    SERIALIZATION_END()
};


const SerializationInfo_t MODEL2_SERIALIZATION_INFO = {
    SERIALIZATION_START("MODEL2","M", struct_t),
    GENERATE_SERIALIZABLE(Float,        SER_TYPE_FLOAT,  struct_t)  

    SERIALIZATION_END()
};



struct_t resobj;
void *arg;
uint8_t buff[255]; 
JsonConvertHandle hnd;

//parameter of 4 is data lenght filled in buffer for buffer preload in streaming(callback configured) or buffered operation

JsonInitHandle(&hnd, buff, 255, 0, 0);
JsonHandleRegisterCallback(&hnd, DeserializeCallback, arg)

JsonResType res = JsonDeserialize(hnd,  &resobj, MODEL1_SERIALIZATION_INFO);

int DeserializeCallback(void*arg, char *buf,int len, int *bread){
   //read any input
   //arg is your optional state
}



JsonInitHandle(&hnd, buff, 255, 0, 0);
JsonHandleRegisterCallback(&hnd, SerializeCallback, arg)

JsonSerialize(obj, MODEL2_SERIALIZATION_INFO, &hnd);

int SerializeCallback(void*arg, char *buf,int len, int *bread){ //bread not used
   //write to any output
    //arg is your optional state
}
```

## License

This software is distributed under MIT license, so feel free to integrate it in your commercial products.
