# Generador de Lexer Personalizado para HULK

## Resumen

Este documento describe la implementación de un generador de lexer personalizado en C para el lenguaje HULK, desarrollado como alternativa a Flex/Bison. El lexer utiliza autómatas finitos determinísticos (DFA) y no determinísticos (NFA) para reconocer tokens con prioridad adecuada.

## Arquitectura General

### Componentes Principales

1. **NFA (Autómata Finito No Determinístico)**
   - Representa patrones de tokens como autómatas
   - Permite transiciones epsilon y múltiples caminos
   - Base para la construcción del DFA

2. **DFA (Autómata Finito Determinístico)**
   - Versión determinística del NFA
   - Una transición por símbolo de entrada
   - Eficiente para reconocimiento de tokens

3. **Parser de Expresiones Regulares**
   - Convierte patrones regex a NFA
   - Soporta operadores básicos: concatenación, alternancia, repetición

4. **Generador de Lexer**
   - Combina todos los NFAs en uno solo
   - Convierte NFA combinado a DFA
   - Implementa algoritmo de "longest match" con prioridad

## Estructura de Archivos

```
lexer/
├── nfa.h/nfa.c          # Implementación de NFA
├── dfa.h/dfa.c          # Implementación de DFA
├── regex_parser.h/c     # Parser de expresiones regulares
├── lexer_generator.h/c  # Generador principal del lexer
├── token.h/token.c      # Definición y manejo de tokens
├── patterns.h           # Patrones de tokens del lenguaje
└── README.md           # Documentación específica del lexer
```

## Implementación Detallada

### 1. Autómata Finito No Determinístico (NFA)

#### Estructura de Datos
```c
typedef struct {
    int from;
    int to;
    char symbol;
    bool is_epsilon;
} Transition;

typedef struct {
    int states[MAX_STATES];
    int num_states;
    int start_state;
    int final_states[MAX_STATES];
    TokenType final_token_types[MAX_STATES];
    int num_final_states;
    Transition transitions[MAX_TRANSITIONS];
    int num_transitions;
    char alphabet[MAX_ALPHABET];
    int alphabet_size;
} NFA;
```

#### Funcionalidades Principales
- **Construcción de NFA**: Creación de autómatas para patrones literales y regex
- **Operaciones básicas**: Unión, concatenación, repetición (Kleene star)
- **Estados finales**: Marcado de estados finales con tipos de token

### 2. Autómata Finito Determinístico (DFA)

#### Estructura de Datos
```c
typedef struct {
    int states[MAX_STATES];
    int num_states;
    int start_state;
    int transitions[MAX_STATES][MAX_ALPHABET];
    int final_states[MAX_STATES];
    TokenType final_token_types[MAX_STATES];
    int num_final_states;
    char alphabet[MAX_ALPHABET];
    int alphabet_size;
} DFA;
```

#### Algoritmo de Conversión NFA a DFA
1. **Epsilon-clausura**: Calcular estados alcanzables por transiciones epsilon
2. **Subconjuntos**: Construir estados del DFA como conjuntos de estados del NFA
3. **Transiciones**: Calcular transiciones para cada símbolo del alfabeto
4. **Estados finales**: Asignar tipos de token con prioridad

### 3. Parser de Expresiones Regulares

#### Operadores Soportados
- **Concatenación**: `ab` - secuencia de patrones
- **Alternancia**: `a|b` - uno u otro patrón
- **Repetición**: `a*` - cero o más repeticiones
- **Grupos**: `(a)` - agrupación de patrones

#### Implementación
```c
NFA* regex_parse(const char* pattern, bool is_literal);
```
- Para patrones literales: crea NFA simple con transiciones directas
- Para regex: construye NFA usando operadores de expresiones regulares

### 4. Sistema de Prioridad de Tokens

#### Función de Prioridad
```c
int get_token_priority(TokenType type) {
    switch (type) {
        case TOKEN_FUNCTION: return 0;  // Prioridad más alta
        case TOKEN_LET: return 0;
        // ... otras palabras clave
        case TOKEN_ID: return 5;        // Prioridad más baja
        default: return 6;
    }
}
```

#### Algoritmo de Longest Match con Prioridad
1. Buscar la coincidencia más larga posible
2. Entre coincidencias de igual longitud, elegir la de mayor prioridad
3. Prioridad más alta = número más bajo

### 5. Patrones de Tokens

#### Definición en `patterns.h`
```c
static TokenPattern patterns[] = {
    // Palabras clave (prioridad alta)
    {TOKEN_FUNCTION, "function", 1},
    {TOKEN_LET, "let", 1},
    
    // Identificadores (prioridad baja)
    {TOKEN_ID, "[a-zA-ZñÑ][a-zA-ZñÑ0-9_]*", 0},
    
    // Literales
    {TOKEN_NUMBER, "[0-9]+(\\.[0-9]+)?", 0},
    {TOKEN_STRING, "\"([^\"\\\\]|\\\\.)*\"", 0},
    
    // Operadores y delimitadores
    {TOKEN_PLUS, "+", 1},
    {TOKEN_EQUALS, "=", 1},
    // ...
};
```

## Flujo de Trabajo

