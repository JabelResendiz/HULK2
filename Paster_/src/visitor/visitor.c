
//visitor.c

#include "visitor.h"

void accept(ASTVisitor* visitor,ASTNode* node)
{
    fprintf(stderr, "accept called with node type: %d\n", node ? node->type : -1);
    
    // fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
    if(!node) return;

    fprintf(stderr, "Processing node type: %d\n", node->type);
    
        //fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
    switch (node->type)
    {
    case AST_PROGRAM:
        fprintf(stderr, "Calling visit_program\n");
        //fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
        visitor->basic.program(visitor,node);
        break;
    case AST_NUM:
        fprintf(stderr, "Calling visit_number\n");
        visitor->basic.number(visitor,node);
        break;
    case AST_STRING:
        fprintf(stderr, "Calling visit_string\n");
        visitor->basic.string(visitor,node);
        break;
    case AST_VAR:
        fprintf(stderr, "Calling visit_variable\n");
        //fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
        visitor->basic.variable(visitor,node);
        break;
    case AST_BOOLEAN:
        fprintf(stderr, "Calling visit_boolean\n");
        visitor->basic.boolean(visitor,node);
        break;
    case AST_BINARY_OP:
        fprintf(stderr, "Calling visit_binary_op\n");
        visitor->expr.binary(visitor,node);
        break;
    case AST_ASSIGNMENT:
    case AST_DESTRUCTOR:
        fprintf(stderr, "Calling visit_assignment\n");
        visitor->expr.assignment(visitor,node);
        break;
    case AST_CALL_FUNC:
        fprintf(stderr, "Calling visit_call_function\n");
        visitor->expr.call_function(visitor,node);
        break;
    case AST_BLOCK:
        fprintf(stderr, "Calling visit_block\n");
        visitor->control.block(visitor,node);
        break;
    case AST_LET:
        fprintf(stderr, "Calling visit_let\n");
        visitor->control.let_in(visitor,node);
        return;
    case AST_IF:
        fprintf(stderr, "Calling visit_conditional\n");
        visitor->control.conditional(visitor,node);
        break;
    case AST_WHILE:
        fprintf(stderr, "Calling visit_while_loop\n");
        visitor->control.while_loop(visitor,node);
        break;
    case AST_DECL_FUNC:
        fprintf(stderr, "Calling visit_dec_function\n");
    //fprintf(stderr,"VOY A LLAMAR AL METODO DE DECL_ FUNC\n");
        visitor->control.dec_function(visitor,node);
        break;

    case AST_TYPE:
        fprintf(stderr, "Calling visit_type_dec\n");
    //fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
        visitor->types.type_dec(visitor,node);
        break;
    case AST_INSTANCE:
        fprintf(stderr, "Calling visit_type_instance\n");
       // fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
        visitor->types.type_instance(visitor,node);
        break;
    
    case AST_GETTER:
        fprintf(stderr, "Calling visit_getter\n");
       // fprintf(stderr,"VOY A LLAMAR AL VISITOR DE TYPE\n");
        visitor->attrs.getter(visitor,node);
        break;
    case AST_SETTER:
        fprintf(stderr, "Calling visit_setter\n");
        visitor->attrs.setter(visitor,node);
        break;
        
    default:
        fprintf(stderr, "Unknown node type: %d\n", node->type);
        break;
    }

    fprintf(stderr, "accept completed for node type: %d\n", node->type);
}

