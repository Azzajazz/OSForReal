// --------------------------------------------------
// CSTRING FUNCTIONS
// --------------------------------------------------

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}



// --------------------------------------------------
// STD STRING FUNCTIONS
// --------------------------------------------------

typedef struct {
    char *data;
    size_t length;
} String;

String str_literal(char *cstr) {
    return (String) {.data = cstr, .length = strlen(cstr)};
}
