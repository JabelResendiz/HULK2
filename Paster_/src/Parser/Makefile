CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
INCLUDES = -I../token -I../ast -I../scope -I../type_value -I../error

# Archivos fuente
SOURCES = parser_generator.c generate_parser.c
OBJECTS = $(SOURCES:.c=.o)

# Archivos de dependencias
DEPS = ../token/token.h ../ast/ast.h ../scope/scope.h ../type_value/type.h ../error/error.h

# Objetivo principal
TARGET = generate_parser

# Regla principal
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

# Reglas para objetos
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Limpiar
clean:
	rm -f $(OBJECTS) $(TARGET)

# Ejecutar con gramática de ejemplo
test: $(TARGET)
	./$(TARGET) gramatica.ll parser_generated.c

# Instalar
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Ayuda
help:
	@echo "Makefile para el Generador de Parser LL(1)"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all        - Compilar el generador de parser (por defecto)"
	@echo "  clean      - Limpiar archivos compilados"
	@echo "  test       - Ejecutar con gramática de ejemplo"
	@echo "  install    - Instalar en /usr/local/bin/"
	@echo "  help       - Mostrar esta ayuda"
	@echo ""
	@echo "Uso:"
	@echo "  make                    # Compilar"
	@echo "  make test              # Probar con gramatica.ll"
	@echo "  ./generate_parser gramatica.ll parser.c"

.PHONY: clean test install help 