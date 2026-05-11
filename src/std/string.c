// --------------------------------------------------
// CSTRING
// --------------------------------------------------

size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}



// --------------------------------------------------
// STD STRING
// --------------------------------------------------

String str_literal(char *cstr) {
    return (String) {.data = cstr, .length = strlen(cstr)};
}

bool str_equals_cstr(String str, char *cstr) {
    size_t i = 0;
    for (; i < str.length; i++) {
        if (str.data[i] != cstr[i]) {
            return false;
        }
    }
    return cstr[i] == '\0';
}
