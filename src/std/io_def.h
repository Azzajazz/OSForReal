void put_char(char c);

#define fmt_print(...) fmt_print_impl(__VA_ARGS__, 0)
void fmt_print_impl(char *fmt, ...);
