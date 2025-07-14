

// typecheck.c

#include "check_semantic.h"
#include "../type_value/type.h"
#include "../scope/unifiedIndex.h"
#include "../scope/scope.h"
#include <string.h>

void visit_program(ASTVisitor *v, ASTNode *node)
{
    if (!node || !node->scope)
    {
        return;
    }
    
    // fprintf(stderr,"ESTOY EN LE VISIT DE PROGRAM \n");
    init(node->scope);
    
    // fprintf(stderr,"ESTOY EN LE VISIT DE PROGRAM \n");
    initialize_program_environment(v, node); // Regsitra funciones/tipos en el contexto global

    for (int i = 0; i < node->data.program.count; i++)
    {
        ASTNode *child = node->data.program.statements[i];
        
        if (!child)
            continue;
            
        propagate_env_scope(node, child);
        accept(v, child);

        if (child->type == AST_ASSIGNMENT)
        {
            node->computed_type = &TYPE_ERROR;
            message_semantic_error(v, "Variable '%s' tiene que ser inicializada primero en un bloque 'let-in'. Line %d.",
                                   child->data.binary_op.left->data.var_name, child->line);
        }
    }
}

void visit_number(ASTVisitor *v, ASTNode *node) {}

void visit_boolean(ASTVisitor *v, ASTNode *node) {}

void visit_string(ASTVisitor *v, ASTNode *node)
{
    const char *str = node->data.string_value;
    int i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == '\\')
        {
            char next = str[i + 1];
            if (next == '\0' || !(
                                    str[i + 1] == 'n' ||
                                    str[i + 1] == 't' ||
                                    str[i + 1] == '\\' ||
                                    str[i + 1] == '\"'))
            {
                message_semantic_error(v, "Secuencia de escape invalida '\\%c'. Line %d", str[i + 1], node->line);
            }
            i++;
        }
        i++;
    }
}

// Las funciones de visita han sido comentadas para evitar definiciones múltiples.
/*
void visit_variable(ASTVisitor *v, ASTNode *node) {
    if (!node) return;
    
    // Inferir el tipo de la variable
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: Variable '%s' inferred type: %s\n", 
            node->data.var_name, inferred_type->name);
}
*/

void visit_binary_op(ASTVisitor *v, ASTNode *node) {
    fprintf(stderr, "Entering visit_binary_op\n");
    
    // Usar inferencia de tipos automática
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: Binary op inferred type: %s\n", inferred_type->name);
}

/*
void visit_call_function(ASTVisitor *v, ASTNode *node) {
    if (!node) return;
    
    // Inferir el tipo de la llamada a función
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: Function call '%s' inferred type: %s\n", 
            node->data.func_node.name, inferred_type->name);
}

void visit_let(ASTVisitor *v, ASTNode *node) {
    if (!node) return;
    
    // Inferir el tipo de la expresión let
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: Let expression inferred type: %s\n", inferred_type->name);
}

void visit_conditional(ASTVisitor *v, ASTNode *node) {
    if (!node) return;
    
    // Inferir el tipo de la expresión condicional
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: Conditional expression inferred type: %s\n", inferred_type->name);
}

void visit_while_loop(ASTVisitor *v, ASTNode *node) {
    if (!node) return;
    
    // Inferir el tipo del bucle while
    TypeValue* inferred_type = infer_expression_type_semantic(v, node);
    node->computed_type = inferred_type;
    
    fprintf(stderr, "DEBUG: While loop inferred type: %s\n", inferred_type->name);
}
*/

void visit_block(ASTVisitor *v, ASTNode *node)
{
    if (!node->checked)
    {
        // si el nodo no ha sido verifica antes,
        // se inicializa el entonro del prgoram
        initialize_program_environment(v, node);
    }

    node->checked = 1;

    // itera por las sentencias del bloque
    ASTNode *curr = NULL;

    for (int i = 0; i < node->data.program.count; i++)
    {
        curr = node->data.program.statements[i];
        if (!curr) continue;
        // propaga el scope y el entorno
        propagate_env_scope(node, curr);

        // visita recursivamente la sentencia
        accept(v, curr);

        if (curr->type == AST_ASSIGNMENT)
        {
            node->computed_type = &TYPE_ERROR;
            message_semantic_error(v, "Variable '%s' tiene que ser inicializada primero en un bloque 'let-in'. Line %d.",
                                   curr->data.binary_op.left->data.var_name, curr->line);
        }
    }

    if (curr)
    {
        // determinacion del tipo resultante
        node->computed_type = resolve_node_type(curr);
        node->usages = add_usages(curr, node->usages);
    }

    else
    {
        // si no hay sentencias
        node->computed_type = &TYPE_VOID;
    }
}


// void visit_unary_op(ASTVisitor* v, ASTNode* node)
// {

// }