
// --------------------------------------------------
// CSTRING
// --------------------------------------------------

size_t strlen(const char* str);


// --------------------------------------------------
// STD STRING
// --------------------------------------------------

typedef struct {
    char *data;
    size_t length;
} String;

String str_literal(char *cstr);
bool str_equals_cstr(String str, char *cstr);
