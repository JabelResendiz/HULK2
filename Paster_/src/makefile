# Makefile for HulkCompiler

# CORRECCIÓN: Se usa gcc como compilador para código C puro.
CC = gcc

# Los flags de compilación son compatibles con gcc.
CFLAGS = -g -Wall -I. -I/usr/lib/llvm-18/include  -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

# Flags del enlazador
LDFLAGS = -L/usr/lib/llvm-18/lib  -lLLVM-18

# Directorios
SRCDIR = .
OBJDIR = .

# Archivos fuente
# CORRECCIÓN: Se usan los archivos .c para todos los componentes del parser convertidos a C puro.
SRCS = main.c \
       ast/ast.c ast/ast_compat.c ast/basic.c ast/cond_while.c ast/function.c ast/keyword.c ast/oop.c ast/op.c \
       check/check_semantic.c check/cond_while.c check/function.c check/type_semantic.c check/typecheck.c check/variable_semantic.c \
       error/error.c \
       lexer/theoretical_lexer.c \
       llvm/codegen.c llvm/codegen_basic.c llvm/codegen_control.c llvm/codegen_function.c llvm/codegen_op.c llvm/init_codegen.c llvm/typeId.c llvm/visitor_llvm.c \
       llvm/type/declaration.c llvm/type/instance.c llvm/type/getter.c llvm/type/setter.c \
       llvm_scope/llvm_scope.c \
       scope/env.c scope/function.c scope/inheritance.c scope/scope.c scope/unifiedIndex.c scope/usage.c \
       token/token.c \
       type_value/operators.c type_value/type.c \
       utils/print_utils.c \
       visitor/visitor.c \
       automata/dfa.c automata/nfa.c automata/nfa_to_dfa.c \
       Parser/derivation_tree.c \
       Parser/ll1_parser.c

# Los archivos de objeto se derivan de los archivos fuente, todos en C.
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# Nombre del ejecutable
TARGET = hulk_compiler

# Objetivo por defecto
all: $(TARGET)

# Objetivo 'compile' para que coincida con tu comando
compile: all

# Regla para enlazar el ejecutable
$(TARGET): $(OBJS)
	@echo "--> Linking..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "--> Build finished."

# Regla para compilar archivos C
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling C: $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Objetivo para limpiar
clean:
	@echo "Cleaning up..."
	rm -f $(OBJDIR)/*.o $(TARGET)
	@echo "Clean finished."

# Objetivos ficticios
.PHONY: all clean compile
