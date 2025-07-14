
// visitor_llvm.c

#include "visitor_llvm.h"
#include <stdlib.h>
#include <stdio.h>

LLVMValueRef codegen_accept(LLVMVisitor *visitor, ASTNode *node)
{
    if (!node)
        return 0;

    switch (node->type)
    {
    case AST_PROGRAM:
        return visitor->basic.program(visitor, node);

    case AST_NUM:
        return visitor->basic.number(visitor, node);

    case AST_STRING:
        return visitor->basic.string(visitor, node);

    case AST_VAR:   
        fprintf(stderr,"JABEL RESENDIZ AGUIRRE\n");
        LLVMValueRef c =visitor->basic.variable(visitor, node);
    fprintf(stderr,"2222222222222222222222");
        return c;
    case AST_BOOLEAN:
        return visitor->basic.boolean(visitor, node);

    case AST_BINARY_OP:
        return visitor->expr.binary(visitor, node);

    case AST_ASSIGNMENT:
        //fprintf(stderr, "JABEL RESENDIZ AGUIRRE\n");
        return visitor->expr.assignment(visitor, node);

    case AST_CALL_FUNC:
        fprintf(stderr, "JABEL RESENDIZ AGUIRRE\n");
        return visitor->expr.call_function(visitor, node);

    case AST_BLOCK:
        return visitor->control.block(visitor, node);

    case AST_LET:
        return visitor->control.let_in(visitor, node);

    case AST_IF:
        return visitor->control.conditional(visitor, node);

    case AST_WHILE:
        return visitor->control.while_loop(visitor, node);

    case AST_DECL_FUNC:
        return visitor->control.dec_function(visitor, node);
    
    case AST_TYPE:
        return visitor->types.type_dec(visitor,node);
    
    case AST_INSTANCE:
        return visitor->types.type_instance(visitor,node);
    
    case AST_GETTER:
        if(node->data.binary_op.right->type == AST_CALL_FUNC)
        {
            return visitor->attrs.method_getter(visitor,node);
        }
        return visitor->attrs.attr_getter(visitor,node);
    
    case AST_SETTER:
        return visitor->attrs.attr_setter(visitor,node);
    
    default:
        exit(1);
    }
}