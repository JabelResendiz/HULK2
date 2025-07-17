#ifndef CST_TO_AST_H
#define CST_TO_AST_H

#include "../ast/ast.h"
#include "ll1_structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// MACROS PARA DEBUG Y LOGGING
// ============================================================================

// Define colores para stderr
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

// Macros para debugging con colores
#define DEBUG(...) \
    fprintf(stderr, YELLOW "DEBUG:" RESET __VA_ARGS__)

#define ERROR(...) \
    fprintf(stderr, RED "ERROR:" RESET __VA_ARGS__)

#define ACCEPT(...) \
    fprintf(stderr, GREEN "ACCEPT:" RESET __VA_ARGS__)

#define FINISH(...) \
    fprintf(stderr, BLUE "FINISH:" RESET __VA_ARGS__)

// Macro para simplificar parámetros de funciones
#define cst CSTNode* cst_node

// ============================================================================
// FUNCIONES PRINCIPALES DE TRANSFORMACIÓN
// ============================================================================

/**
 * @brief Función principal que transforma un CST completo a AST
 * @param cst_node Nodo raíz del CST (debe ser "Program")
 * @return ASTNode* Nodo raíz del AST o NULL si hay error
 */
ASTNode* cst_to_ast(cst);

/**
 * @brief Transforma una lista de statements del CST a AST
 * @param cst_node Nodo "StmtList" del CST
 * @return ASTNode* Nodo programa con array de statements
 */
ASTNode* stmtList_to_ast(cst);

/**
 * @brief Transforma un statement terminado (con ";") del CST a AST
 * @param cst_node Nodo "TerminatedStmt" del CST
 * @return ASTNode* Nodo statement sin el terminador
 */
ASTNode* terminated_stmt_to_ast(cst);

/**
 * @brief Dispatcher que identifica el tipo de statement y lo transforma
 * @param cst_node Nodo statement del CST
 * @return ASTNode* Nodo AST correspondiente al tipo de statement
 */
ASTNode* stmt_to_ast(cst);

// ============================================================================
// TRANSFORMACIÓN DE STATEMENTS
// ============================================================================

/**
 * @brief Transforma definición de función: function ID(params) { body }
 * @param cst_node Nodo "FunctionDef" del CST
 * @return ASTNode* Nodo función con nombre, parámetros y cuerpo
 */
ASTNode* functionDef_to_ast(cst);

/**
 * @brief Transforma definición de tipo: type ID(params) inherits Parent { body }
 * @param cst_node Nodo "TypeDef" del CST
 * @return ASTNode* Nodo tipo con nombre, herencia y definiciones
 */
ASTNode* typeDef_to_ast(cst);

/**
 * @brief Transforma bucle while: while condition { body }
 * @param cst_node Nodo "WhileStmt" del CST
 * @return ASTNode* Nodo bucle con condición y cuerpo
 */
ASTNode* while_to_ast(cst);

/**
 * @brief Transforma bloque de código: { statements }
 * @param cst_node Nodo "Block" del CST
 * @return ASTNode* Nodo bloque con lista de statements
 */
ASTNode* block_to_ast(cst);

// ============================================================================
// TRANSFORMACIÓN DE EXPRESIONES
// ============================================================================

/**
 * @brief Dispatcher para transformar expresiones del CST a AST
 * @param cst_node Nodo expresión del CST
 * @return ASTNode* Nodo AST correspondiente al tipo de expresión
 */
ASTNode* expr_to_ast(cst);

/**
 * @brief Transforma condicional: if condition then expr else expr
 * @param cst_node Nodo "Conditional" del CST
 * @return ASTNode* Nodo condicional con condición y ramas
 */
ASTNode* conditional_to_ast(cst);

/**
 * @brief Transforma expresión let-in: let declarations in body
 * @param cst_node Nodo "LetIn" del CST
 * @return ASTNode* Nodo let-in con declaraciones y cuerpo
 */
ASTNode* let_in_to_ast(cst);

/**
 * @brief Transforma asignación: let var = expr
 * @param cst_node Nodo "Assignment" del CST
 * @return ASTNode* Nodo asignación con variable y valor
 */
ASTNode* assignment_to_ast(cst);

/**
 * @brief Transforma llamada a función: func(args)
 * @param cst_node Nodo "FunCall" del CST
 * @return ASTNode* Nodo llamada con nombre y argumentos
 */
ASTNode* funCall_to_ast(cst);

// ============================================================================
// TRANSFORMACIÓN DE EXPRESIONES PRIMARIAS
// ============================================================================

/**
 * @brief Transforma literal numérico
 * @param cst_node Nodo "NUMBER" del CST
 * @return ASTNode* Nodo número con valor
 */
ASTNode* number_to_ast(cst);

/**
 * @brief Transforma literal string
 * @param cst_node Nodo "STRING" del CST
 * @return ASTNode* Nodo string con valor
 */
ASTNode* string_to_ast(cst);

/**
 * @brief Transforma literal booleano
 * @param cst_node Nodo "BOOLEAN" del CST
 * @return ASTNode* Nodo booleano con valor
 */
ASTNode* boolean_to_ast(cst);

