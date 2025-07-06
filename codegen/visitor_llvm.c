
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
    case NODE_PROGRAM:
        return visitor->basic.program(visitor, node);

    case NODE_NUMBER:
        return visitor->basic.number(visitor, node);

    case NODE_STRING:
        return visitor->basic.string(visitor, node);

    case NODE_VARIABLE:   
        fprintf(stderr,"JABEL RESENDIZ AGUIRRE\n");
        LLVMValueRef c =visitor->basic.variable(visitor, node);
    fprintf(stderr,"2222222222222222222222");
        return c;
    case NODE_BOOLEAN:
        return visitor->basic.boolean(visitor, node);

    case NODE_BINARY_OP:
        return visitor->expr.binary(visitor, node);
        
    case NODE_ASSIGNMENT:
        fprintf(stderr, "JABEL RESENDIZ AGUIRRE\n");
        return visitor->expr.assignment(visitor, node);

    case NODE_FUNC_CALL:
        fprintf(stderr, "JABEL RESENDIZ AGUIRRE\n");
        return visitor->expr.call_function(visitor, node);

    case NODE_BLOCK:
        return visitor->control.block(visitor, node);

    case NODE_LET_IN:
        return visitor->control.let_in(visitor, node);

    case NODE_CONDITIONAL:
        return visitor->control.conditional(visitor, node);

    case NODE_LOOP:
        return visitor->control.while_loop(visitor, node);

    case NODE_FUNC_DEC:
        return visitor->control.dec_function(visitor, node);
    
    case NODE_TYPE_DEC:
        return visitor->types.type_dec(visitor,node);
    
    case NODE_TYPE_INST:
        return visitor->types.type_instance(visitor,node);
    
    case NODE_TYPE_GET_ATTR:
        fprintf(stderr,"Estoy en el codegen accept de getter\n");
        if(node->data.op_node.right->type == NODE_FUNC_CALL)
        {
            return visitor->attrs.method_getter(visitor,node);
        }
        return visitor->attrs.attr_getter(visitor,node);
    
    case NODE_TYPE_SET_ATTR:
        return visitor->attrs.attr_setter(visitor,node);
    
    default:
        exit(1);
    }
}