### 1. Inicialización del Lexer
```c
void lexer_init(Lexer *lexer) {
    // 1. Compilar patrones a NFAs
    for (int i = 0; i < NUM_PATTERNS; i++) {
        nfas[i] = regex_parse(patterns[i].pattern, patterns[i].is_literal);
    }
    
    // 2. Unir todos los NFAs
    NFA *combined_nfa = nfa_union(nfas[0], nfas[1]);
    // ... unir todos los NFAs
    
    // 3. Convertir a DFA
    DFA *dfa = nfa_to_dfa(combined_nfa);
    
    // 4. Asignar DFA al lexer
    lexer->dfa = dfa;
}
```

### 2. Reconocimiento de Tokens
```c
Token* lexer_next_token(Lexer *lexer) {
    // 1. Saltar espacios en blanco
    while (isspace(lexer->input[lexer->pos])) {
        lexer->pos++;
    }
    
    // 2. Buscar longest match con prioridad
    int current_state = lexer->dfa->start_state;
    int last_accepting_pos = -1;
    TokenType best_token_type = TOKEN_ERROR;
    int best_priority = 999;
    
    while (end_pos < strlen(lexer->input)) {
        // Procesar símbolo por símbolo
        // Actualizar mejor coincidencia encontrada
    }
    
    // 3. Crear y retornar token
    return create_token(best_token_type, value, line, column);
}
```

## Problemas Resueltos

### 1. Gestión de Memoria
- **Problema**: Memory leaks en la asignación de `value.string`
- **Solución**: Implementación correcta de `create_token()` y `destroy_token()`

### 2. Construcción de NFA
- **Problema**: Estados finales no marcados correctamente
- **Solución**: Verificación y marcado de estados finales en construcción

### 3. Prioridad de Tokens
- **Problema**: Palabras clave reconocidas como identificadores
- **Solución**: Corrección de lógica de prioridad en conversión NFA→DFA

### 4. Longest Match
- **Problema**: Reconocimiento de caracteres individuales en lugar de palabras
- **Solución**: Implementación correcta del algoritmo longest match con prioridad

## Integración con el Proyecto

### Modificaciones en `main.c`
```c
#ifdef USE_CUSTOM_LEXER
    // Usar lexer personalizado para debug
    Lexer *lexer = lexer_create();
    lexer_set_input(lexer, input);
    
    printf("[LEXER] Tokens reconocidos en %s:\n", filename);
    Token *token;
    while ((token = lexer_next_token(lexer))->type != TOKEN_EOF) {
        printf("%s: '%s' (línea %d, columna %d)\n", 
               token_type_to_string(token->type), 
               token->value.string, 
               token->line, 
               token->column);
        destroy_token(token);
    }
    lexer_destroy(lexer);
#endif
```

### Modificaciones en `makefile`
```makefile
# Dependencias del ejecutable
$(EXEC): ... $(LEXER_DIR)/token.o $(LEXER_DIR)/nfa.o $(LEXER_DIR)/dfa.o \
         $(LEXER_DIR)/regex_parser.o $(LEXER_DIR)/lexer_generator.o

# Reglas de compilación
$(LEXER_DIR)/token.o: $(LEXER_DIR)/token.c $(LEXER_DIR)/token.h
$(LEXER_DIR)/nfa.o: $(LEXER_DIR)/nfa.c $(LEXER_DIR)/nfa.h
# ...

# Limpieza
clean:
    @rm -f $(LEXER_DIR)/*.o
```

## Ventajas de la Implementación

1. **Control Total**: Implementación desde cero permite control completo sobre el comportamiento
2. **Optimización**: DFA proporciona reconocimiento eficiente O(n)
3. **Flexibilidad**: Fácil modificación de patrones y prioridades
4. **Debugging**: Salida detallada para análisis de tokens
5. **Independencia**: No depende de herramientas externas como Flex

## Limitaciones y Consideraciones

1. **Complejidad**: Implementación más compleja que usar Flex
2. **Mantenimiento**: Requiere más código para mantener
3. **Funcionalidad**: Soporte limitado para expresiones regulares complejas
4. **Integración**: Necesita adaptación del parser para usar tokens del lexer personalizado

## Uso

### Compilación
```bash
make clean
make compile
```

### Ejecución con Debug de Lexer
```bash
# El lexer personalizado se ejecuta automáticamente en modo debug
./build/HULK script.hulk
```

### Salida de Ejemplo
```
[LEXER] Tokens reconocidos en script.hulk:
FUNCTION: 'function' (línea 2, columna 1)
LET: 'let' (línea 3, columna 1)
ID: 'a' (línea 3, columna 5)
EQUALS: '=' (línea 3, columna 7)
NUMBER: '3' (línea 3, columna 9)
```

## Conclusiones

La implementación del generador de lexer personalizado proporciona una base sólida para el análisis léxico del lenguaje HULK. Aunque requiere más desarrollo inicial que usar herramientas estándar, ofrece mayor control y flexibilidad para las necesidades específicas del proyecto.

El sistema de prioridad implementado asegura que las palabras clave sean reconocidas correctamente antes que los identificadores, y el algoritmo de longest match garantiza que se reconozcan las coincidencias más largas posibles. 