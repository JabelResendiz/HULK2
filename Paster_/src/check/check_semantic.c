
// check_semantic.c

#include "check_semantic.h"
#include "../type_value/type.h"
#include "../scope/unifiedIndex.h"
#include "../scope/scope.h"
#include <string.h>

// Prototipos de funciones auxiliares de inferencia de tipos
TypeValue* infer_expression_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_binary_op_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_function_call_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_let_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_conditional_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_while_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* infer_block_type_semantic(ASTVisitor* v, ASTNode* node);
TypeValue* lookup_variable_type_semantic(ASTVisitor* v, const char* var_name);
TypeValue* lookup_function_type_semantic(ASTVisitor* v, const char* func_name);

// Función para inferir el tipo de una expresión
TypeValue* infer_expression_type_semantic(ASTVisitor* v, ASTNode* node) {
    if (!node) return &TYPE_ERROR;
    
    // Si ya tiene un tipo computado, usarlo
    if (node->computed_type) {
        return node->computed_type;
    }
    
    // Inferir tipo basado en el tipo de nodo
    switch (node->type) {
        case AST_NUM:
            return &TYPE_NUM;
        case AST_STRING:
            return &TYPE_STRING;
        case AST_BOOLEAN:
            return &TYPE_BOOLEAN;
        case AST_VAR:
            // Para variables, buscar en el scope
            return lookup_variable_type_semantic(v, node->data.var_name);
        case AST_BINARY_OP:
            // Para operaciones binarias, inferir basado en operandos
            return infer_binary_op_type_semantic(v, node);
        case AST_CALL_FUNC:
            // Para llamadas a función, inferir basado en la función
            return infer_function_call_type_semantic(v, node);
        case AST_LET:
            // Para expresiones let, inferir basado en el cuerpo
            return infer_let_type_semantic(v, node);
        case AST_IF:
            // Para expresiones if, unificar tipos de ramas
            return infer_conditional_type_semantic(v, node);
        case AST_WHILE:
            // Para bucles while, tipo del cuerpo
            return infer_while_type_semantic(v, node);
        case AST_BLOCK:
            // Para bloques, tipo de la última expresión
            return infer_block_type_semantic(v, node);
        default:
            return &TYPE_ANY;
    }
}

// Función para inferir tipo de operaciones binarias
TypeValue* infer_binary_op_type_semantic(ASTVisitor* v, ASTNode* node) {
    ASTNode* left = node->data.binary_op.left;
    ASTNode* right = node->data.binary_op.right;
    
    if (!left || !right) return &TYPE_ERROR;
    
    // Visitar operandos para inferir sus tipos
    propagate_env_scope(node, left);
    accept(v, left);
    propagate_env_scope(node, right);
    accept(v, right);
    
    TypeValue* left_type = resolve_node_type(left);
    TypeValue* right_type = resolve_node_type(right);
    
    // Determinar el tipo de resultado basado en el operador
    return resolve_op_type(left_type, right_type, node->data.binary_op.op);
}

// Función para inferir tipo de llamadas a función
TypeValue* infer_function_call_type_semantic(ASTVisitor* v, ASTNode* node) {
    char* func_name = node->data.func_node.name;
    
    // Buscar la función en el scope
    TypeValue* func_type = lookup_function_type_semantic(v, func_name);
    if (!func_type) {
        message_semantic_error(v, "Función '%s' no definida. Línea %d.", func_name, node->line);
        return &TYPE_ERROR;
    }
    
    // Verificar que sea un tipo de función
    if (!compare_types(func_type->super_type, &TYPE_FUNCTION)) {
        message_semantic_error(v, "'%s' no es una función. Línea %d.", func_name, node->line);
        return &TYPE_ERROR;
    }
    
    // Retornar el tipo de retorno de la función
    return func_type->element_type ? func_type->element_type : &TYPE_ANY;
}

// Función para inferir tipo de expresiones let
TypeValue* infer_let_type_semantic(ASTVisitor* v, ASTNode* node) {
    // El tipo de una expresión let es el tipo de su cuerpo
    ASTNode* body = node->data.func_node.body;
    if (!body) return &TYPE_VOID;
    
    propagate_env_scope(node, body);
    accept(v, body);
    
    return resolve_node_type(body);
}

// Función para inferir tipo de expresiones condicionales
TypeValue* infer_conditional_type_semantic(ASTVisitor* v, ASTNode* node) {
    ASTNode* then_branch = node->data.conditional.body;
    ASTNode* else_branch = node->data.conditional.else_branch;
    
    TypeValue* then_type = &TYPE_VOID;
    TypeValue* else_type = &TYPE_VOID;
    
    if (then_branch) {
        propagate_env_scope(node, then_branch);
        accept(v, then_branch);
        then_type = resolve_node_type(then_branch);
    }
    
    if (else_branch) {
        propagate_env_scope(node, else_branch);
        accept(v, else_branch);
        else_type = resolve_node_type(else_branch);
    }
    
    // Unificar tipos de ambas ramas
    return unify_types(then_type, else_type);
}

