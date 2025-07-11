
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

LLVMValueRef codegen_variable(LLVMVisitor *v, ASTNode *node)
{

    fprintf(stderr, RED "ESTO EN EL CODEGEN_VARIABLE\n" RESET);

    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    const char *var_name = node->data.variable_name;

    fprintf(stderr, "El nombre de mi variable es %s\n", var_name);

    // --- Special handling for 'self' ---
    // 'self' is the first argument to any method.
    // We need to check if we're currently inside a method.
    // Assuming 'v->current_function' holds the LLVMValueRef of the function being built.
    // You might need to add a 'current_function' field to your LLVMVisitor or LLVMCoreContext.
    if (strcmp(var_name, "self") == 0)
    {
        // Ensure we are in a function context. If not, it's an error (e.g., 'self' outside a method).
        if (!v->current_function)
        { // You need to track the current function being generated
            fprintf(stderr, RED "Error: 'self' utilizado fuera del contexto de una función de clase.\n" RESET);
            return NULL;
        }
        // 'self' is always the first parameter (index 0)
        LLVMValueRef self_param = LLVMGetParam(v->current_function, 0);
        // If your 'self' is a pointer, you'll just return it.
        // If your language uses value semantics for 'self', you might need to load it if it's an alloca.
        // But for OOP, 'self' is typically a pointer passed by value.
        fprintf(stderr, YELLOW "DEBUG: Devolviendo el parámetro 'self' (%s).\n" RESET, LLVMPrintValueToString(self_param));
        return self_param;
    }

    // --- Original lookup for other variables ---
    LLVMValueRef alloca = lookup_variable(var_name);

    if (!alloca)
    {
        fprintf(stderr, "Error: Variable '%s' no declarada\n", node->data.variable_name);
        return NULL;
    }

    // LLVMTypeRef var_ptr_type = LLVMTypeOf(alloca);           // alloca es un puntero
    // fprintf(stderr,"UN tremendo problema aqui\n");
    // LLVMTypeRef var_type = LLVMGetElementType(var_ptr_type); // tipo apuntado
    //     fprintf(stderr,"UN tremendo problema aqui\n");
    // LLVMValueRef loaded = LLVMBuildLoad2(builder, var_type, alloca, "tmp");
    // fprintf(stderr,"UN tremendo problema aqui\n");

    LLVMTypeRef var_type = type_to_llvm(v->ctx, node->return_type);

    LLVMValueRef loaded = LLVMBuildLoad2(builder, var_type, alloca, "tmp");

    fprintf(stderr, "Hasta aqui todo joya\n");
    if (node->return_type && node->return_type != NULL)
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
            printf("Debug: Variable %s is a pointer or struct type, returning alloca\n", node->data.variable_name);
            return loaded;
        }

        // Intentar hacer el load de manera segura
        printf("Debug: Attempting to load variable %s\n", node->data.variable_name);

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
        printf("Debug: Unknown type for %s, returning alloca\n", node->data.variable_name);
        return loaded;
    }

    fprintf(stderr, "VOY A SALIR DE codegen variable \n");
}

LLVMValueRef codegen_assignments(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTOY EN EL CODEGEN DEL ASSIGNMENT\n");
    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;

    const char *var_name = node->data.op_node.left->data.variable_name;
    LLVMValueRef value = codegen_accept(v, node->data.op_node.right);

    LLVMTypeRef new_type;
    if (type_equals(node->data.op_node.right->return_type, &TYPE_STRING))
    {
        new_type = LLVMPointerType(LLVMInt8Type(), 0);
    }
    else if (type_equals(node->data.op_node.right->return_type, &TYPE_BOOLEAN))
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
        if (node->type == NODE_D_ASSIGNMENT)
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

    char *alloca_str = LLVMPrintValueToString(alloca);
    fprintf(stderr, "Guardando el valor en la variable %s (alloca: %s)\n",  var_name, alloca_str);
    //LLVMDisposeMessage(val_str);
    LLVMDisposeMessage(alloca_str);
    exit(1);



    LLVMPositionBuilderAtEnd(builder, current_block);
    LLVMBuildStore(builder, value, alloca);

    // Para asignación destructiva (:=), retornar el valor asignado
    if (node->type == NODE_D_ASSIGNMENT)
    {
        fprintf(stderr, "0000000000000000000000000000000\n");

        return value;
    }
    return NULL;
}
