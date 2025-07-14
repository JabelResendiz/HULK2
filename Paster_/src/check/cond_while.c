
// cond_while.c

#include "check_semantic.h"
#include "../scope/unifiedIndex.h"
#include <stdio.h>

static int validate_condition(ASTVisitor* v, ASTNode* condition) {
    
    int unified = try_unify_operand(v, condition, &TYPE_BOOLEAN);
    
    if(unified) {
        accept(v, condition);
    }

    // Hallar el tipo de la condition
    TypeValue* type_condition = resolve_node_type(condition);
    
    if(!compare_types(type_condition, &TYPE_BOOLEAN) && 
       !compare_types(type_condition, &TYPE_ERROR)) {
        
        message_semantic_error(v,"La condicion del bloque 'if' debe retornar un 'BOOLEAN' no un %s. Linea %d.",
                            type_condition->name,condition->line);
        
    }
    
    return 1;
}

// Devuelve el tipo computador de la union del cuepro del if con el del else
static TypeValue* validate_body(ASTVisitor* v, ASTNode* body, ASTNode* else_branch)
{
    TypeValue* body_type = resolve_node_type(body);
    TypeValue* else_type = NULL;

    if(else_branch)
    {
        accept(v,else_branch);
        else_type = resolve_node_type(else_branch);

        return compute_lca(body_type,else_type);

    }

    return body_type;

}

void visit_conditional (ASTVisitor* v, ASTNode* node)
{
    // extraer las partes del conditional
    ASTNode* condition = node->data.conditional.cond;
    ASTNode* body = node->data.conditional.body;
    ASTNode* else_branch = node->data.conditional.else_branch;

    // propagar el scope a los hijos
    if (condition) propagate_env_scope(node,condition);
    if (body) propagate_env_scope(node,body);
    if (else_branch) propagate_env_scope(node,else_branch);

    // Visitar la condicion
    if (condition) {
        accept(v,condition);
        validate_condition(v,condition);
    }
    
    // Visitamos el cuerpo
    if (body) accept(v,body);
    
    if (body) {
        node->computed_type = validate_body(v,body,else_branch);
    } else {
        node->computed_type = &TYPE_VOID;
    }

    fprintf(stderr,"el tipo de node es %s\n",node->computed_type->name);
}

void visit_while_loop(ASTVisitor *v, ASTNode *node)
{

    fprintf(stderr,"estoy en el visit de while\n");
    
    ASTNode *condition = node->data.binary_op.left;
    ASTNode *body = node->data.binary_op.right;

    // Heredar scope
    propagate_env_scope(node,condition);
    propagate_env_scope(node,body);

    // Analizar condicion
    accept(v, condition);

    validate_condition(v,condition);
    
    // Analizar cuerpo
    accept(v,body);

    // Tipo del bucle = tipo del cuerpo
    node->computed_type = resolve_node_type(body);

}

// Intentar unificación con BOOLEAN