// Función para inferir tipo de bucles while
TypeValue* infer_while_type_semantic(ASTVisitor* v, ASTNode* node) {
    ASTNode* body = node->data.conditional.body;
    if (!body) return &TYPE_VOID;
    
    propagate_env_scope(node, body);
    accept(v, body);
    
    return resolve_node_type(body);
}

// Función para inferir tipo de bloques
TypeValue* infer_block_type_semantic(ASTVisitor* v, ASTNode* node) {
    if (node->data.program.count == 0) {
        return &TYPE_VOID;
    }
    
    // El tipo de un bloque es el tipo de su última expresión
    ASTNode* last_stmt = node->data.program.statements[node->data.program.count - 1];
    if (!last_stmt) return &TYPE_VOID;
    
    propagate_env_scope(node, last_stmt);
    accept(v, last_stmt);
    
    return resolve_node_type(last_stmt);
}

// Función para buscar el tipo de una variable en el scope
TypeValue* lookup_variable_type_semantic(ASTVisitor* v, const char* var_name) {
    // Buscar en el scope actual - simplificado por ahora
    // TODO: Implementar búsqueda real en el scope
    return &TYPE_ANY;
}

// Función para buscar el tipo de una función en el scope
TypeValue* lookup_function_type_semantic(ASTVisitor* v, const char* func_name) {
    // Buscar en el scope global - simplificado por ahora
    // TODO: Implementar búsqueda real en el scope
    return &TYPE_FUNCTION;
}

// Comento la función unify_types para evitar definición múltiple
/*
TypeValue* unify_types(TypeValue* type1, TypeValue* type2) {
    if (!type1 || !type2) return &TYPE_ERROR;
    if (compare_types(type1, type2)) {
        return type1;
    }
    if (compare_types(type1, &TYPE_ANY)) {
        return type2;
    }
    if (compare_types(type2, &TYPE_ANY)) {
        return type1;
    }
    return compute_lca(type1, type2);
}
*/

/// @brief Crea el visitor del analisis semantico
/// @param node 
/// @return 
int make_checker(ASTNode *node)
{

    fprintf(stderr, "estamos en el checker make de check_semantic\n");
    
    if (!node) {
        fprintf(stderr, "ERROR: node is NULL\n");
        return 1;
    }

    ASTVisitor visitor =
        {
            .basic =
                {
                    .program = visit_program,
                    .number = visit_number,
                    .string = visit_string,
                    .boolean = visit_boolean,
                    .variable = visit_variable},
            .expr =
                {
                    .binary = visit_binary_op,
                    .assignment = visit_assignment,
                    .call_function = visit_call_function},
            .control =
                {
                    .block = visit_block,
                    .let_in = visit_let,
                    .conditional = visit_conditional,
                    .while_loop = visit_while_loop,
                    .dec_function = visit_dec_function},
            .types =
                {
                    .type_dec = visit_type_dec,
                    .type_instance = visit_type_instance,
                },
            .attrs =
                {
                    .getter = visit_getter,
                    .setter = visit_setter
                },
            .error_count = 0,
            .errors = NULL,
            .current_function = NULL,
            .current_type = NULL};

    fprintf(stderr, "About to call accept with visitor\n");
    accept(&visitor, node);
    fprintf(stderr, "accept completed successfully\n");
    
    ERROR* e = error_to_string(visitor.errors,visitor.error_count);
    
    print_error_structure(e);
    
    // Liberar la estructura de errores
    if (e) {
        free_semantic_error(e);
    }
    
    return visitor.error_count;
}


/// @brief Inicializa el entorno global
/// @param v
/// @param node EL nodo del programa
void initialize_program_environment(ASTVisitor *v, ASTNode *node)
{
    if (!node || !node->env)
    {
        return;
    }

    for (int i = 0; i < node->data.program.count; i++)
    {
        ASTNode *child = node->data.program.statements[i];
        
        if (!child)
            continue;

        switch (child->type)
        {
        case AST_DECL_FUNC:
            propagate_env_scope(node,child);

            if (!create_env_item(node->env, child, child->data.func_node.name, 0))
            {
                message_semantic_error(v,"Funcion '%s' ya existe. Line %d." ,child->data.func_node.name, child->line);
            }
            break;

        case AST_TYPE:
            
            propagate_env_scope(node,child);

            if (!create_env_item(node->env, child, child->data.typeDef.name_type, 1))
            {
                message_semantic_error(v,"Type '%s' ya existe. Line %d." ,child->data.typeDef.name_type, child->line);
            }
            break;

        default:
            break;
        }
    }
}