/**
 * @brief Transforma identificador/variable
 * @param cst_node Nodo "ID" del CST
 * @return ASTNode* Nodo variable con nombre
 */
ASTNode* variable_to_ast(cst);

// ============================================================================
// TRANSFORMACIÓN DE OPERACIONES DE TIPOS
// ============================================================================

/**
 * @brief Transforma casting/testing de tipos: expr as Type, expr is Type
 * @param cst_node Nodo "TypeCasting" del CST
 * @return ASTNode* Nodo casting/testing con expresión y tipo
 */
ASTNode* casting_to_ast(cst);

/**
 * @brief Transforma instanciación de tipo: new Type(args)
 * @param cst_node Nodo "TypeInstance" del CST
 * @return ASTNode* Nodo instancia con tipo y argumentos
 */
ASTNode* instance_to_ast(cst);

/**
 * @brief Transforma acceso a atributo: instance.member
 * @param cst_node Nodo "AttributeGetter" del CST
 * @return ASTNode* Nodo getter con instancia y miembro
 */
ASTNode* getter_to_ast(cst);

/**
 * @brief Transforma asignación de atributo: instance.member = value
 * @param cst_node Nodo "AttributeSetter" del CST
 * @return ASTNode* Nodo setter con instancia, miembro y valor
 */
ASTNode* setter_to_ast(cst);

/**
 * @brief Transforma llamada a base: base(args)
 * @param cst_node Nodo "BaseCall" del CST
 * @return ASTNode* Nodo base con argumentos
 */
ASTNode* base_to_ast(cst);

// ============================================================================
// TRANSFORMACIÓN DE OPERADORES
// ============================================================================

/**
 * @brief Transforma expresión binaria: left op right
 * @param cst_node Nodo "BinaryExpr" del CST
 * @return ASTNode* Nodo operación binaria con operador y operandos
 */
ASTNode* binary_expr_to_ast(cst);

/**
 * @brief Transforma expresión unaria: op operand
 * @param cst_node Nodo "UnaryExpr" del CST
 * @return ASTNode* Nodo operación unaria con operador y operando
 */
ASTNode* unary_expr_to_ast(cst);

/**
 * @brief Procesa producciones primed con operadores (Term', Factor', etc.)
 * @param primed_node Nodo de producción primed
 * @param left_operand Operando izquierdo ya procesado
 * @return ASTNode* Nodo de operación binaria o el operando izquierdo
 */
ASTNode* process_primed_expression(CSTNode* primed_node, ASTNode* left_operand);

// ============================================================================
// FUNCIONES AUXILIARES Y UTILIDADES
// ============================================================================

/**
 * @brief Función de debugging para visualizar estructura del CST
 * @param node Nodo CST a visualizar
 * @param depth Profundidad de indentación
 */
void debug_cst_node(CSTNode* node, int depth);

/**
 * @brief Convierte string de operador a enum Operator
 * @param op_str String del operador ("+", "-", "*", etc.)
 * @return Operator Enum correspondiente al operador
 */
Operator string_to_operator(const char* op_str);

/**
 * @brief Verifica si un nodo CST es un terminal
 * @param node Nodo CST a verificar
 * @return int 1 si es terminal, 0 si no
 */
int is_cst_terminal(CSTNode* node);

/**
 * @brief Verifica si un nodo CST es epsilon (ε)
 * @param node Nodo CST a verificar
 * @return int 1 si es epsilon, 0 si no
 */
int is_cst_epsilon(CSTNode* node);

/**
 * @brief Busca un hijo específico por símbolo en un nodo CST
 * @param parent Nodo padre donde buscar
 * @param symbol Símbolo a buscar
 * @return CSTNode* Primer hijo que coincida o NULL si no se encuentra
 */
CSTNode* find_child_by_symbol(CSTNode* parent, const char* symbol);

/**
 * @brief Cuenta los hijos no terminales de un nodo CST
 * @param node Nodo CST a analizar
 * @return int Número de hijos no terminales
 */
int count_non_terminal_children(CSTNode* node);

/**
 * @brief Extrae el valor de un token de un nodo CST
 * @param node Nodo CST con token
 * @return char* Valor del token o NULL si no existe
 */
char* extract_token_value(CSTNode* node);

/**
 * @brief Procesa una lista de argumentos/parámetros del CST
 * @param arg_list_node Nodo que contiene la lista de argumentos
 * @param args Puntero a array de ASTNode* (salida)
 * @param count Puntero a contador de argumentos (salida)
 * @return int 1 si exitoso, 0 si hay error
 */
int process_argument_list(CSTNode* arg_list_node, ASTNode*** args, int* count);

/**
 * @brief Procesa una lista de parámetros del CST
 * @param param_list_node Nodo que contiene la lista de parámetros
 * @param params Puntero a array de ASTNode* (salida)
 * @param count Puntero a contador de parámetros (salida)
 * @return int 1 si exitoso, 0 si hay error
 */
int process_parameter_list(CSTNode* param_list_node, ASTNode*** params, int* count);

// ============================================================================
// FUNCIONES DE VALIDACIÓN Y MANEJO DE ERRORES
// ============================================================================

