#ifndef ENUM_WITH_NAMES
#define ENUM_WITH_NAMES

// Enum value definition.
#define ENUM_VALUE(value, name) value,

// Enum string definition.
#define ENUM_STRING(value, name) name,

// String to enum conversion.
#define ENUM_STRCMP(value, name) if(equals(str,name)) return value;

// Enum declaration.
#define DECLARE_ENUM(EnumType, ENUM_DEF) \
  enum EnumType { \
    ENUM_DEF(ENUM_VALUE) \
  }; \
  String EnumType##_names[]; \
  EnumType Get##EnumType##Value(String str);

// Define enum to string and string to enum conversions
#define DEFINE_ENUM(EnumType,ENUM_DEF) \
  String EnumType##_names[] = { \
    ENUM_DEF(ENUM_STRING) \
  }; \
  \
  EnumType EnumType##_get_value(String str) \
  { \
    ENUM_DEF(ENUM_STRCMP) \
    return (EnumType)0; /* handle input error */ \
  } \

#endif