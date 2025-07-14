#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ast_compat.h"

// Implementación simplificada que mapea a los tipos existentes

Program* create_program(void) {
    ASTNode** statements = (ASTNode**)malloc(sizeof(ASTNode*) * 10); // Inicialmente 10 elementos
    if (!statements) return NULL;
    
    ASTNode* program = create_program_node(statements, 0, AST_PROGRAM);
    return (Program*)program;
}

void program_add_stmt(Program* program, Stmt* stmt) {
    ASTNode* prog_node = (ASTNode*)program;
    // Usar un límite mucho más alto en lugar de array dinámico
    if (prog_node->data.program.count < 1000) { // Límite aumentado a 1000
        prog_node->data.program.statements[prog_node->data.program.count] = (ASTNode*)stmt;
        fprintf(stderr, "[AST-ADD-2] Statement agregado al array: tipo=%d\n", ((ASTNode*)stmt)->type);
        prog_node->data.program.count++;
    } else {
        fprintf(stderr, "[AST-ADD-2] ERROR: Límite de statements alcanzado (1000)\n");
    }
}

void free_program(Program* program) {
    if (program) {
        free_ast((ASTNode*)program);
    }
}

Stmt* create_expr_stmt(Expr* expr) {
    // En el sistema existente, una expresión es directamente un ASTNode
    return (Stmt*)expr;
}

void free_stmt(Stmt* stmt) {
    if (stmt) {
        free_ast((ASTNode*)stmt);
    }
}

Expr* create_variable_expr(const char* name) {
    return (Expr*)create_var_node((char*)name, NULL, 0);
}

Expr* create_number_expr(double value) {
    return (Expr*)create_num_node(value);
}

Expr* create_string_expr(const char* value) {
    return (Expr*)create_string_node((char*)value);
}

Expr* create_bool_expr(bool value) {
    char* bool_str = value ? "true" : "false";
    return (Expr*)create_boolean_node(bool_str);
}

Expr* create_binary_expr(Expr* left, Expr* right, BinaryOp op) {
    // Verificar que ambos operandos sean válidos
    if (!left || !right) {
        fprintf(stderr, "Error: create_binary_expr recibió operandos nulos\n");
        return NULL;
    }
    
    // Mapear BinaryOp a Operator
    Operator operator_map[] = {
        OP_ADD, OP_SUB, OP_MULT, OP_DIV, OP_CONCAT, OP_DCONCAT, OP_POW, OP_MOD,
        OP_AND, OP_OR, OP_NEQ, OP_GT, OP_LT, OP_EQ, OP_GE, OP_LE
    };
    
    Operator mapped_op = (op >= 0 && op < 16) ? operator_map[op] : OP_ADD;
    char* op_name = "unknown";
    
    // Mapear nombres de operadores
    switch (op) {
        case BINARY_OP_PLUS: op_name = "+"; break;
        case BINARY_OP_MINUS: op_name = "-"; break;
        case BINARY_OP_MULT: op_name = "*"; break;
        case BINARY_OP_DIV: op_name = "/"; break;
        case BINARY_OP_MOD: op_name = "%"; break;
        case BINARY_OP_POW: op_name = "^"; break;
        case BINARY_OP_CONCAT: op_name = "++"; break;
        case BINARY_OP_CONCAT_WS: op_name = "+++"; break;
        case BINARY_OP_EQ: op_name = "=="; break;
        case BINARY_OP_NEQ: op_name = "!="; break;
        case BINARY_OP_LESS_THAN: op_name = "<"; break;
        case BINARY_OP_GREATER_THAN: op_name = ">"; break;
        case BINARY_OP_LE: op_name = "<="; break;
        case BINARY_OP_GE: op_name = ">="; break;
        case BINARY_OP_AND: op_name = "&&"; break;
        case BINARY_OP_OR: op_name = "||"; break;
        default: op_name = "unknown"; break;
    }
    
    return (Expr*)create_binary_op_node(mapped_op, op_name, (ASTNode*)left, (ASTNode*)right, NULL);
}