/**
 * @brief Valida que un nodo CST tenga la estructura esperada
 * @param node Nodo CST a validar
 * @param expected_symbol Símbolo esperado
 * @param min_children Número mínimo de hijos esperados
 * @return int 1 si válido, 0 si inválido
 */
int validate_cst_node(CSTNode* node, const char* expected_symbol, int min_children);

/**
 * @brief Reporta error de transformación con contexto
 * @param function_name Nombre de la función donde ocurrió el error
 * @param node Nodo CST problemático
 * @param message Mensaje de error
 */
void report_transform_error(const char* function_name, CSTNode* node, const char* message);

/**
 * @brief Limpia memoria de arrays temporales usados en transformación
 * @param arrays Array de punteros a liberar
 * @param count Número de arrays
 */
void cleanup_temp_arrays(void** arrays, int count);

// ============================================================================
// CONFIGURACIÓN Y CONTROL DE DEBUGGING
// ============================================================================

/**
 * @brief Habilita o deshabilita el modo verbose de debugging
 * @param enable 1 para habilitar, 0 para deshabilitar
 */
void set_debug_mode(int enable);

/**
 * @brief Obtiene el estado actual del modo debug
 * @return int 1 si habilitado, 0 si deshabilitado
 */
int get_debug_mode(void);

/**
 * @brief Imprime estadísticas de transformación CST->AST
 * @param nodes_processed Número de nodos procesados
 * @param errors_found Número de errores encontrados
 */
void print_transform_stats(int nodes_processed, int errors_found);

// ============================================================================
// MACROS AUXILIARES PARA TRANSFORMACIÓN
// ============================================================================

/**
 * @brief Macro para validar y procesar nodo CST
 * @param node Nodo a validar
 * @param expected Símbolo esperado
 * @param min_children Mínimo de hijos
 */
#define VALIDATE_AND_PROCESS(node, expected, min_children) \
    do { \
        if (!validate_cst_node(node, expected, min_children)) { \
            ERROR("Validación fallida para %s\n", expected); \
            return NULL; \
        } \
    } while(0)

/**
 * @brief Macro para procesamiento seguro de tokens
 * @param node Nodo con token
 * @param var Variable donde almacenar el valor
 */
#define SAFE_TOKEN_EXTRACT(node, var) \
    do { \
        char* temp = extract_token_value(node); \
        if (temp) { \
            var = strdup(temp); \
        } else { \
            ERROR("Token no encontrado en nodo %s\n", node->symbol); \
            var = NULL; \
        } \
    } while(0)

/**
 * @brief Macro para liberación segura de memoria
 * @param ptr Puntero a liberar
 */
#define SAFE_FREE(ptr) \
    do { \
        if (ptr) { \
            free(ptr); \
            ptr = NULL; \
        } \
    } while(0)

// ============================================================================
// CONSTANTES Y CONFIGURACIÓN
// ============================================================================

// Tamaño máximo de arrays temporales
#define MAX_TEMP_ARRAYS 100

// Tamaño máximo de buffer para mensajes de error
#define MAX_ERROR_MESSAGE 512

// Profundidad máxima de recursión permitida
#define MAX_RECURSION_DEPTH 1000

// ============================================================================
// TIPOS AUXILIARES
// ============================================================================

/**
 * @brief Estructura para manejar contexto de transformación
 */
typedef struct {
    int depth;              // Profundidad actual de recursión
    int nodes_processed;    // Número de nodos procesados
    int errors_found;       // Número de errores encontrados
    int debug_enabled;      // Estado del modo debug
} TransformContext;

/**
 * @brief Estructura para resultado de transformación
 */
typedef struct {
    ASTNode* node;          // Nodo AST resultante
    int success;            // 1 si exitoso, 0 si falló
    char* error_message;    // Mensaje de error si falló
} TransformResult;

// ============================================================================
// FUNCIONES AVANZADAS DE TRANSFORMACIÓN
// ============================================================================

/**
 * @brief Transforma CST a AST con contexto completo
 * @param cst_node Nodo CST a transformar
 * @param context Contexto de transformación
 * @return TransformResult Resultado de la transformación
 */
TransformResult transform_with_context(CSTNode* cst_node, TransformContext* context);

/**
 * @brief Inicializa contexto de transformación
 * @return TransformContext* Contexto inicializado
 */
TransformContext* init_transform_context(void);

/**
 * @brief Libera contexto de transformación
 * @param context Contexto a liberar
 */
void free_transform_context(TransformContext* context);

ASTNode** process_var_binding_list(CSTNode* var_binding_list, int* count);
ASTNode* process_var_binding(CSTNode* var_binding);

ASTNode** process_var_binding_list_tail(CSTNode* tail_node, int* count);

ASTNode* process_let_body(CSTNode* let_body);
ASTNode* primary_to_ast(CSTNode* cst_node);
ASTNode* process_primary_tail(ASTNode* base, CSTNode* primary_tail);
ASTNode** process_arg_list(CSTNode* arg_list, int* count);
ASTNode** process_arg_list_tail(CSTNode* tail_node, int* count);

#endif // CST_TO_AST_H