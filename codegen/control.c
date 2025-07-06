

#include "codegen.h"
#include <stdio.h>
#include <string.h>

LLVMValueRef codegen_conditional(LLVMVisitor *v, ASTNode *node)
{
    // Extraccion de sub-arboles del AST

    fprintf(stderr, "JABEL RESENDIZ COMPILADOR\n");
    ASTNode *cond = node->data.cond_node.cond;
    ASTNode *body = node->data.cond_node.body_true;
    ASTNode *else_branch = node->data.cond_node.body_false;
    // LLVMCoreContext* ctx = v->ctx;
    // LLVMModuleRef module = ctx-> module;
    // LLVMBuilderRef builder = ctx-> builder;

    // Obtencion de la funcion actual que esta generando el codigo
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(v->ctx->builder));

    // Creacion de bloques basicos
    /// bloque para el cuerpo del if
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then");
    LLVMBasicBlockRef else_block = else_branch ? LLVMAppendBasicBlock(current_function, "else") : NULL;

    /// bloque donde concluyen ambos caminos
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");

    // Generacion de codigo para la condicion
    LLVMValueRef cond_val = codegen_accept(v, cond);

    // Creacion de bifurcacion condicional
    /// Creacion de una instruccion LLVM que :
    /*
    1. Salta a then_block si cond_val es verdadera
    2. Salta a else_block o merge_block si es falso o no hay else respectivamente
    */
    LLVMBuildCondBr(v->ctx->builder, cond_val, then_block, else_block ? else_block : merge_block);

    // Generacion del bloque then

    /// Posiciona el builder en then_block
    LLVMPositionBuilderAtEnd(v->ctx->builder, then_block);

    /// Genera el codigo del cuerpo del if
    LLVMValueRef then_val = codegen_accept(v, body);

    /// Agrega un salto incondicional a merge_block
    LLVMBuildBr(v->ctx->builder, merge_block);

    /// Actualiza then_block por su hubo cambios
    then_block = LLVMGetInsertBlock(v->ctx->builder);

    // Generacion del bloque else(si existe)

    LLVMValueRef else_val = NULL;
    if (else_branch)
    {
        LLVMPositionBuilderAtEnd(v->ctx->builder, else_block);
        else_val = codegen_accept(v, else_branch);
        LLVMBuildBr(v->ctx->builder, merge_block);
        else_block = LLVMGetInsertBlock(v->ctx->builder);
    }

    // Preparacion del bloque merge

    /// Posiciona el builder en merge_block para continuar el codegen
    LLVMPositionBuilderAtEnd(v->ctx->builder, merge_block);

    // Manejo de valores de retorno
    if (type_equals(node->return_type, &TYPE_VOID))
    {
        // si el if no produce valor (es void) termina aqui
        return NULL;
    }

    // Creacion de nodo PHI (para valores no void)

    LLVMTypeRef phi_type = type_to_llvm(v->ctx,node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(v->ctx->builder, phi_type, "if_result");

    // Conexion de valores al PHI
    LLVMValueRef incoming_value[] =
        {
            then_val, else_val ? else_val : LLVMConstNull(phi_type)};

    LLVMBasicBlockRef incoming_blocks[] =
        {
            then_block, else_block ? else_block : merge_block};

    LLVMAddIncoming(phi, incoming_value, incoming_blocks, else_block ? 2 : 1);

    fprintf(stderr, "JABEL RESENDIZ COMPILADOR COJOEN\n");

    return phi;
}

LLVMValueRef codegen_while(LLVMVisitor *v, ASTNode *node)
{

    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    // Obtener la función actual
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Determinar el tipo del cuerpo del ciclo
    LLVMTypeRef body_type = type_to_llvm(v->ctx,node->data.op_node.right->return_type);

    // Solo si el cuerpo no es void, reservar una variable temporal para almacenar el valor
    LLVMValueRef result_addr = NULL;
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        result_addr = LLVMBuildAlloca(builder, body_type, "while.result.addr");
        // Inicializamos con un valor nulo del tipo correspondiente
        LLVMBuildStore(builder, LLVMConstNull(body_type), result_addr);
    }

    // Crear bloques básicos: condición, cuerpo y merge
    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_function, "while.cond");
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlock(current_function, "while.body");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "while.end");

    // Saltar al bloque de condición
    LLVMBuildBr(builder, cond_block);

    // Bloque de condición: evalúa la condición del ciclo
    LLVMPositionBuilderAtEnd(builder, cond_block);
    LLVMValueRef cond_val = codegen_accept(v, node->data.op_node.left);
    LLVMBuildCondBr(builder, cond_val, loop_block, merge_block);

    // Bloque del cuerpo del ciclo: se evalúa la expresión del cuerpo
    LLVMPositionBuilderAtEnd(builder, loop_block);
    LLVMValueRef body_val = codegen_accept(v, node->data.op_node.right);
    // Si el cuerpo no es void, se almacena el valor resultante
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        LLVMBuildStore(builder, body_val, result_addr);
    }
    // Volver a la condición para la siguiente iteración
    LLVMBuildBr(builder, cond_block);

    // Bloque de merge: una vez que la condición es falsa, se continúa
    LLVMPositionBuilderAtEnd(builder, merge_block);
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        LLVMValueRef final_val = LLVMBuildLoad2(builder, body_type, result_addr, "while.result");
        return final_val;
    }
    else
    {
        // Si el cuerpo es void, simplemente retornamos NULL
        return NULL;
    }
}

LLVMValueRef codegen_block(LLVMVisitor *v, ASTNode *node)
{
    push_scope();
    LLVMValueRef last = NULL;

    fprintf(stderr, "JABEL RESENDIZ\n");

    for (int i = 0; i < node->data.program_node.count; i++)
    {
        ASTNode *stmt = node->data.program_node.statements[i];
        if (stmt->type != NODE_FUNC_DEC)
        {
            last = codegen_accept(v, stmt);
        }
        fprintf(stderr, "JABEL RESENDIZ\n");
    }

    if(last) pop_scope();

    return last;
}