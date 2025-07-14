
// codegen_basic.c
#include "codegen.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char *process_string_escapes(const char *input)
{
    size_t len = strlen(input);
    char *output = malloc(len + 1); // El resultado podría ser más corto, nunca más largo
    size_t j = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (input[i] == '\\' && i + 1 < len)
        {
            switch (input[i + 1])
            {
            case 'n':
                output[j++] = '\n';
                i++;
                break;
            case 't':
                output[j++] = '\t';
                i++;
                break;
            case '"':
                output[j++] = '"';
                i++;
                break;
            case '\\':
                output[j++] = '\\';
                i++;
                break;
            default:
                output[j++] = input[i];
                break;
            }
        }
        else
        {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
    return output;
}

LLVMValueRef codegen_number(LLVMVisitor *v, ASTNode *node)
{
    return LLVMConstReal(LLVMDoubleType(), node->data.number_value);
}

LLVMValueRef codegen_string(LLVMVisitor *v, ASTNode *node)
{
    char *processed = process_string_escapes(node->data.string_value);
    LLVMValueRef str = LLVMBuildGlobalStringPtr(v->ctx->builder, processed, "str");
    free(processed);
    return str;
}

LLVMValueRef codegen_boolean(LLVMVisitor *v, ASTNode *node)
{
    int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
    return LLVMConstInt(LLVMInt1Type(), value, 0);
}

// LLVMValueRef codegen_variable(LLVMVisitor *v, ASTNode *node)
// {
//     fprintf(stderr, "ESTYOY EN EL CODEGEN DE VARIABLE\n");
//     LLVMValueRef alloca = lookup_variable(node->data.var_name);

//     if (!alloca)
//         exit(1);
//     fprintf(stderr, "ESTYOY EN EL CODEGEN DE VARIABLE\n");
//     LLVMTypeRef var_type = LLVMGetElementType(LLVMTypeOf(alloca));

//     fprintf(stderr, "ESTYOY EN EL CODEGEN DE VARIABLE\n");
//     LLVMValueRef c = LLVMBuildLoad2(v->ctx->builder, var_type, alloca, "load");
//     fprintf(stderr, "111111111111111111111111111111111111LE\n");
//     return c;
// }



LLVMValueRef codegen_variable(LLVMVisitor *v, ASTNode *node)
{

    fprintf(stderr, "estoy en el codegen de varaible\n");

    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    LLVMValueRef alloca = lookup_variable(node->data.var_name);

    if (!alloca)
    {
        fprintf(stderr, "Error: Variable '%s' no declarada\n", node->data.var_name);
        return NULL;
    }

    // LLVMTypeRef var_ptr_type = LLVMTypeOf(alloca);           // alloca es un puntero
    // fprintf(stderr,"UN tremendo problema aqui\n");
    // LLVMTypeRef var_type = LLVMGetElementType(var_ptr_type); // tipo apuntado
    //     fprintf(stderr,"UN tremendo problema aqui\n");
    // LLVMValueRef loaded = LLVMBuildLoad2(builder, var_type, alloca, "tmp");
    // fprintf(stderr,"UN tremendo problema aqui\n");
    
    LLVMTypeRef var_type = type_to_llvm(v,node->computed_type);
    
    LLVMValueRef loaded = LLVMBuildLoad2(builder, var_type, alloca, "tmp");

    fprintf(stderr,"Hasta aqui todo joya\n");
    if (node->computed_type && node->computed_type != NULL)
    {
        fprintf(stderr, "candela\n");

        return loaded;
    }

    else
    {
        LLVMTypeRef alloca_type = LLVMTypeOf(alloca);
        if (!alloca_type)
        {
            return loaded;
        }

        // Obtener el tipo del elemento
        LLVMTypeRef var_type = LLVMGetElementType(alloca_type);
        if (!var_type)
        {
            return loaded;
        }

        // Si el tipo es un puntero o estructura devolver alloca

        LLVMTypeKind type_kind = LLVMGetTypeKind(var_type);
        if (type_kind == LLVMPointerTypeKind &&
            LLVMGetTypeKind(LLVMGetElementType(var_type)) == LLVMIntegerTypeKind &&
            LLVMGetIntTypeWidth(LLVMGetElementType(var_type)) == 8)
        {
            return LLVMBuildLoad2(builder, var_type, alloca, "string_load");
        }

        if (type_kind == LLVMPointerTypeKind || type_kind == LLVMStructTypeKind)
        {
            printf("Debug: Variable %s is a pointer or struct type, returning alloca\n", node->data.var_name);
            return loaded;
        }

        // Intentar hacer el load de manera segura
        printf("Debug: Attempting to load variable %s\n", node->data.var_name);

        // Si el tipo es uno que sabemos que es seguro cargar
        if (type_kind == LLVMDoubleTypeKind ||
            type_kind == LLVMIntegerTypeKind ||
            type_kind == LLVMFloatTypeKind)
        {
            return LLVMBuildLoad2(builder, var_type, alloca, "load");
        }

        // Para strings (que son punteros a char), devolvemos el valor cargado
        if (type_kind == LLVMPointerTypeKind &&
            LLVMGetTypeKind(LLVMGetElementType(var_type)) == LLVMIntegerTypeKind &&
            LLVMGetIntTypeWidth(LLVMGetElementType(var_type)) == 8)
        {
            return LLVMBuildLoad2(builder, var_type, alloca, "string_load");
        }

        // Para cualquier otro tipo, mejor devolver el alloca por seguridad
        printf("Debug: Unknown type for %s, returning alloca\n", node->data.var_name);
        return loaded;
    }
}

LLVMValueRef codegen_assignments(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTOY EN EL CODEGEN DEL ASSIGNMENT\n");
    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    const char *var_name = node->data.binary_op.left->data.var_name;
    LLVMValueRef value = codegen_accept(v, node->data.binary_op.right);

    LLVMTypeRef new_type;
    if (compare_types(node->data.binary_op.right->computed_type, &TYPE_STRING))
    {
        new_type = LLVMPointerType(LLVMInt8Type(), 0);
    }
    else if (compare_types(node->data.binary_op.right->computed_type, &TYPE_BOOLEAN))
    {
        new_type = LLVMInt1Type();
    }
    else
    {
        new_type = LLVMDoubleType();
    }

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(LLVMGetBasicBlockParent(current_block));
    LLVMPositionBuilderAtEnd(builder, entry_block);

    LLVMValueRef existing_alloca = lookup_variable(var_name);
    LLVMValueRef alloca;

    if (existing_alloca)
    {
        fprintf(stderr, "no hay error se ecnotro la variable\n");

        // Para asignación destructiva (:=), actualizar en todos los scopes
        if (node->type == AST_DESTRUCTOR)
        {
            update_variable(var_name, existing_alloca);
        }
        LLVMTypeRef existing_type = LLVMGetElementType(LLVMTypeOf(existing_alloca));
        if (existing_type != LLVMTypeOf(value))
        {
            alloca = LLVMBuildAlloca(builder, new_type, var_name);
            update_variable(var_name, alloca);
        }
        else
        {
            alloca = existing_alloca;
        }
    }
    else
    {
        alloca = LLVMBuildAlloca(builder, new_type, var_name);
        declare_variable(var_name, alloca);
    }

    LLVMPositionBuilderAtEnd(builder, current_block);
    LLVMBuildStore(builder, value, alloca);

    // Para asignación destructiva (:=), retornar el valor asignado
    if (node->type == AST_DESTRUCTOR)
    {
        return value;
    }
    return NULL;
}
