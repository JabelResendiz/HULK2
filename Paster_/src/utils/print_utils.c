#include <stdio.h>
#include "print_utils.h"

void print_banner(const char* icon, const char* color, const char* label, const char* msg) {
    printf("%s\n", color);
    printf("┌────────────────────────────────────────────┐\n");
    printf("│ %s %-8s │ %s\n", icon, label, msg);
    printf("└────────────────────────────────────────────┘\n");
    printf(ANSI_COLOR_RESET);
}

void print_info(const char* msg) {
    print_banner("ℹ️", ANSI_COLOR_CYAN, "INFO", msg);
}

void print_success(const char* msg) {
    print_banner("✅", ANSI_COLOR_GREEN, "SUCCESS", msg);
}

void print_error(const char* msg) {
    print_banner("❌", ANSI_COLOR_RED, "ERROR", msg);
}

void print_warning(const char* msg) {
    print_banner("⚠️", ANSI_COLOR_YELLOW, "WARNING", msg);
}

void print_title(const char* msg) {
    printf(ANSI_COLOR_BLUE);
    printf("\n══════════════════════════════════════════════\n");
    printf("🌳 %s\n", msg);
    printf("══════════════════════════════════════════════\n\n");
    printf(ANSI_COLOR_RESET);
}
