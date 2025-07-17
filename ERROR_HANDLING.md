# Sistema de Manejo de Errores Mejorado

## Descripción

Se ha implementado un sistema de manejo de errores mejorado para el lexer y parser que proporciona información detallada sobre la ubicación y naturaleza de los errores, incluyendo:

- **Línea y columna** exactas donde ocurre el error
- **Descripción detallada** del error
- **Contexto del código** alrededor del error
- **Tipos de error** específicos para lexer y parser

## Estructura de Archivos

### Nuevos archivos agregados:

- `lexer/error_handler.h` - Definiciones de estructuras y funciones para manejo de errores
- `lexer/error_handler.c` - Implementación del manejador de errores
- `test_errors.hulk` - Archivo de prueba con errores para demostrar el sistema
- `test_error_system.sh` - Script de prueba del sistema de errores

### Archivos modificados:

- `lexer/lexer_generator.h` - Agregado soporte para manejo de errores
- `lexer/lexer_generator.c` - Implementación de manejo de errores en el lexer
- `parser/ll1_parser.h` - Agregado soporte para manejo de errores
- `parser/ll1_parser.c` - Implementación de manejo de errores en el parser
- `main.c` - Integración del nuevo sistema de errores
- `makefile` - Agregado compilación del nuevo módulo de errores

## Tipos de Errores

### Errores del Lexer (`LexerErrorType`)

- `ERROR_UNKNOWN_CHARACTER` - Carácter no reconocido
- `ERROR_UNTERMINATED_STRING` - Cadena no terminada
- `ERROR_UNTERMINATED_COMMENT` - Comentario no terminado
- `ERROR_INVALID_NUMBER` - Número inválido
- `ERROR_INVALID_ESCAPE_SEQUENCE` - Secuencia de escape inválida
- `ERROR_UNEXPECTED_EOF` - Fin de archivo inesperado

### Errores del Parser (`ParserErrorType`)

- `ERROR_UNEXPECTED_TOKEN` - Token inesperado
- `ERROR_MISSING_TOKEN` - Token faltante
- `ERROR_INVALID_PRODUCTION` - Producción inválida
- `ERROR_UNEXPECTED_EOF` - Fin de archivo inesperado
- `ERROR_SYNTAX_ERROR` - Error de sintaxis general

## Funciones Principales

### Para el Lexer:

```c
// Obtener el último error del lexer
LexerError *lexer_get_last_error(Lexer *lexer);

// Limpiar el último error
void lexer_clear_error(Lexer *lexer);

// Imprimir error con contexto
void lexer_print_error(Lexer *lexer, const char *source_code);
```

### Para el Parser:

```c
// Obtener el último error del parser
ParserError *parser_get_last_error(LL1Parser *parser);

// Limpiar el último error
void parser_clear_error(LL1Parser *parser);

// Imprimir error con contexto
void parser_print_error(LL1Parser *parser, const char *source_code);
```

## Ejemplo de Uso

### Error Léxico:

```
🚨 ERROR LEXER: Carácter desconocido
📍 Ubicación: Línea 3, Columna 9
📝 Descripción: Carácter no reconocido por el lexer
🔍 Lexema: '@'

📄 Contexto del error:
    1 │ // Archivo de prueba con errores
    2 │ 
>>> 3 │ let x = @;  // @ no es un carácter válido
     │         ^ aquí
    4 │ 
    5 │ // Error de sintaxis: falta punto y coma
```

### Error de Sintaxis:

```
🚨 ERROR PARSER: Token inesperado
📍 Ubicación: Línea 6, Columna 12
📝 Descripción: Token inesperado
❌ Se esperaba: ';', pero se encontró: '//'

📄 Contexto del error:
    4 │ 
    5 │ // Error de sintaxis: falta punto y coma
>>> 6 │ let y = 42  // Falta ;
     │            ^ aquí
    7 │ 
    8 │ // Error de sintaxis: paréntesis no balanceados
```

## Compilación

Para compilar el proyecto con el nuevo sistema de manejo de errores:

```bash
make clean
make compile
```

## Pruebas

Para probar el sistema de manejo de errores:

```bash
# En sistemas Unix/Linux
./test_error_system.sh

# O manualmente
./build/HULK test_errors.hulk
```

## Características del Sistema

1. **Información Detallada**: Cada error incluye línea, columna, descripción y contexto
2. **Contexto Visual**: Muestra las líneas alrededor del error con indicador de posición
3. **Tipos Específicos**: Diferentes tipos de error para mejor diagnóstico
4. **Integración Transparente**: No requiere cambios en el código existente
5. **Gestión de Memoria**: Manejo automático de memoria para estructuras de error
6. **Formato Legible**: Salida colorida y bien formateada para fácil lectura

## Mejoras Futuras

- Agregar más tipos de errores específicos
- Implementar recuperación de errores
- Agregar sugerencias de corrección
- Integrar con el sistema de análisis semántico
- Agregar soporte para múltiples errores simultáneos 