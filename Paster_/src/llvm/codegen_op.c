

// codegen_op.c
#include "codegen.h"
#include <stdio.h>

static LLVMValueRef generate_string_concatenation(LLVMVisitor*v,LLVMValueRef L, LLVMValueRef R, int is_dconcat) {
    // Declarar strlen si no existe
    LLVMTypeRef strlen_type = LLVMFunctionType(LLVMInt64Type(),
        (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 0);
    LLVMValueRef strlen_func = LLVMGetNamedFunction(v->ctx->module, "strlen");

    // Obtener longitudes de los strings
    LLVMValueRef len1 = LLVMBuildCall2(v->ctx->builder, strlen_type, strlen_func, 
        (LLVMValueRef[]){L}, 1, "len1");
    LLVMValueRef len2 = LLVMBuildCall2(v->ctx->builder, strlen_type, strlen_func,
        (LLVMValueRef[]){R}, 1, "len2");

    // Calcular tamaño total
    LLVMValueRef total_len = LLVMBuildAdd(v->ctx->builder, len1, len2, "total_len");
    if (is_dconcat) {
        // Agregar 1 para el espacio en @@
        total_len = LLVMBuildAdd(v->ctx->builder, total_len,
            LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_space");
    }
    // Agregar 1 para el null terminator
    total_len = LLVMBuildAdd(v->ctx->builder, total_len,
        LLVMConstInt(LLVMInt64Type(), 1, 0), "total_len_null");

    // Asignar memoria para el resultado
    LLVMValueRef malloc_func = LLVMGetNamedFunction(v->ctx->module, "malloc");
    LLVMValueRef buffer = LLVMBuildCall2(v->ctx->builder,
        LLVMFunctionType(LLVMPointerType(LLVMInt8Type(), 0),
            (LLVMTypeRef[]){LLVMInt64Type()}, 1, 0),
        malloc_func, (LLVMValueRef[]){total_len}, 1, "buffer");

    // Copiar primer string
    LLVMBuildCall2(v->ctx->builder,
        LLVMFunctionType(LLVMVoidType(),
            (LLVMTypeRef[]){
                LLVMPointerType(LLVMInt8Type(), 0),
                LLVMPointerType(LLVMInt8Type(), 0)
            }, 2, 0),
        LLVMGetNamedFunction(v->ctx->module, "strcpy"),
        (LLVMValueRef[]){buffer, L}, 2, "");

    if (is_dconcat) {
        // Agregar espacio para @@
        LLVMValueRef space_ptr = LLVMBuildGEP2(v->ctx->builder,
            LLVMInt8Type(), buffer,
            (LLVMValueRef[]){len1}, 1, "space_ptr");
        LLVMBuildStore(v->ctx->builder,
            LLVMConstInt(LLVMInt8Type(), ' ', 0),
            space_ptr);

        // Concatenar segundo string después del espacio
        LLVMValueRef after_space = LLVMBuildGEP2(v->ctx->builder,
            LLVMInt8Type(), buffer,
            (LLVMValueRef[]){
                LLVMBuildAdd(v->ctx->builder, len1,
                    LLVMConstInt(LLVMInt64Type(), 1, 0), "")
            }, 1, "after_space");
        LLVMBuildCall2(v->ctx->builder,
            LLVMFunctionType(LLVMVoidType(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 2, 0),
            LLVMGetNamedFunction(v->ctx->module, "strcpy"),
            (LLVMValueRef[]){after_space, R}, 2, "");
    } else {
        // Concatenar directamente para @
        LLVMBuildCall2(v->ctx->builder,
            LLVMFunctionType(LLVMVoidType(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 2, 0),
            LLVMGetNamedFunction(v->ctx->module, "strcat"),
            (LLVMValueRef[]){buffer, R}, 2, "");
    }

    return buffer;
}


LLVMValueRef handle_object_operation(LLVMVisitor *v, LLVMValueRef left, LLVMValueRef right, int op)
{
    // Obtener el tipo ID de los operandos
    LLVMValueRef left_type_id = LLVMBuildExtractValue(v->ctx->builder, left, 0, "left_type_id");
    LLVMValueRef right_type_id = LLVMBuildExtractValue(v->ctx->builder, right, 0, "right_type_id");

    // Crear bloques para el manejo dinámico de tipos
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(v->ctx->builder);
    LLVMValueRef current_function = LLVMGetBasicBlockParent(current_block);

    // Bloque para operación numérica
    LLVMBasicBlockRef number_block = LLVMAppendBasicBlock(current_function, "number_op");
    LLVMBasicBlockRef string_block = LLVMAppendBasicBlock(current_function, "string_op");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");

    // Comparar tipos y bifurcar
    LLVMValueRef is_number = LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, left_type_id,
                                           LLVMConstInt(LLVMInt32Type(), 1, 0), "is_number"); // 1 = NUMBER_TYPE_ID
    LLVMBuildCondBr(v->ctx->builder, is_number, number_block, string_block);

    // Bloque de operación numérica
    LLVMPositionBuilderAtEnd(v->ctx->builder, number_block);
    LLVMValueRef num_result;
    switch (op)
    {
    case OP_ADD:
        num_result = LLVMBuildFAdd(v->ctx->builder, left, right, "add");
        break;
    case OP_SUB:
        num_result = LLVMBuildFSub(v->ctx->builder, left, right, "sub");
        break;
        // ... otros casos ...
    }
    LLVMBuildBr(v->ctx->builder, merge_block);

    // Bloque de operación de string
    LLVMPositionBuilderAtEnd(v->ctx->builder, string_block);
    LLVMValueRef str_result;
    // Manejar operaciones de string
    LLVMBuildBr(v->ctx->builder, merge_block);

    // Bloque de merge
    LLVMPositionBuilderAtEnd(v->ctx->builder, merge_block);
    LLVMValueRef phi = LLVMBuildPhi(v->ctx->builder, LLVMDoubleType(), "result");
    LLVMValueRef incoming_values[] = {num_result, str_result};
    LLVMBasicBlockRef incoming_blocks[] = {number_block, string_block};
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

    return phi;
}

LLVMValueRef codegen_binary_op(LLVMVisitor *v, ASTNode *node)
{
    LLVMValueRef L = codegen_accept(v, node->data.binary_op.left);
    LLVMValueRef R = codegen_accept(v, node->data.binary_op.right);

    // Manejo de operaciones con strings (concatenación)
    if (node->data.binary_op.op == OP_CONCAT || node->data.binary_op.op == OP_DCONCAT) {
        // Convertir números a strings si es necesario
        if (compare_types(node->data.binary_op.left->computed_type, &TYPE_NUM)) {
            LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMInt64Type(),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 3, 1);
            LLVMValueRef snprintf_func = LLVMGetNamedFunction(v->ctx->module, "snprintf");

            // Buffer para el número
            LLVMValueRef num_buffer = LLVMBuildAlloca(v->ctx->builder,
                LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");
            LLVMValueRef buffer_ptr = LLVMBuildBitCast(v->ctx->builder, num_buffer,
                LLVMPointerType(LLVMInt8Type(), 0), "buffer_cast");

            LLVMValueRef format = LLVMBuildGlobalStringPtr(v->ctx->builder, "%g", "num_format");
            LLVMBuildCall2(v->ctx->builder, snprintf_type, snprintf_func,
                (LLVMValueRef[]){
                    buffer_ptr,
                    LLVMConstInt(LLVMInt64Type(), 32, 0),
                    format,
                    L
                }, 4, "");

            L = buffer_ptr;
        }

        if (compare_types(node->data.binary_op.right->computed_type, &TYPE_NUM)) {
            LLVMTypeRef snprintf_type = LLVMFunctionType(LLVMInt32Type(),
                (LLVMTypeRef[]){
                    LLVMPointerType(LLVMInt8Type(), 0),
                    LLVMInt64Type(),
                    LLVMPointerType(LLVMInt8Type(), 0)
                }, 3, 1);
            LLVMValueRef snprintf_func = LLVMGetNamedFunction(v->ctx->module, "snprintf");

            LLVMValueRef num_buffer = LLVMBuildAlloca(v->ctx->builder,
                LLVMArrayType(LLVMInt8Type(), 32), "num_buffer");
            LLVMValueRef buffer_ptr = LLVMBuildBitCast(v->ctx->builder, num_buffer,
                LLVMPointerType(LLVMInt8Type(), 0), "buffer_cast");

            LLVMValueRef format = LLVMBuildGlobalStringPtr(v->ctx->builder, "%g", "num_format");
            LLVMBuildCall2(v->ctx->builder, snprintf_type, snprintf_func,
                (LLVMValueRef[]){
                    buffer_ptr,
                    LLVMConstInt(LLVMInt64Type(), 32, 0),
                    format,
                    R
                }, 4, "");

            R = buffer_ptr;
        }

        return generate_string_concatenation(v,L, R, node->data.binary_op.op == OP_DCONCAT);
    }

    // Si los operandos son números
    if (compare_types(node->data.binary_op.left->computed_type, &TYPE_NUM))
    {
        switch (node->data.binary_op.op)
        {
        case OP_ADD:
            return LLVMBuildFAdd(v->ctx->builder, L, R, "add_tmp");
        case OP_SUB:
            return LLVMBuildFSub(v->ctx->builder, L, R, "sub_tmp");
        case OP_MULT:
            return LLVMBuildFMul(v->ctx->builder, L, R, "mul_tmp");
        case OP_DIV:
            return LLVMBuildFDiv(v->ctx->builder, L, R, "div_tmp");
        case OP_MOD:
        {
            LLVMTypeRef fmod_type = LLVMFunctionType(LLVMDoubleType(),
                                                     (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
            LLVMValueRef fmod_func = LLVMGetNamedFunction(v->ctx->module, "fmod");
            return LLVMBuildCall2(v->ctx->builder, fmod_type, fmod_func,
                                  (LLVMValueRef[]){L, R}, 2, "mod_tmp");
        }
        case OP_POW:
        {
            LLVMTypeRef pow_type = LLVMFunctionType(LLVMDoubleType(),
                                                    (LLVMTypeRef[]){LLVMDoubleType(), LLVMDoubleType()}, 2, 0);
            LLVMValueRef pow_func = LLVMGetNamedFunction(v->ctx->module, "pow");
            return LLVMBuildCall2(v->ctx->builder, pow_type, pow_func,
                                  (LLVMValueRef[]){L, R}, 2, "pow_tmp");
        }
        case OP_EQ:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealOEQ, L, R, "eq_tmp");
        case OP_NEQ:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealONE, L, R, "neq_tmp");
        case OP_GT:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealOGT, L, R, "gt_tmp");
        case OP_GE:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealOGE, L, R, "ge_tmp");
        case OP_LT:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealOLT, L, R, "lt_tmp");
        case OP_LE:
            return LLVMBuildFCmp(v->ctx->builder, LLVMRealOLE, L, R, "le_tmp");
        default:
            break;
        }
    }

    // Operadores lógicos
    if (compare_types(node->data.binary_op.left->computed_type, &TYPE_BOOLEAN))
    {
        fprintf(stderr, "JABEL RESENDIZ binary operador");
        switch (node->data.binary_op.op)
        {
        case OP_AND:
            return LLVMBuildAnd(v->ctx->builder, L, R, "and_tmp");
        case OP_OR:
            return LLVMBuildOr(v->ctx->builder, L, R, "or_tmp");
        default:
            break;
        }
    }

    // Comparación de strings
    if (compare_types(node->data.binary_op.left->computed_type, &TYPE_STRING))
    {
        LLVMTypeRef strcmp_type = LLVMFunctionType(LLVMInt32Type(),
                                                   (LLVMTypeRef[]){
                                                       LLVMPointerType(LLVMInt8Type(), 0),
                                                       LLVMPointerType(LLVMInt8Type(), 0)},
                                                   2, 0);
        LLVMValueRef strcmp_func = LLVMGetNamedFunction(v->ctx->module, "strcmp");

        LLVMValueRef cmp = LLVMBuildCall2(v->ctx->builder, strcmp_type, strcmp_func,
                                          (LLVMValueRef[]){L, R}, 2, "strcmp_tmp");

        switch (node->data.binary_op.op)
        {
        case OP_EQ:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_eq_tmp");
        case OP_NEQ:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntNE, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_neq_tmp");
        case OP_GT:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntSGT, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_gt_tmp");
        case OP_GE:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntSGE, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_ge_tmp");
        case OP_LT:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntSLT, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_lt_tmp");
        case OP_LE:
            return LLVMBuildICmp(v->ctx->builder, LLVMIntSLE, cmp,
                                 LLVMConstInt(LLVMInt32Type(), 0, 0), "str_le_tmp");
        default:
            break;
        }
    }

    fprintf(stderr, "SERES HUMANOS\n");

    // Manejo de operaciones con objetos
    if (compare_types(node->data.binary_op.left->computed_type, &TYPE_OBJ) ||
        compare_types(node->data.binary_op.right->computed_type, &TYPE_OBJ))
    {

        fprintf(stderr, "SERES HUMANOS\n");

        return handle_object_operation(v, L, R, node->data.binary_op.op);
    }

    exit(1);
}

