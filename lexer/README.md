# Generador de Lexer HULK

Este proyecto implementa un **generador de lexer** en C puro, siguiendo la misma estructura que el proyecto original en C++. El generador compila expresiones regulares a autómatas finitos para realizar análisis léxico eficiente.

## Arquitectura

```
Expresiones Regulares → NFA → DFA → Analizador Léxico
```

### Componentes Principales

1. **Parser de Expresiones Regulares** (`regex_parser.c`)
   - Parser recursivo descendente
   - Soporta operadores: `|`, `*`, `+`, `?`
   - Maneja clases de caracteres `[a-z]` y rangos `[0-9]`
   - Soporte para escapes y strings

2. **Autómatas Finitos** (`nfa.c`, `dfa.c`)
   - **NFA**: Autómata finito no determinista
   - **DFA**: Autómata finito determinista
   - Conversión NFA → DFA usando algoritmo de subconjuntos

3. **Generador de Lexer** (`lexer_generator.c`)
   - Compila patrones de tokens en DFA unificado
   - Reconocimiento eficiente de tokens
   - Manejo de errores y posiciones

## Estructura de Archivos

```
lexer/
├── token.h/c              # Definición y manejo de tokens
├── patterns.h             # Patrones de tokens basados en lexer.l
├── nfa.h/c               # Implementación de NFA
├── dfa.h/c               # Implementación de DFA
├── regex_parser.h/c      # Parser de expresiones regulares
├── lexer_generator.h/c   # Generador principal del lexer
├── test_lexer.c          # Programa de prueba
├── Makefile              # Sistema de compilación
└── README.md             # Esta documentación
```

## Tokens Soportados

Basados en `lexer.l`, el generador reconoce:

### Literales
- **Números**: `[0-9]+(\.[0-9]+)?`
- **Strings**: `"([^"\\]|\\.)*"`
- **Booleanos**: `true|false`
- **Identificadores**: `[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*`

### Operadores
- **Aritméticos**: `+`, `-`, `*`, `/`, `%`, `^`
- **Asignación**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `^=`, `&=`, `|=`, `@=`, `:=`
- **Comparación**: `==`, `!=`, `>`, `<`, `>=`, `<=`
- **Lógicos**: `&`, `|`, `!`

### Delimitadores
- **Paréntesis**: `(`, `)`
- **Llaves**: `{`, `}`
- **Puntuación**: `;`, `,`, `.`, `:`

### Palabras Clave
- `function`, `let`, `in`, `if`, `elif`, `else`
- `while`, `as`, `is`, `type`, `inherits`
- `new`, `base`, `for`, `range`, `PI`, `E`

## Compilación y Uso

### Compilar el proyecto
```bash
make
```

### Ejecutar pruebas
```bash
make test
```

### Limpiar archivos generados
```bash
make clean
```

### Ver ayuda
```bash
make help
```

## Ejemplo de Uso

```c
#include "lexer_generator.h"

int main() {
    // Crear lexer
    Lexer* lexer = lexer_create();
    
    // Establecer input
    const char* input = "let x = 42; function add(a, b) { return a + b; }";
    lexer_set_input(lexer, input);
    
    // Procesar tokens
    Token* token;
    while ((token = lexer_next_token(lexer))->type != TOKEN_EOF) {
        printf("Token: %s, Lexeme: %s\n", 
               get_token_name(token->type), token->lexeme);
        destroy_token(token);
    }
    
    // Limpiar
    lexer_destroy(lexer);
    return 0;
}
```

## Ventajas de esta Implementación

1. **Eficiencia**: DFA permite reconocimiento O(n)
2. **Fidelidad**: Patrones coinciden exactamente con `lexer.l`
3. **Flexibilidad**: Fácil agregar nuevos patrones
4. **Portabilidad**: C puro, sin dependencias externas
5. **Manejo de errores**: Detecta caracteres inválidos
6. **Información de posición**: Línea y columna de cada token

## Comparación con Flex

Esta implementación es conceptualmente similar a **Flex**, pero:

- **Ventajas**: Control total, sin dependencias, más educativo
- **Desventajas**: Menos optimizado, requiere más código manual

## Estructura del Algoritmo

1. **Inicialización**: Compilar patrones en DFA unificado
2. **Escaneo**: Procesar input carácter por carácter
3. **Reconocimiento**: DFA determina token más largo
4. **Generación**: Crear token con tipo, lexema y posición

## Extensibilidad

Para agregar nuevos tokens:

1. Agregar tipo en `token.h`
2. Agregar patrón en `patterns.h`
3. Actualizar `get_token_name()` en `token.c`

El generador automáticamente incluirá el nuevo patrón en el DFA unificado. 