Expr* create_unary_expr(Expr* operand, UnaryOp op) {
    // Para operadores unarios, creamos una operación binaria con un operando nulo
    Operator operator_map[] = {OP_ADD, OP_SUB, OP_NEQ};
    Operator mapped_op = (op >= 0 && op < 3) ? operator_map[op] : OP_ADD;
    
    char* op_name = "unknown";
    switch (op) {
        case UNARY_OP_PLUS: op_name = "+"; break;
        case UNARY_OP_MINUS: op_name = "-"; break;
        case UNARY_OP_NOT: op_name = "!"; break;
        default: op_name = "unknown"; break;
    }
    
    // Crear un nodo binario con el operando izquierdo como el operando unario
    // y el derecho como NULL, pero esto causará problemas en el checker semántico
    // Vamos a crear un nodo especial para operaciones unarias
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->line = line_num;
    node->type = AST_BINARY_OP;
    node->scope = create_scope(NULL);
    node->env = create_env(NULL);
    node->computed_type = NULL;
    node->data.binary_op.name_op = op_name;
    node->data.binary_op.op = mapped_op; 
    node->data.binary_op.left = (ASTNode*)operand;
    node->data.binary_op.right = NULL; // Operación unaria, solo un operando

    return (Expr*)node;
}

Expr* create_call_expr(const char* func_name, Expr** args, size_t arg_count) {
    ASTNode** ast_args = (ASTNode**)malloc(sizeof(ASTNode*) * arg_count);
    if (!ast_args) return NULL;
    
    for (size_t i = 0; i < arg_count; i++) {
        ast_args[i] = (ASTNode*)args[i];
    }
    
    // Duplicar el nombre de la función para evitar problemas de memoria
    char* func_name_copy = func_name ? strdup(func_name) : NULL;
    return (Expr*)create_call_function_node(func_name_copy, ast_args, (int)arg_count);
}

Expr* create_method_call_expr(Expr* object, const char* method_name, Expr** args, size_t arg_count) {
    // Crear un nodo de acceso a propiedad primero
    ASTNode* property_node = create_var_node((char*)method_name, NULL, 0);
    // Nota: access_node se crea pero no se usa en esta implementación simplificada
    // ASTNode* access_node = create_property_access_node((ASTNode*)object, property_node);
    
    // Luego crear la llamada a función
    return (Expr*)create_call_function_node((char*)method_name, (ASTNode**)args, (int)arg_count);
}

Expr* create_get_attr_expr(Expr* object, const char* attr_name) {
    ASTNode* property_node = create_var_node((char*)attr_name, NULL, 0);
    return (Expr*)create_property_access_node((ASTNode*)object, property_node);
}

Expr* create_new_expr(const char* type_name, Expr** args, size_t arg_count) {
    ASTNode** ast_args = (ASTNode**)malloc(sizeof(ASTNode*) * arg_count);
    if (!ast_args) return NULL;
    
    for (size_t i = 0; i < arg_count; i++) {
        ast_args[i] = (ASTNode*)args[i];
    }
    
    return (Expr*)create_struct_instance_node((char*)type_name, ast_args, (int)arg_count);
}

Expr* create_let_expr(VarBinding* bindings, size_t binding_count, Stmt* body) {
    // Crear nodos de asignación para cada binding
    ASTNode** let_bindings = (ASTNode**)malloc(sizeof(ASTNode*) * binding_count);
    if (!let_bindings) return NULL;
    
    for (size_t i = 0; i < binding_count; i++) {
        let_bindings[i] = create_assignment_node(
            bindings[i].name,
            bindings[i].initializer,
            NULL,
            AST_ASSIGNMENT
        );
    }
    
    return (Expr*)create_let_in_node(let_bindings, (int)binding_count, (ASTNode*)body);
}