LLVMValueRef codegen_let_in(LLVMVisitor *v, ASTNode *node)
{

    fprintf(stderr, "ESTYOY EN EL CODEGEN DE LET_IN\n");
    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    push_scope();
    // Procesar las declaraciones
    ASTNode **declarations = node->data.func_node.args;
    int dec_count = node->data.func_node.arg_count;

    // Evaluar y almacenar cada declaración
    for (int i = 0; i < dec_count; i++)
    {
        ASTNode *decl = declarations[i];
        const char *var_name = decl->data.binary_op.left->data.var_name;
        LLVMValueRef value = codegen_accept(v, decl->data.binary_op.right);
        if (!value) return NULL;
        
        // Crear alloca para la variable
        LLVMTypeRef var_type = type_to_llvm(v,decl->data.binary_op.right->computed_type);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, var_type, var_name);
        LLVMBuildStore(builder, value, alloca);
        declare_variable(var_name, alloca);
    }

    

    // Obtener el bloque actual antes de evaluar el cuerpo
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);

    // Evaluar el cuerpo
    LLVMValueRef result = codegen_accept(v, node->data.func_node.body);

    // Asegurar que estamos en el bloque correcto después de la evaluación
    LLVMPositionBuilderAtEnd(builder, LLVMGetInsertBlock(builder));

    pop_scope();
    return result;
}

