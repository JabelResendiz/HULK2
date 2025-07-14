; ModuleID = 'program'
source_filename = "program"

@current_stack_depth = private global i32 0

declare void @strcpy(ptr, ptr)

declare void @strcat(ptr, ptr)

declare i64 @strlen(ptr)

declare double @sqrt(double)

declare double @sin(double)

declare double @cos(double)

declare double @log(double)

declare double @exp(double)

declare i32 @rand()

declare double @pow(double, double)

declare double @fmod(double, double)

declare ptr @malloc(i64)

declare i32 @printf(ptr, ...)

declare i32 @puts(ptr)

declare i32 @snprintf(ptr, i64, ptr, ...)

declare i32 @strcmp(ptr, ptr)

declare void @exit(i32)

define i32 @main() {
entry:
  ret i32 0
}