Expr* create_if_expr(Expr* condition, Stmt* then_body, Stmt* else_body) {
    return (Expr*)create_conditional_node((ASTNode*)condition, (ASTNode*)then_body, (ASTNode*)else_body);
}

Expr* create_while_expr(Expr* condition, Stmt* body) {
    return (Expr*)create_while_node((ASTNode*)condition, (ASTNode*)body);
}

Expr* create_for_expr(const char* var_name, Expr* collection, Stmt* body) {
    // Simplificado: crear un while con una variable de iteración
    // Nota: var_node se crea pero no se usa en esta implementación simplificada
    // ASTNode* var_node = create_var_node((char*)var_name, NULL, 0);
    return (Expr*)create_while_node((ASTNode*)collection, (ASTNode*)body);
}

Expr* create_for_loop_node(const char* var_name, Expr** range_args, ASTNode* body, size_t range_arg_count) {
    // Simplificado: crear un while con una variable de iteración
    // Los range_args se pueden usar para crear la condición del while
    // Nota: var_node se crea pero no se usa en esta implementación simplificada
    // ASTNode* var_node = create_var_node((char*)var_name, NULL, 0);
    
    // Crear una condición simple para el while (siempre true por ahora)
    ASTNode* true_node = create_boolean_node("true");
    
    return (Expr*)create_while_node(true_node, body);
}

Expr* create_is_expr(Expr* object, const char* type_name) {
    // Simplificado: crear una comparación de tipos
    ASTNode* type_node = create_var_node((char*)type_name, NULL, 0);
    return (Expr*)create_binary_op_node(OP_EQ, "is", (ASTNode*)object, type_node, NULL);
}

Expr* create_as_expr(Expr* object, const char* type_name) {
    // Simplificado: crear una asignación de tipo
    // Nota: type_node se crea pero no se usa en esta implementación simplificada
    // ASTNode* type_node = create_var_node((char*)type_name, NULL, 0);
    return (Expr*)create_assignment_node("cast", (ASTNode*)object, (char*)type_name, AST_ASSIGNMENT);
}

Expr* create_base_call_expr(Expr** args, size_t arg_count) {
    return create_call_expr("base", args, arg_count);
}

Expr* create_assign_expr(const char* var_name, Expr* value) {
    return (Expr*)create_assignment_node((char*)var_name, (ASTNode*)value, NULL, AST_ASSIGNMENT);
}

Expr* create_expr_block(Stmt** stmts, size_t stmt_count) {
    ASTNode** ast_stmts = (ASTNode**)malloc(sizeof(ASTNode*) * stmt_count);
    if (!ast_stmts) return NULL;
    
    for (size_t i = 0; i < stmt_count; i++) {
        ast_stmts[i] = (ASTNode*)stmts[i];
    }
    
    return (Expr*)create_program_node(ast_stmts, (int)stmt_count, AST_BLOCK);
}

Stmt* create_block_stmt(Stmt** stmts, size_t stmt_count) {
    ASTNode** ast_stmts = (ASTNode**)malloc(sizeof(ASTNode*) * stmt_count);
    if (!ast_stmts) return NULL;
    
    for (size_t i = 0; i < stmt_count; i++) {
        ast_stmts[i] = (ASTNode*)stmts[i];
    }
    
    return (Stmt*)create_program_node(ast_stmts, (int)stmt_count, AST_BLOCK);
}

Stmt* create_return_stmt_wrapper(Expr* value) {
    return (Stmt*)create_return_stmt((ASTNode*)value);
}

