#ifndef PRINT_UTILS_H
#define PRINT_UTILS_H

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print_info(const char* msg);
void print_success(const char* msg);
void print_error(const char* msg);
void print_warning(const char* msg);
void print_title(const char* msg);

#endif