// Funciones para declaraciones (simplificadas)
FunctionDecl* create_function_decl(const char* name, char** params, size_t param_count, 
                                 Stmt* body, TypeValue** param_types, TypeValue* return_type) {
    ASTNode** ast_params = (ASTNode**)malloc(sizeof(ASTNode*) * param_count);
    if (!ast_params) return NULL;
    
    for (size_t i = 0; i < param_count; i++) {
        ast_params[i] = create_var_node(params[i], NULL, 1); // 1 indica parámetro
    }
    
    // Duplicar el nombre de la función
    char* name_copy = name ? strdup(name) : NULL;
    
    // Crear el tipo de retorno como string si está disponible
    char* return_type_name = NULL;
    if (return_type && return_type->name) {
        return_type_name = strdup(return_type->name);
    }
    
    return (FunctionDecl*)create_decl_function_node(
        name_copy, ast_params, (int)param_count, (ASTNode*)body, return_type_name
    );
}

TypeDecl* create_type_decl(const char* name, char** params, size_t param_count,
                          const char* base_type, Expr** base_args, size_t base_arg_count,
                          AttributeDecl** attrs, size_t attr_count,
                          MethodDecl** methods, size_t method_count) {
    // Crear argumentos del constructor
    ASTNode** ast_params = (ASTNode**)malloc(sizeof(ASTNode*) * param_count);
    if (!ast_params) return NULL;
    
    for (size_t i = 0; i < param_count; i++) {
        ast_params[i] = create_var_node(params[i], NULL, 0);
    }
    
    // Crear elementos del cuerpo (atributos y métodos)
    ASTNode** body_elements = (ASTNode**)malloc(sizeof(ASTNode*) * (attr_count + method_count));
    if (!body_elements) {
        free(ast_params);
        return NULL;
    }
    
    int body_count = 0;
    
    // Agregar atributos
    for (size_t i = 0; i < attr_count; i++) {
        body_elements[body_count++] = (ASTNode*)attrs[i];
    }
    
    // Agregar métodos
    for (size_t i = 0; i < method_count; i++) {
        body_elements[body_count++] = (ASTNode*)methods[i];
    }
    
    // Duplicar nombres
    char* name_copy = name ? strdup(name) : NULL;
    char* base_type_copy = base_type ? strdup(base_type) : NULL;
    
    return (TypeDecl*)create_type_node(
        name_copy,
        base_type_copy,
        ast_params,
        (int)param_count,
        NULL, // p_args
        0,    // p_args_count
        body_elements,
        body_count,
        0     // p_constructor
    );
}

AttributeDecl* create_attribute_decl(const char* name, Expr* initializer, TypeInfo* type) {
    // Crear un nodo de asignación como atributo
    char* name_copy = name ? strdup(name) : NULL;
    return (AttributeDecl*)create_assignment_node(name_copy, (ASTNode*)initializer, NULL, AST_ASSIGNMENT);
}

MethodDecl* create_method_decl(const char* name, char** params, size_t param_count,
                              Stmt* body, TypeInfo** param_types, TypeInfo* return_type) {
    ASTNode** ast_params = (ASTNode**)malloc(sizeof(ASTNode*) * param_count);
    if (!ast_params) return NULL;
    
    for (size_t i = 0; i < param_count; i++) {
        ast_params[i] = create_var_node(params[i], NULL, 1); // 1 indica parámetro
    }
    
    char* name_copy = name ? strdup(name) : NULL;
    
    return (MethodDecl*)create_decl_function_node(
        name_copy, ast_params, (int)param_count, (ASTNode*)body, NULL
    );
}

void free_expr(Expr* expr) {
    if (expr) {
        free_ast((ASTNode*)expr);
    }
}

void free_function_decl(FunctionDecl* func) {
    if (func) {
        free_ast((ASTNode*)func);
    }
}

void free_type_decl(TypeDecl* type) {
    if (type) {
        free_ast((ASTNode*)type);
    }
}

void free_attribute_decl(AttributeDecl* attr) {
    if (attr) {
        free_ast((ASTNode*)attr);
    }
}

void free_method_decl(MethodDecl* method) {
    if (method) {
        free_ast((ASTNode*)method);
    }
} 