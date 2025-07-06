#include "llvm_codegen.h"
#include "llvm_core.h"
#include "../scope/llvm_scope.h"
#include "llvm_operators.h"
#include "llvm_string.h"
#include "llvm_builtins.h"
#include "../type/type.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Funciones helper para el mapeo de tipos
static int get_type_id(const char *type_name)
{
    TypeIDMap *curr = type_id_map;
    while (curr)
    {
        if (strcmp(curr->type_name, type_name) == 0)
        {
            return curr->id;
        }
        curr = curr->next;
    }
    return -1;
}

static void register_type_id(const char *type_name, int id)
{
    TypeIDMap *new_entry = (TypeIDMap *)malloc(sizeof(TypeIDMap));
    new_entry->type_name = strdup(type_name);
    new_entry->id = id;
    new_entry->next = type_id_map;
    type_id_map = new_entry;
    printf("Debug: Registered type %s with ID %d\n", type_name, id);
}

static LLVMValueRef get_dynamic_type_id(LLVMValueRef instance)
{
    // Load type ID from instance (first field, index 0)
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(builder, LLVMGetElementType(LLVMTypeOf(instance)),
                                              instance, 0, "type_id_ptr");
    return LLVMBuildLoad2(builder, LLVMInt32Type(), id_ptr, "type_id");
}

LLVMTypeRef get_llvm_type(Type *type)
{
    if (type->sub_type)
    {
        return get_llvm_type(type->sub_type);
    }

    if (type_equals(type, &TYPE_NUMBER))
    {
        return LLVMDoubleType();
    }
    else if (type_equals(type, &TYPE_STRING))
    {
        return LLVMPointerType(LLVMInt8Type(), 0);
    }
    else if (type_equals(type, &TYPE_BOOLEAN))
    {
        return LLVMInt1Type();
    }
    else if (type_equals(type, &TYPE_VOID))
    {
        return LLVMVoidType();
    }
    else if (type_equals(type, &TYPE_OBJECT) || type_equals(type, &TYPE_NULL))
    {
        return LLVMPointerType(object_type, 0);
    }
    else if (type->dec != NULL)
    {
        // Es un tipo personalizado
        LLVMTypeRef struct_type = LLVMGetTypeByName(module, type->name);
        if (!struct_type)
        {
            fprintf(stderr, "Error: Tipo %s no encontrado\n", type->name);
            exit(1);
        }
        // Retornamos un puntero al tipo estructurado
        return LLVMPointerType(struct_type, 0);
    }

    fprintf(stderr, "Error: Tipo desconocido %s\n", type->name);
    exit(1);
}

LLVMValueRef get_default(LLVM_Visitor *v, Type *type)
{
    if (type->sub_type)
    {
        return get_llvm_type(type->sub_type);
    }

    if (type_equals(type, &TYPE_NUMBER))
    {
        return generate_number(v, create_number_node(0));
    }
    else if (type_equals(type, &TYPE_STRING))
    {
        return generate_string(v, create_string_node(""));
    }
    else if (type_equals(type, &TYPE_BOOLEAN))
    {
        return generate_boolean(v, create_boolean_node("false"));
    }
    else if (type_equals(type, &TYPE_VOID))
    {
        return generate_block(v, create_program_node(NULL, 0, NODE_BLOCK));
    }
    else if (type_equals(type, &TYPE_OBJECT) || type_equals(type, &TYPE_NULL))
    {
        // Usamos LLVMConstNull para retornar un puntero nulo del tipo object
        return LLVMConstNull(LLVMPointerType(object_type, 0));
    }
    else if (type->dec != NULL)
    {
        // Es un tipo personalizado
        LLVMTypeRef struct_type = LLVMGetTypeByName(module, type->name);
        if (!struct_type)
        {
            fprintf(stderr, "Error: Tipo %s no encontrado\n", type->name);
            exit(1);
        }
        // Retornamos un puntero nulo al tipo estructurado para tipos definidos por el usuario
        return LLVMConstNull(LLVMPointerType(struct_type, 0));
    }

    fprintf(stderr, "Error: Tipo desconocido %s\n", type->name);
    exit(1);
}

void generate_main_function(ASTNode *ast, const char *filename)
{
    LLVM_Visitor visitor = {
        .visit_program = generate_program,
        .visit_assignment = generate_assignment,
        .visit_binary_op = generate_binary_operation,
        .visit_number = generate_number,
        .visit_function_call = generate_builtin_function,
        .visit_string = generate_string,
        .visit_boolean = generate_boolean,
        .visit_unary_op = generate_unary_operation,
        .visit_variable = generate_variable,
        .visit_block = generate_block,
        .visit_function_dec = generate_function_body,
        .visit_let_in = generate_let_in,
        .visit_conditional = generate_conditional,
        .visit_q_conditional = generate_q_conditional,
        .visit_loop = generate_loop,
        .visit_type_dec = generate_type_declaration,
        .visit_type_inst = generate_type_instance,
        .visit_type_get_attr = generate_field_access,
        .visit_type_set_attr = generate_set_attr,
        .visit_type_method = generate_method_call,
        // .visit_type_test = generate_test_type,
        // .visit_type_cast = generate_cast_type,
        // .visit_base_call = generate_base_call
    };
    init_llvm();

    // Declare external functions
    declare_external_functions();

    // Process function and type declarations
    find_function_dec(&visitor, ast);
    make_body_function_dec(&visitor, ast);

    // Create scope
    push_scope();
    // Create main function
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(module, "main", main_type);

    // Create entry block
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");
    LLVMPositionBuilderAtEnd(builder, entry);

    // Generate code for AST
    if (ast)
    {
        accept_gen(&visitor, ast);
    }

    // Make sure we're in the right block for the return
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    if (!LLVMGetBasicBlockTerminator(current_block))
    {
        // Return 0 from main if the block isn't already terminated
        LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    }

    // Write to file
    char *error = NULL;
    if (LLVMPrintModuleToFile(module, filename, &error))
    {
        fprintf(stderr, "Error writing IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    // Free resources
    free_llvm_resources();
}

LLVMValueRef generate_program(LLVM_Visitor *v, ASTNode *node)
{
    push_scope();
    LLVMValueRef last = NULL;

    for (int i = 0; i < node->data.program_node.count; i++)
    {
        ASTNode *stmt = node->data.program_node.statements[i];
        if (stmt->type != NODE_FUNC_DEC)
        {
            last = accept_gen(v, stmt);
        }
    }

    pop_scope();
    return last ? last : LLVMConstInt(LLVMInt32Type(), 0, 0);
}

LLVMValueRef generate_number(LLVM_Visitor *v, ASTNode *node)
{
    return LLVMConstReal(LLVMDoubleType(), node->data.number_value);
}

LLVMValueRef generate_string(LLVM_Visitor *v, ASTNode *node)
{
    char *processed = process_string_escapes(node->data.string_value);
    LLVMValueRef str = LLVMBuildGlobalStringPtr(builder, processed, "str");
    free(processed);
    return str;
}

LLVMValueRef generate_boolean(LLVM_Visitor *v, ASTNode *node)
{
    int value = strcmp(node->data.string_value, "true") == 0 ? 1 : 0;
    return LLVMConstInt(LLVMInt1Type(), value, 0);
}

LLVMValueRef generate_block(LLVM_Visitor *v, ASTNode *node)
{
    push_scope();
    LLVMValueRef last_val = NULL;
    for (int i = 0; i < node->data.program_node.count; i++)
    {
        ASTNode *stmt = node->data.program_node.statements[i];
        if (stmt->type != NODE_FUNC_DEC)
        {
            last_val = accept_gen(v, stmt);
        }
    }
    if (last_val)
    {
        pop_scope();
    }
    return last_val;
}

LLVMValueRef generate_assignment(LLVM_Visitor *v, ASTNode *node)
{
    const char *var_name = node->data.op_node.left->data.variable_name;
    LLVMValueRef value = accept_gen(v, node->data.op_node.right);

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

    LLVMPositionBuilderAtEnd(builder, current_block);
    LLVMBuildStore(builder, value, alloca);

    if (node->type == NODE_D_ASSIGNMENT)
    {
        return value;
    }
    return NULL;
}

LLVMValueRef generate_variable(LLVM_Visitor *v, ASTNode *node)
{
    fprintf(stderr,"ESTOY EN EL CODEGEN DE VARIABLE\n");
    
    LLVMValueRef alloca = lookup_variable(node->data.variable_name);
    if (!alloca)
    {
        fprintf(stderr, "Error: Variable '%s' no declarada\n", node->data.variable_name);
        exit(1);
    }
    printf("Debug: Generating variable %s\n", node->data.variable_name);
    if (node->return_type && node->return_type->dec != NULL)
    {
        printf("Debug: Variable %s is a custom type\n", node->data.variable_name);
        return alloca;
    }
    else
    {
        printf("Debug: Getting type for variable %s\n", node->data.variable_name);
        LLVMTypeRef alloca_type = LLVMTypeOf(alloca);
        if (!alloca_type)
        {
            printf("Warning: Variable %s has null type, returning alloca\n", node->data.variable_name);
            return alloca;
        }

        // Obtener el tipo del elemento con verificación
        LLVMTypeRef var_type = LLVMGetElementType(alloca_type);
        if (!var_type)
        {
            printf("Warning: Cannot get element type for %s, returning alloca\n", node->data.variable_name);
            return alloca;
        }

        // Si el tipo es un puntero o estructura, devolver alloca
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
            return alloca;
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
        return alloca;
    }
}

void find_function_dec(LLVM_Visitor *visitor, ASTNode *node)
{
    if (!node)
        return;

    if (node->type == NODE_FUNC_DEC)
    {
        make_function_dec(visitor, node);
        find_function_dec(visitor, node->data.func_node.body);
        return;
    }

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (int i = 0; i < node->data.program_node.count; i++)
        {
            find_function_dec(visitor, node->data.program_node.statements[i]);
        }
        break;

    case NODE_LET_IN:
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT)
            {
                find_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
            }
        }
        find_function_dec(visitor, node->data.func_node.body);
        break;
    }
}

void make_body_function_dec(LLVM_Visitor *visitor, ASTNode *node)
{
    if (!node)
        return;

    if (node->type == NODE_FUNC_DEC)
    {
        accept_gen(visitor, node);
        make_body_function_dec(visitor, node->data.func_node.body);
        return;
    }

    switch (node->type)
    {
    case NODE_PROGRAM:
    case NODE_BLOCK:
        for (int i = 0; i < node->data.program_node.count; i++)
        {
            make_body_function_dec(visitor, node->data.program_node.statements[i]);
        }
        break;

    case NODE_LET_IN:
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT)
            {
                make_body_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
            }
        }

        make_body_function_dec(visitor, node->data.func_node.body);
        break;
    }
}

LLVMValueRef make_function_dec(LLVM_Visitor *v, ASTNode *node)
{
    const char *name = node->data.func_node.name;
    Type *return_type = node->data.func_node.body->return_type;
    ASTNode **params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;

    // Obtener tipos de parámetros
    LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++)
    {
        param_types[i] = get_llvm_type(params[i]->return_type);
    }

    LLVMTypeRef func_type = LLVMFunctionType(
        get_llvm_type(return_type),
        param_types,
        param_count,
        0);

    LLVMValueRef func = LLVMAddFunction(module, name, func_type);
    free(param_types);
    return func;
}

LLVMValueRef generate_function_body(LLVM_Visitor *v, ASTNode *node)
{
    const char *name = node->data.func_node.name;
    Type *return_type = node->data.func_node.body->return_type;
    ASTNode **params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;
    ASTNode *body = node->data.func_node.body;

    LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++)
    {
        param_types[i] = get_llvm_type(params[i]->return_type);
    }

    LLVMValueRef func = LLVMGetNamedFunction(module, name);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(func, "function_exit");

    LLVMPositionBuilderAtEnd(builder, entry);

    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);

    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT, new_depth,
                                     LLVMConstInt(LLVMInt32Type(), MAX_STACK_DEPTH, 0), "cmp_overflow");

    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(func, "stack_overflow");
    LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(func, "func_body");
    LLVMBuildCondBr(builder, cmp, error_block, continue_block);

    LLVMPositionBuilderAtEnd(builder, error_block);
    handle_stack_overflow(
        builder, module, current_stack_depth_var,
        node->line, node->data.func_node.name);

    LLVMPositionBuilderAtEnd(builder, continue_block);

    push_scope();

    for (int i = 0; i < param_count; i++)
    {
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, param_types[i], params[i]->data.variable_name);
        LLVMBuildStore(builder, param, alloca);
        declare_variable(params[i]->data.variable_name, alloca);
    }
    LLVMValueRef body_val = accept_gen(v, body);

    LLVMBuildBr(builder, exit_block);

    LLVMPositionBuilderAtEnd(builder, exit_block);

    LLVMValueRef final_depth = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth_final");
    LLVMValueRef dec_depth = LLVMBuildSub(builder, final_depth, LLVMConstInt(int32_type, 1, 0), "dec_depth");
    LLVMBuildStore(builder, dec_depth, current_stack_depth_var);

    // Return value handling
    if (type_equals(return_type, &TYPE_VOID))
    {
        LLVMBuildRetVoid(builder);
    }
    else if (body_val)
    {
        // If we have a return value, use it
        LLVMBuildRet(builder, body_val);
    }
    else
    {
        // Default return 0.0 as double
        LLVMBuildRet(builder, LLVMConstReal(LLVMDoubleType(), 0.0));
    }

    pop_scope();
    free(param_types);

    return func;
}

LLVMValueRef generate_let_in(LLVM_Visitor *v, ASTNode *node)
{
    push_scope();
    ASTNode **declarations = node->data.func_node.args;
    int dec_count = node->data.func_node.arg_count;

    for (int i = 0; i < dec_count; i++)
    {
        ASTNode *decl = declarations[i];
        const char *var_name = decl->data.op_node.left->data.variable_name;
        LLVMValueRef value = accept_gen(v, decl->data.op_node.right);
        if (!value)
        {
            return NULL;
        }
        LLVMTypeRef var_type = get_llvm_type(decl->data.op_node.right->return_type);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, var_type, var_name);
        LLVMBuildStore(builder, value, alloca);
        declare_variable(var_name, alloca);
    }

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(builder);
    LLVMValueRef result = accept_gen(v, node->data.func_node.body);
    LLVMPositionBuilderAtEnd(builder, LLVMGetInsertBlock(builder));

    pop_scope();
    return result;
}

LLVMValueRef generate_conditional(LLVM_Visitor *v, ASTNode *node)
{
    ASTNode *condition = node->data.cond_node.cond;
    ASTNode *true_body = node->data.cond_node.body_true;
    ASTNode *false_body = node->data.cond_node.body_false;

    // Get current function
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Create basic blocks
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "then");
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "else");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "merge");

    // Generate condition code
    LLVMValueRef cond_val = accept_gen(v, condition);

    // Create conditional branch
    LLVMBuildCondBr(builder, cond_val, then_block, else_block ? else_block : merge_block);

    // Generate 'then' block
    LLVMPositionBuilderAtEnd(builder, then_block);
    LLVMValueRef then_val = accept_gen(v, true_body);

    // Cast then_val to the return type if necessary
    if (!type_equals(true_body->return_type, node->return_type))
    {
        then_val = cast_value_to_type(then_val, true_body->return_type, node->return_type);
    }

    LLVMBuildBr(builder, merge_block);

    // Get updated then_block for PHI
    then_block = LLVMGetInsertBlock(builder);

    // Generate 'else' block if it exists
    LLVMValueRef else_val = NULL;
    LLVMBasicBlockRef last_else_block = NULL;
    LLVMPositionBuilderAtEnd(builder, else_block);
    else_val = false_body ? accept_gen(v, false_body) : get_default(v, true_body->return_type);
    Type *false_type = false_body ? false_body->return_type : NULL;

    if (!false_type)
    {
        if (is_builtin_type(true_body->return_type) &&
            !type_equals(true_body->return_type, &TYPE_OBJECT))
        {
            false_type = true_body->return_type;
        }
        else
        {
            false_type = &TYPE_NULL;
        }
    }

    // Cast else_val to the return type if necessary
    if (!type_equals(false_type, node->return_type))
    {
        else_val = cast_value_to_type(else_val, false_type, node->return_type);
    }

    LLVMBuildBr(builder, merge_block);
    last_else_block = LLVMGetInsertBlock(builder);

    // Generate merge block with PHI node if needed
    LLVMPositionBuilderAtEnd(builder, merge_block);

    if (type_equals(node->return_type, &TYPE_VOID))
    {
        return NULL;
    }

    // Create PHI node with the correct type
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "if_result");

    // Add incoming values to PHI with proper null values for missing branches
    LLVMValueRef incoming_values[2] = {
        then_val ? then_val : LLVMConstNull(phi_type),
        else_val ? else_val : LLVMConstNull(phi_type)};
    LLVMBasicBlockRef incoming_blocks[2] = {then_block, last_else_block};
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

    return phi;
}

LLVMValueRef generate_q_conditional(LLVM_Visitor *v, ASTNode *node)
{
    // Extraemos los nodos correspondientes
    ASTNode *condition = node->data.cond_node.cond;
    ASTNode *true_body = node->data.cond_node.body_true;
    ASTNode *false_body = node->data.cond_node.body_false;

    // Obtenemos la función actual a partir del bloque insertado
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    // Creamos tres bloques: then, else y merge
    LLVMBasicBlockRef then_block = LLVMAppendBasicBlock(current_function, "q_then");
    LLVMBasicBlockRef else_block = LLVMAppendBasicBlock(current_function, "q_else");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "q_merge");

    // Generamos el código para la condición
    LLVMValueRef cond_expr = accept_gen(v, condition);

    // Para verificar "if (x is null)" de forma coherente:
    // - Si "cond_expr" es un puntero, usamos LLVMBuildIsNull.
    // - Si no es un puntero (por ejemplo, un número), asumimos que nunca es null.
    LLVMValueRef cond_bool = NULL;
    LLVMTypeRef cond_type = LLVMTypeOf(cond_expr);
    if (LLVMGetTypeKind(cond_type) == LLVMPointerTypeKind)
    {
        cond_bool = LLVMBuildIsNull(builder, cond_expr, "is_null");
    }
    else
    {
        // Para tipos que no son punteros, definimos la condición como siempre falsa.
        cond_bool = LLVMConstInt(LLVMInt1Type(), 0, 0);
    }

    // Construimos la bifurcación condicional basada en cond_bool.
    LLVMBuildCondBr(builder, cond_bool, then_block, else_block);

    // --- Bloque 'then' (cuerpo true) ---
    LLVMPositionBuilderAtEnd(builder, then_block);
    LLVMValueRef then_val = accept_gen(v, true_body);
    // Realizamos conversiones/casts si es necesario para que el tipo encaje.
    if (then_val && !type_equals(true_body->return_type, node->return_type))
    {
        then_val = cast_value_to_type(then_val, true_body->return_type, node->return_type);
    }
    LLVMBuildBr(builder, merge_block);
    then_block = LLVMGetInsertBlock(builder);

    // --- Bloque 'else' (cuerpo false) ---
    LLVMPositionBuilderAtEnd(builder, else_block);
    LLVMValueRef else_val = accept_gen(v, false_body);
    // Realizamos el cast si es necesario.
    if (else_val && !type_equals(false_body->return_type, node->return_type))
    {
        else_val = cast_value_to_type(else_val, false_body->return_type, node->return_type);
    }
    LLVMBuildBr(builder, merge_block);
    LLVMBasicBlockRef last_else_block = LLVMGetInsertBlock(builder);

    // --- Bloque de merge ---
    LLVMPositionBuilderAtEnd(builder, merge_block);

    // Si el tipo de retorno es void, no es necesario crear un PHI
    if (type_equals(node->return_type, &TYPE_VOID))
    {
        return NULL;
    }

    // Creamos el nodo PHI para unificar los valores de ambos bloques,
    // asegurándonos de que se retorne el tipo esperado.
    LLVMTypeRef phi_type = get_llvm_type(node->return_type);
    LLVMValueRef phi = LLVMBuildPhi(builder, phi_type, "q_if_result");

    // Agregamos las entradas del PHI: una de entonces y otra de else.
    LLVMValueRef incoming_values[2] = {
        then_val ? then_val : LLVMConstNull(phi_type),
        else_val ? else_val : LLVMConstNull(phi_type)};
    LLVMBasicBlockRef incoming_blocks[2] = {then_block, last_else_block};
    LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

    return phi;
}

// Helper function to cast values between types
LLVMValueRef cast_value_to_type(LLVMValueRef value, Type *from_type, Type *to_type)
{
    // If types are equal or value is NULL, no casting needed
    if (from_type->sub_type)
        from_type = from_type->sub_type;
    if (to_type->sub_type)
        to_type = to_type->sub_type;

    if (!value || type_equals(from_type, to_type))
    {
        return value;
    }

    // Handle primitive type conversions first
    if (is_builtin_type(to_type))
    {
        if (type_equals(to_type, &TYPE_NUMBER))
        {
            if (type_equals(from_type, &TYPE_BOOLEAN))
            {
                return LLVMBuildSIToFP(builder, value, LLVMDoubleType(), "bool_to_num");
            }
            return LLVMConstReal(LLVMDoubleType(), 0.0);
        }

        if (type_equals(to_type, &TYPE_STRING))
        {
            if (type_equals(from_type, &TYPE_NUMBER))
            {
                return LLVMBuildGlobalStringPtr(builder, "0", "num_to_str");
            }
            if (type_equals(from_type, &TYPE_BOOLEAN))
            {
                return LLVMBuildICmp(builder, LLVMIntNE, value, LLVMConstInt(LLVMInt1Type(), 0, 0), "bool_val") ? LLVMBuildGlobalStringPtr(builder, "true", "bool_to_str") : LLVMBuildGlobalStringPtr(builder, "false", "bool_to_str");
            }
            return LLVMBuildGlobalStringPtr(builder, "", "to_str");
        }

        if (type_equals(to_type, &TYPE_BOOLEAN))
        {
            if (type_equals(from_type, &TYPE_NUMBER))
            {
                return LLVMBuildFCmp(builder, LLVMRealONE, value,
                                     LLVMConstReal(LLVMDoubleType(), 0.0), "num_to_bool");
            }
            return LLVMConstInt(LLVMInt1Type(), 0, 0);
        }
    }

    // Manejo de conversión de tipos de usuario a un supertipo built-in, por ejemplo, a OBJECT.
    // Esto es útil si TYPE_OBJECT es considerado built-in pero queremos admitir que
    // un tipo definido por el usuario (como A) se pueda convertir a Object.
    if (!is_builtin_type(from_type) && (type_equals(to_type, &TYPE_OBJECT)))
    {
        LLVMTypeRef from_struct = LLVMGetTypeByName(module, from_type->name);
        LLVMTypeRef object_struct = LLVMGetTypeByName(module, to_type->name);
        if (from_struct && object_struct)
        {
            LLVMTypeRef object_ptr = LLVMPointerType(object_struct, 0);
            return LLVMBuildBitCast(builder, value, object_ptr, "to_object");
        }
    }

    // Handle user-defined types
    if (!is_builtin_type(from_type) && !is_builtin_type(to_type))
    {
        // Find closest common ancestor
        Type *common_ancestor = get_lca(from_type, to_type);
        if (common_ancestor)
        {
            common_ancestor = common_ancestor->sub_type ? common_ancestor->sub_type : common_ancestor;
            // Get the struct types
            LLVMTypeRef from_struct = LLVMGetTypeByName(module, from_type->name);
            LLVMTypeRef to_struct = LLVMGetTypeByName(module, to_type->name);
            LLVMTypeRef ancestor_struct = LLVMGetTypeByName(module, common_ancestor->name);

            if (from_struct && to_struct && ancestor_struct)
            {
                // Create pointers to the struct types
                LLVMTypeRef from_ptr = LLVMPointerType(from_struct, 0);
                LLVMTypeRef to_ptr = LLVMPointerType(to_struct, 0);
                LLVMTypeRef ancestor_ptr = LLVMPointerType(ancestor_struct, 0);

                // Cast through the common ancestor
                LLVMValueRef as_ancestor = LLVMBuildBitCast(builder, value, ancestor_ptr, "as_ancestor");

                // If target type is the ancestor, we're done
                if (type_equals(to_type, common_ancestor))
                {
                    return as_ancestor;
                }

                // Otherwise cast to target type
                return LLVMBuildBitCast(builder, as_ancestor, to_ptr, "as_target");
            }
        }
    }

    // If no valid cast found, return null value of target type
    return LLVMConstNull(get_llvm_type(to_type));
}

LLVMValueRef generate_loop(LLVM_Visitor *v, ASTNode *node)
{
    LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));

    LLVMTypeRef body_type = get_llvm_type(node->data.op_node.right->return_type);

    LLVMValueRef result_addr = NULL;
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        result_addr = LLVMBuildAlloca(builder, body_type, "while.result.addr");
        LLVMBuildStore(builder, LLVMConstNull(body_type), result_addr);
    }

    LLVMBasicBlockRef cond_block = LLVMAppendBasicBlock(current_function, "while.cond");
    LLVMBasicBlockRef loop_block = LLVMAppendBasicBlock(current_function, "while.body");
    LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "while.end");

    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, cond_block);
    LLVMValueRef cond_val = accept_gen(v, node->data.op_node.left);
    LLVMBuildCondBr(builder, cond_val, loop_block, merge_block);

    LLVMPositionBuilderAtEnd(builder, loop_block);
    LLVMValueRef body_val = accept_gen(v, node->data.op_node.right);

    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        LLVMBuildStore(builder, body_val, result_addr);
    }
    LLVMBuildBr(builder, cond_block);

    LLVMPositionBuilderAtEnd(builder, merge_block);
    if (LLVMGetTypeKind(body_type) != LLVMVoidTypeKind)
    {
        LLVMValueRef final_val = LLVMBuildLoad2(builder, body_type, result_addr, "while.result");
        return final_val;
    }
    else
    {
        return NULL;
    }
}

// Genera la estrcutra base de un tipo y su layout en memoria
LLVMTypeRef generate_struct_type(const char *type_name, ASTNode *type_node)
{
    // Se crea un tipo LLVMStrcut con nombre type_name
    LLVMTypeRef struct_type = LLVMStructCreateNamed(context, type_name);

    // Count fields to allocate
    int field_count = 0; // cantidad de campos de declaracion tiene
    for (int i = 0; i < type_node->data.type_node.def_count; i++)
    {
        ASTNode *def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT)
        {
            field_count++;
        }
    }

    // Create array for field types - add 3 for type id, vtable pointer and parent pointer
    int extra_fields = type_node->data.type_node.parent_name[0] != '\0' ? 3 : 2;
    LLVMTypeRef *field_types = malloc((field_count + extra_fields) * sizeof(LLVMTypeRef));
    int field_idx = 0;

    // Add type ID as first field (32-bit integer)
    field_types[field_idx++] = LLVMInt32Type();

    // Add vtable pointer as second field - just create an opaque type for now
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", type_name);
    // Don't create the vtable type here, just get a reference to it
    LLVMTypeRef vtable_type = LLVMStructCreateNamed(context, vtable_type_name);
    field_types[field_idx++] = LLVMPointerType(vtable_type, 0);

    // Add parent pointer if inheriting
    if (type_node->data.type_node.parent_name[0] != '\0')
    {
        LLVMTypeRef parent_struct_type = LLVMGetTypeByName(module, type_node->data.type_node.parent_name);
        if (parent_struct_type)
        {
            field_types[field_idx++] = LLVMPointerType(parent_struct_type, 0);
        }
    }

    // Add this type's fields
    for (int i = 0; i < type_node->data.type_node.def_count; i++)
    {
        ASTNode *def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT)
        {
            Type *field_type = def->data.op_node.right->return_type;
            field_types[field_idx++] = get_llvm_type(field_type);
        }
    }

    LLVMStructSetBody(struct_type, field_types, field_idx, 0);
    free(field_types);

    return struct_type;
}

// Genera las funciones de una clase y construye su vtable
void generate_type_methods(LLVM_Visitor *visitor, const char *type_name, LLVMTypeRef struct_type, ASTNode *type_node)
{
    printf("\nDebug: Generating methods for type %s\n", type_name);

    // First generate vtable type
    LLVMBuilderRef saved_builder = builder;
    builder = LLVMCreateBuilder();

    // Count methods first
    int method_count = 0;
    for (int i = 0; i < type_node->data.type_node.def_count; i++)
    {
        if (type_node->data.type_node.definitions[i]->type == NODE_FUNC_DEC)
        {
            method_count++;
        }
    }
    printf("Debug: Found %d methods to generate\n", method_count);

    if (method_count == 0)
    {
        printf("Debug: No methods to generate for type %s\n", type_name);
        LLVMDisposeBuilder(builder);
        builder = saved_builder;
        printf("Debug: Exiting method generation for type %s\n", type_name);
        return;
    }

    // Arrays for storing method information
    LLVMTypeRef *vtable_fn_types = malloc(method_count * sizeof(LLVMTypeRef));
    LLVMValueRef *method_ptrs = malloc(method_count * sizeof(LLVMValueRef));
    char **method_names = malloc(method_count * sizeof(char *));
    int method_idx = 0;

    // First pass: Generate all method declarations and bodies
    for (int i = 0; i < type_node->data.type_node.def_count; i++)
    {
        ASTNode *def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_FUNC_DEC)
        {
            // Use the method name as is, it's already properly formatted
            method_names[method_idx] = strdup(def->data.func_node.name);
            printf("Debug: Using method name: %s\n", method_names[method_idx]);

            printf("Debug: Generating method %s (#%d)\n", def->data.func_node.name, method_idx);

            // Create function type with 'this' pointer as first argument
            int param_count = def->data.func_node.arg_count + 1;
            LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
            param_types[0] = LLVMPointerType(struct_type, 0); // 'this' pointer

            for (int j = 0; j < def->data.func_node.arg_count; j++)
            {
                param_types[j + 1] = get_llvm_type(def->data.func_node.args[j]->return_type);
            }

            // Get return type
            LLVMTypeRef return_type;
            if (def->data.func_node.body->return_type)
            {
                return_type = get_llvm_type(def->data.func_node.body->return_type);
                printf("Debug: Method %s return type is %s\n",
                       def->data.func_node.name, def->data.func_node.body->return_type->name);
            }
            else
            {
                printf("Warning: No return type specified for method %s, using void\n",
                       def->data.func_node.name);
                return_type = LLVMVoidType();
            }

            // Create function type and store it for vtable
            LLVMTypeRef func_type = LLVMFunctionType(return_type, param_types, param_count, 0);
            vtable_fn_types[method_idx] = LLVMPointerType(func_type, 0);

            printf("Debug: Created function type for method %s: %s\n",
                   def->data.func_node.name, LLVMPrintTypeToString(func_type));

            // Add function declaration with original name
            LLVMValueRef func = LLVMAddFunction(module, def->data.func_node.name, func_type);
            method_ptrs[method_idx] = func;

            // Generate method body
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
            LLVMPositionBuilderAtEnd(builder, entry);

            // Store old scope and create new one
            push_scope();

            // Add 'this' pointer to scope
            LLVMValueRef this_ptr = LLVMGetParam(func, 0);
            declare_variable("self", this_ptr);

            // Add parameters to scope
            for (int j = 0; j < def->data.func_node.arg_count; j++)
            {
                LLVMValueRef param = LLVMGetParam(func, j + 1);
                declare_variable(def->data.func_node.args[j]->data.variable_name, param);
            }

            // Generate body
            LLVMValueRef result = accept_gen(visitor, def->data.func_node.body);

            if (!result)
            {
                printf("Error: Method %s body returned NULL\n", def->data.func_node.name);
                LLVMBuildRetVoid(builder);
            }
            else
            {
                printf("Debug: Method %s returned value of type: %s\n",
                       def->data.func_node.name,
                       LLVMPrintTypeToString(LLVMTypeOf(result)));

                if (LLVMGetTypeKind(return_type) == LLVMVoidTypeKind)
                {
                    LLVMBuildRetVoid(builder);
                }
                else
                {
                    LLVMBuildRet(builder, result);
                }
            }

            pop_scope();
            free(param_types);
            method_idx++;

            printf("Debug: Generated method body for %s\n", def->data.func_node.name);
        }
    }

    // Get the existing vtable type and set its body
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", type_name);
    printf("Debug: Setting vtable type %s body with %d methods\n", vtable_type_name, method_count);

    LLVMTypeRef vtable_type = LLVMGetTypeByName(module, vtable_type_name);
    if (!vtable_type)
    {
        printf("Error: Could not find vtable type %s\n", vtable_type_name);
        return;
    }
    LLVMStructSetBody(vtable_type, vtable_fn_types, method_count, 0);

    // Create vtable global
    char vtable_name[256];
    snprintf(vtable_name, sizeof(vtable_name), "%s_vtable_instance", type_name);
    printf("Debug: Creating vtable global %s\n", vtable_name);

    LLVMValueRef vtable = LLVMAddGlobal(module, vtable_type, vtable_name);

    // Initialize vtable with method pointers
    LLVMValueRef vtable_init = LLVMConstStruct(method_ptrs, method_count, 0);
    LLVMSetInitializer(vtable, vtable_init);

    printf("Debug: Vtable initialization complete. Contents:\n");
    for (int i = 0; i < method_count; i++)
    {
        printf("  Slot %d: %s -> %s\n", i, method_names[i],
               LLVMPrintValueToString(method_ptrs[i]));
    }

    // Cleanup
    for (int i = 0; i < method_count; i++)
    {
        free(method_names[i]);
    }
    free(method_names);
    free(vtable_fn_types);
    free(method_ptrs);

    LLVMDisposeBuilder(builder);
    builder = saved_builder;
}

// coordina todo el proceso de creacion de una clase
LLVMValueRef generate_type_declaration(LLVM_Visitor *v, ASTNode *node)
{

    const char *type_name = node->data.type_node.name;
    // Assign type ID
    int type_id = next_type_id++; // varaible global

    // al nodo le asignar un id
    node->data.type_node.id = type_id;

    // registra el ID en una estructura
    register_type_id(type_name, type_id);

    printf("Debug: Assigned ID %d to type %s\n", type_id, type_name);

    /// ABRI NUEVO SCOPE Y AGREGAR PARAMETROS TIPOS

    // Store type parameters in scope for use in field initializations
    push_scope();

    // Add type parameters to scope
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        ASTNode *param = node->data.type_node.args[i];
        // Create an alloca for each parameter
        LLVMTypeRef param_type = get_llvm_type(param->return_type);
        LLVMValueRef param_alloca = LLVMBuildAlloca(builder, param_type, param->data.variable_name);
        declare_variable(param->data.variable_name, param_alloca);
    }

    // Create the struct type for this class (or use existing forward declaration)
    LLVMTypeRef struct_type = LLVMGetTypeByName(module, type_name);
    if (!struct_type)
    {
        // Create a new struct type if it doesn't exist
        struct_type = LLVMStructCreateNamed(context, type_name);
        printf("Debug: Created new struct type %s\n", type_name);
    }
    else
    {
        printf("Debug: Using existing forward declaration for %s\n", type_name);
    }

    // Count fields to allocate
    int field_count = 0;
    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode *def = node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT)
        {
            field_count++;
        }
    }

    // Create array for field types - add 3 for type id, vtable pointer and parent pointer
    int extra_fields = node->data.type_node.parent_name[0] != '\0' ? 3 : 2;
    LLVMTypeRef *field_types = malloc((field_count + extra_fields) * sizeof(LLVMTypeRef));
    int field_idx = 0;

    // Add type ID as first field (32-bit integer)
    field_types[field_idx++] = LLVMInt32Type();

    // Add vtable pointer as second field
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", type_name);
    // Get or create vtable type (forward declaration if needed)
    LLVMTypeRef vtable_type = LLVMGetTypeByName(module, vtable_type_name);
    if (!vtable_type)
    {
        vtable_type = LLVMStructCreateNamed(context, vtable_type_name);
    }
    field_types[field_idx++] = LLVMPointerType(vtable_type, 0);

    // Add parent pointer if inheriting
    if (node->data.type_node.parent_name[0] != '\0')
    {
        // Get or create parent type (forward declaration if needed)
        LLVMTypeRef parent_struct_type = LLVMGetTypeByName(module, node->data.type_node.parent_name);
        if (!parent_struct_type)
        {
            parent_struct_type = LLVMStructCreateNamed(context, node->data.type_node.parent_name);
        }
        field_types[field_idx++] = LLVMPointerType(parent_struct_type, 0);
    }

    // Add this type's fields
    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        ASTNode *def = node->data.type_node.definitions[i];
        if (def->type == NODE_ASSIGNMENT)
        {
            Type *field_type = def->data.op_node.right->return_type;
            field_types[field_idx++] = get_llvm_type(field_type);
        }
    }

    // Set the body of the struct (even if it was a forward declaration)
    LLVMStructSetBody(struct_type, field_types, field_idx, 0);
    free(field_types);

    // Generate methods for this type
    generate_type_methods(v, type_name, struct_type, node);
    printf("Debug: Generated struct type %s\n", type_name);

    pop_scope();
    return NULL;
}

static Type *find_type_by_method(const char *method_name, Type *type)
{
    char *name = strdup(method_name);
    while (type && !is_builtin_type(type))
    {
        printf("full_name: %s\n", name);

        if (LLVMGetNamedFunction(module, name))
        {
            return type;
        }

        name = delete_underscore_from_str(name, type->name);
        type = type->parent;
        printf("type: %s\n", type ? type->name : "NULL");
        name = concat_str_with_underscore(type->name, name);
    }
    return NULL;
}

// Busca el indice de un metodo en la vtable, recorriendo recursivamente los padres
static int find_vtable_index(Type *type, const char *method_name)
{
    if (!type || !type->dec)
    {
        return -1;
    }

    int method_idx = 0;
    ASTNode *type_node = type->dec;

    // Search through parent types first to maintain vtable order
    if (type->parent && !is_builtin_type(type->parent))
    {
        method_idx = find_vtable_index(type->parent, method_name);
        if (method_idx >= 0)
        {
            return method_idx;
        }
    }

    // Search through current type's methods
    for (int i = 0; i < type_node->data.type_node.def_count; i++)
    {
        ASTNode *def = type_node->data.type_node.definitions[i];
        if (def->type == NODE_FUNC_DEC)
        {
            if (strcmp(def->data.func_node.name, method_name) == 0)
            {
                return method_idx;
            }
            method_idx++;
        }
    }

    return -1;
}

// busca el metodo
static char *find_method_in_hierarchy(const char *method_name, Type *type)
{
    char *name = strdup(method_name);
    while (type && !is_builtin_type(type))
    {
        printf("full_name: %s\n", name);

        if (LLVMGetNamedFunction(module, name))
        {
            return name;
        }

        name = delete_underscore_from_str(name, type->name);
        type = type->parent;
        printf("type: %s\n", type ? type->name : "NULL");
        name = concat_str_with_underscore(type->name, name);
    }
    return NULL;
}

// busca la posicion en la estrcutra de datos , considerando la herencia y campos prefijados
static int find_field_index_in_type(Type *type, const char *field_name, int base_index)
{
    // Look in current type's fields only
    ASTNode *type_node = type->dec;
    if (type_node)
    {
        int current_index = base_index;
        for (int i = 0; i < type_node->data.type_node.def_count; i++)
        {
            ASTNode *def = type_node->data.type_node.definitions[i];
            if (def->type == NODE_ASSIGNMENT)
            {
                if (strcmp(def->data.op_node.left->data.variable_name, field_name) == 0)
                {
                    return current_index;
                }
                current_index++;
            }
        }
    }
    return -1;
}

static // Returns -1 if not found in type hierarchy, otherwise returns the field index
    int
    find_field_index(Type *type, const char *field_name)
{
    int current_index = 2; // Start after type ID and vtable pointer

    // If has parent, skip parent pointer
    if (type->parent && !is_builtin_type(type->parent))
    {
        current_index++;
    }

    // First try to find in current type
    int field_idx = find_field_index_in_type(type, field_name, current_index);
    if (field_idx >= 0)
    {
        printf("Debug: Found field %s in type %s at index %d\n",
               field_name, type->name, field_idx);
        return field_idx;
    }

    // // If not found and we have a non-builtin parent type, try parent
    // if (type->parent && !is_builtin_type(type->parent)) {
    //     printf("Debug: Field %s not found in %s, searching in parent %s\n",
    //            field_name, type->name, type->parent->name);
    //     field_name=delete_underscore_from_str(field_name,type->name);
    //     field_name=concat_str_with_underscore(type->parent->name,field_name);
    //     return find_field_index(type->parent, field_name);
    // }

    return -1; // Not found in type hierarchy
}

// Genera el acceso a un campo (a.b)
LLVMValueRef generate_field_access(LLVM_Visitor *v, ASTNode *node)
{

    fprintf(stderr, "ESTAMOS EN EL GETTER DE UN ATRIBUTO\n");

    LLVMValueRef instance_ptr = accept_gen(v, node->data.op_node.left);

    fprintf(stderr,"EXITOS\n");

    LLVMTypeRef instance_type_ref = LLVMTypeOf(instance_ptr);
    const char *name = node->data.op_node.right->data.variable_name;
    printf("Debug: Accessing field '%s' in instance of type %s\n", name, LLVMPrintTypeToString(instance_type_ref));
    Type *instance_type = node->data.op_node.left->return_type;

    if (LLVMGetTypeKind(instance_type_ref) != LLVMPointerTypeKind)
    {
        fprintf(stderr, "Error: instance_type_ref no es un puntero\n");
        return NULL;
    }

    LLVMTypeRef struct_type = LLVMGetElementType(instance_type_ref);

    if (LLVMGetTypeKind(struct_type) != LLVMStructTypeKind)
    {
        fprintf(stderr, "Error: struct_type no es una estructura\n");
        return NULL;
    }

    Type *field_type = instance_type;

    // Try to find field in current type
    int field_index = find_field_index(instance_type, name);
    fprintf(stderr, "1-HASTA AQUI TODO BIEN\n");

    // If not found in current type, try parent types
    if (field_index == -1 && instance_type->parent && !is_builtin_type(instance_type->parent))
    {
        // Get parent instance pointer (at index 2)
        LLVMValueRef parent_ptr = LLVMBuildStructGEP2(
            builder,
            struct_type,
            instance_ptr,
            2,
            "parent_ptr");

        // Load parent pointer
        instance_ptr = LLVMBuildLoad2(
            builder,
            LLVMGetElementType(LLVMTypeOf(parent_ptr)),
            parent_ptr,
            "parent_instance");

        // Update pointers to use parent instance
        instance_type_ref = LLVMTypeOf(instance_ptr);
        struct_type = LLVMGetElementType(instance_type_ref);
        field_type = instance_type->parent;
        printf("Debug: Field %s not found in type %s, searching in parent type %s\n",
               name, instance_type->name, field_type->name);

        // Primero eliminamos el prefijo del tipo actual para obtener el nombre base del campo
        char *base_name;
        if (strncmp(name, "_", 1) == 0)
        {
            base_name = delete_underscore_from_str(name, instance_type->name);
        }
        else
        {
            base_name = strdup(name);
        }

        // Luego construimos el nombre calificado para el tipo padre
        char *parent_field_name = concat_str_with_underscore(field_type->name, base_name);

        printf("Debug: Searching for field %s in parent type %s\n",
               parent_field_name, field_type->name);

        field_index = find_field_index(field_type, parent_field_name);

        // Limpieza de memoria
        free(base_name);
        if (parent_field_name != name)
        { // Solo liberamos si es un nuevo string
            free(parent_field_name);
        }

        printf("Debug: Accessing parent field %s at index %d in type %s\n",
               parent_field_name, field_index, field_type->name);
    }

    fprintf(stderr, "2-HAST AQUI TODO BIEN\n ");

    if (field_index == -1)
    {
        printf("Error: Field %s not found in type or parent types\n", name);
        return NULL;
    }

    // Get pointer to the field
    LLVMValueRef field_ptr = LLVMBuildStructGEP2(
        builder,
        struct_type,
        instance_ptr,
        field_index,
        "field_ptr");

    fprintf(stderr, "3-HAST AQUI TODO BIEN\n ");

    if (!builder)
    {
        fprintf(stderr, "Error: builder is NULL\n");
    }
    if (!field_ptr)
    {
        fprintf(stderr, "Error: field_ptr is NULL\n");
    }
    LLVMTypeRef field_ptr_type = LLVMTypeOf(field_ptr);
    if (!field_ptr_type)
    {
        fprintf(stderr, "Error: LLVMTypeOf(field_ptr) is NULL\n");
    }
    else
    {
        fprintf(stderr, "field_ptr_type: %s\n", LLVMPrintTypeToString(field_ptr_type));
    }

    if (LLVMGetTypeKind(field_ptr_type) != LLVMPointerTypeKind)
    {
        fprintf(stderr, "Error: field_ptr is not a pointer\n");
        return NULL;
    }

    LLVMTypeRef element_type = LLVMGetElementType(field_ptr_type);
    if (!element_type)
    {
        fprintf(stderr, "Error: element_type is NULL\n");
        return NULL;
    }

    fprintf(stderr, "element_type: %s\n", LLVMPrintTypeToString(element_type));
    fprintf(stderr, "struct_type: %s\n", LLVMPrintTypeToString(struct_type));
    fprintf(stderr, "field_index: %d\n", field_index);

    // Load the field value
    LLVMValueRef field_value = LLVMBuildLoad2(
        builder,
        element_type,
        field_ptr,
        name);

    fprintf(stderr, "4-HAST AQUI TODO BIEN\n ");

    printf("Debug: Field value type is: %s\n", LLVMPrintTypeToString(LLVMTypeOf(field_value)));
    fprintf(stderr, "TODO ESTUVO CORRECTO\n");

    return field_value;
}

// extrae el nombre del tipo de un metodo con nombre con prefijo como _MyType_method
char *extract_type_from_method_name(const char *method_full_name)
{
    // 1. Verificar formato básico
    if (!method_full_name || method_full_name[0] != '_')
        return NULL;

    // 2. Encontrar el segundo '_' que separa tipo y nombre
    const char *first_underscore = method_full_name;
    const char *second_underscore = strchr(first_underscore + 1, '_');

    if (!second_underscore || second_underscore == first_underscore + 1)
        return NULL;

    // 3. Calcular longitud del tipo
    size_t type_length = second_underscore - first_underscore - 1; // -1 para excluir el primer '_'

    // 4. Reservar memoria y copiar
    char *type = (char *)malloc(type_length + 1);
    if (!type)
        return NULL;

    strncpy(type, first_underscore + 1, type_length);
    type[type_length] = '\0'; // Terminar cadena

    return type;
}

// OTROS
LLVMValueRef generate_type_instance(LLVM_Visitor *v, ASTNode *node)
{
    push_scope();
    printf("Debug: Generating instance for type %s\n", node->data.type_node.name);
    const char *type_name = node->data.type_node.name;
    printf("Debug: Type name: %s\n", type_name);
    LLVMTypeRef struct_type = LLVMGetTypeByName(module, type_name);
    printf("Debug: Struct type: %s\n", LLVMPrintTypeToString(struct_type));
    LLVMValueRef instance = LLVMBuildMalloc(builder, struct_type, "instance");
    printf("Debug: Allocated instance: %s\n", LLVMPrintValueToString(instance));
    Type *type = node->return_type;
    ASTNode *type_def = type->dec;
    printf("Debug: Type definition found for %s\n", type_name);

    // Almacenamos los parámetros del constructor en el scope antes de cualquier inicialización
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        LLVMValueRef arg_value = accept_gen(v, node->data.type_node.args[i]);
        const char *param_name = type_def->data.type_node.args[i]->data.variable_name;

        // Crear alloca para el parámetro y almacenarlo en el scope
        Type *param_type = node->data.type_node.args[i]->return_type;
        if (type_equals(param_type, &TYPE_STRING))
        {
            // For strings, store directly the i8* value
            declare_variable(param_name, arg_value);
        }
        else
        {
            // For other types, we need to handle them differently based on whether they're class types or not
            Type *param_type = node->data.type_node.args[i]->return_type;
            if (!is_builtin_type(param_type))
            {
                // For class types, store the pointer directly
                declare_variable(param_name, arg_value);
            }
            else
            {
                // For primitive types, create alloca and store
                LLVMTypeRef alloca_type = LLVMTypeOf(arg_value);
                LLVMValueRef param_alloca = LLVMBuildAlloca(builder, alloca_type, "param_alloca");
                LLVMBuildStore(builder, arg_value, param_alloca);
                declare_variable(param_name, param_alloca);
            }
        }
        printf("Debug: Declared constructor parameter '%s' in scope\n", param_name);
    }
    // Initialize type ID field (index 0)
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, 0, "type_id_ptr");
    printf("Debug: Type ID pointer: %s\n", LLVMPrintValueToString(id_ptr));
    LLVMBuildStore(builder, LLVMConstInt(LLVMInt32Type(), type_def->data.type_node.id, 0), id_ptr);
    printf("Debug: Set type ID to %d\n", type_def->data.type_node.id);
    // Initialize vtable pointer (index 1)
    char vtable_name[256];
    snprintf(vtable_name, sizeof(vtable_name), "%s_vtable_instance", type_name);
    LLVMValueRef vtable_ptr = LLVMGetNamedGlobal(module, vtable_name);
    if (vtable_ptr)
    {
        LLVMValueRef vtable_field_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, 1, "vtable_ptr");
        LLVMBuildStore(builder, vtable_ptr, vtable_field_ptr);
    }
    printf("Debug: Set vtable pointer for %s\n", vtable_name);
    // Initialize parent instance if it exists (index 2)
    if (type_def->data.type_node.parent_name[0] != '\0' && type_def->data.type_node.parent_instance)
    {
        printf("Debug: Creating parent instance of type %s\n", type_def->data.type_node.parent_name);

        // Generar la instancia del padre - los parámetros ya están en el scope actual
        LLVMValueRef parent_instance = accept_gen(v, type_def->data.type_node.parent_instance);
        if (parent_instance)
        {
            LLVMValueRef parent_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, 2, "parent_ptr");
            LLVMBuildStore(builder, parent_instance, parent_ptr);
            printf("Debug: Parent instance created and stored in child\n");
        }
    }
    printf("Debug: Initialized parent instance if exists\n");
    // Initialize constructor parameters and fields
    for (int i = 0; i < node->data.type_node.arg_count; i++)
    {
        const char *param_name = type_def->data.type_node.args[i]->data.variable_name;

        Type *param_type = type_def->data.type_node.args[i]->return_type;
        LLVMValueRef arg_value;

        if (type_equals(param_type, &TYPE_STRING))
        {
            // For strings, just use the value directly
            arg_value = lookup_variable(param_name);
        }
        else
        {
            // For other types, load from alloca
            LLVMValueRef param_alloca = lookup_variable(param_name);
            arg_value = LLVMBuildLoad2(builder,
                                       LLVMGetElementType(LLVMTypeOf(param_alloca)),
                                       param_alloca,
                                       "param_val");
        }

        // LLVMValueRef arg_value = accept_gen(v, node->data.type_node.args[i]);

        int field_index = -1;

        for (int j = 0; j < type_def->data.type_node.def_count; j++)
        {
            ASTNode *def = type_def->data.type_node.definitions[j];
            if (def->type == NODE_ASSIGNMENT)
            {
                if (def->data.op_node.right->type == NODE_VARIABLE &&
                    strcmp(def->data.op_node.right->data.variable_name, param_name) == 0)
                {
                    field_index = find_field_index(type, def->data.op_node.left->data.variable_name);
                    break;
                }
            }
        }
        printf("Debug: Parameter '%s' maps to field index %d\n", param_name, field_index);
        if (field_index >= 0)
        {
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(
                builder, struct_type, instance, field_index, "field_ptr");
            printf("Debug: Field pointer for '%s': %s\n", param_name, LLVMPrintValueToString(field_ptr));

            // If arg_value is a pointer to double (parameter alloca), load the actual value
            if (LLVMGetTypeKind(LLVMTypeOf(arg_value)) == LLVMPointerTypeKind &&
                LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(arg_value))) == LLVMDoubleTypeKind)
            {
                arg_value = LLVMBuildLoad2(builder, LLVMDoubleTypeInContext(context), arg_value, "param_val");
            }

            LLVMBuildStore(builder, arg_value, field_ptr);
        }
        printf("Debug: Set field '%s' to value %s\n", param_name, LLVMPrintValueToString(arg_value));
    }

    // Initialize default field values for non-parameter fields
    if (node->data.type_node.arg_count == 0)
    {
        for (int j = 0; j < type_def->data.type_node.def_count; j++)
        {
            ASTNode *def = type_def->data.type_node.definitions[j];
            if (def->type == NODE_ASSIGNMENT)
            {
                if (def->data.op_node.right->type != NODE_VARIABLE)
                {
                    LLVMValueRef def_val = accept_gen(v, def->data.op_node.right);
                    const char *field_name = def->data.op_node.left->data.variable_name;
                    int field_index = find_field_index(type, field_name);
                    if (field_index >= 0)
                    {
                        LLVMValueRef field_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, field_index, "field_ptr_default");
                        LLVMBuildStore(builder, def_val, field_ptr);
                    }
                }
            }
        }
    }
    pop_scope();
    return instance;
}

LLVMValueRef generate_method_call(LLVM_Visitor *v, ASTNode *node)
{
    // Stack depth tracking
    LLVMValueRef current_stack_depth_var = LLVMGetNamedGlobal(module, "current_stack_depth");
    LLVMValueRef current_depth = LLVMBuildLoad2(builder, LLVMInt32Type(), current_stack_depth_var, "current_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, current_depth, LLVMConstInt(LLVMInt32Type(), 1, 0), "new_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);

    // Stack overflow check
    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT,
                                     new_depth, LLVMConstInt(LLVMInt32Type(), MAX_STACK_DEPTH, 0),
                                     "cmp_overflow_call");

    // Create basic blocks for overflow handling
    LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
    LLVMValueRef current_func = LLVMGetBasicBlockParent(current_bb);
    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(current_func, "stack_overflow_call");
    LLVMBasicBlockRef call_block = LLVMAppendBasicBlock(current_func, "method_call_body");

    LLVMBuildCondBr(builder, cmp, error_block, call_block);

    LLVMPositionBuilderAtEnd(builder, error_block);
    handle_stack_overflow(
        builder, module, current_stack_depth_var, node->line,
        node->data.op_node.right->data.func_node.name);

    LLVMPositionBuilderAtEnd(builder, call_block);

    // Get instance and method info
    LLVMValueRef instance_ptr = accept_gen(v, node->data.op_node.left);
    const char *method_name = node->data.op_node.right->data.func_node.name;
    ASTNode **args = node->data.op_node.right->data.func_node.args;
    int arg_count = node->data.op_node.right->data.func_node.arg_count;
    printf("Debug: Generating method call for %s with %d args\n", method_name, arg_count);
    // Get instance type and target type for the method
    Type *instance_type = node->data.op_node.left->dynamic_type;
    if (!instance_type)
    {
        printf("Error: NULL instance type\n");
        return NULL;
    }
    printf("Debug: Looking for method %s in type %s\n", method_name, instance_type->name);

    // First try to find the method directly in the current type
    Type *method_class = NULL;
    // snprintf(mangled_name, sizeof(mangled_name), "_%s_%s", instance_type->name, method_name);

    if (LLVMGetNamedFunction(module, method_name))
    {
        method_class = instance_type;
        printf("Debug: Found method %s directly in type %s\n", method_name, instance_type->name);
    }
    else
    {
        // If not found in current type, search up the hierarchy
        Type *current_type = instance_type->parent;
        while (current_type && !is_builtin_type(current_type))
        {
            printf("Debug: Searching in parent type %s\n", current_type->name);
            // snprintf(method_name, sizeof(method_name), "_%s_%s", current_type->name, method_name);
            // Construct the method name for the current parent type
            char parent_method[256];
            const char *base_method = strrchr(method_name, '_');
            if (base_method)
            {
                base_method++; // Skip the underscore
                snprintf(parent_method, sizeof(parent_method), "_%s_%s", current_type->name, base_method);
                printf("Debug: Looking for method %s in parent type %s\n", parent_method, current_type->name);

                if (LLVMGetNamedFunction(module, parent_method))
                {
                    method_class = current_type;
                    method_name = strdup(parent_method);
                    printf("Debug: Found method %s in parent type %s\n", method_name, current_type->name);
                    break;
                }
            }
            current_type = current_type->parent;
        }
    }

    if (!method_class)
    {
        printf("Error: Method %s not found in type hierarchy starting from %s\n",
               method_name, instance_type->name);
        return NULL;
    }

    // Get parent instance if method is in a parent class
    LLVMValueRef target_instance = instance_ptr;
    LLVMTypeRef source_struct = LLVMGetTypeByName(module, method_class->name);
    if (!source_struct)
    {
        printf("Error: Could not find struct type for %s\n", method_class->name);
        return NULL;
    }

    // If the method is in a different class than the instance type, we need to get the parent instance
    if (method_class != instance_type)
    {
        printf("Debug: Method is in parent class %s, getting parent instance\n", method_class->name);
        // First get the current instance
        LLVMValueRef current_instance;
        if (LLVMGetTypeKind(LLVMTypeOf(target_instance)) == LLVMPointerTypeKind &&
            LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(target_instance))) == LLVMPointerTypeKind)
        {
            current_instance = LLVMBuildLoad2(builder, LLVMGetElementType(LLVMTypeOf(target_instance)), target_instance, "loaded_instance");
        }
        else
        {
            current_instance = target_instance;
        }

        // Get the parent instance pointer (at index 2)
        LLVMTypeRef current_struct = LLVMGetTypeByName(module, instance_type->name);
        LLVMValueRef parent_ptr = LLVMBuildStructGEP2(builder, current_struct, current_instance, 2, "parent_ptr");
        target_instance = LLVMBuildLoad2(builder, LLVMPointerType(source_struct, 0), parent_ptr, "parent_instance");
        printf("Debug: Got parent instance of type %s\n", method_class->name);
    }

    // Use the target instance (either original or parent)
    LLVMValueRef instance = target_instance;

    // Ensure we have a properly dereferenced instance pointer
    if (LLVMGetTypeKind(LLVMTypeOf(instance)) == LLVMPointerTypeKind &&
        LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(instance))) == LLVMPointerTypeKind)
    {
        instance = LLVMBuildLoad2(builder, LLVMGetElementType(LLVMTypeOf(instance)), instance, "dereferenced_instance");
    }

    printf("Debug: Instance type after dereference = %s\n", LLVMPrintTypeToString(LLVMTypeOf(instance)));

    // Create vtable type name using the type that defines the method
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", method_class->name);
    printf("Debug: Looking for vtable type: %s\n", vtable_type_name);

    LLVMTypeRef vtable_type = LLVMGetTypeByName(module, vtable_type_name);
    if (!vtable_type)
    {
        printf("Error: Could not find vtable type %s\n", vtable_type_name);
        return NULL;
    }

    // Print struct layout
    printf("Debug: Source struct type: %s\n", LLVMPrintTypeToString(source_struct));

    // Get vtable pointer from instance (using the correct type's vtable)
    LLVMValueRef vtable_ptr_ptr = LLVMBuildStructGEP2(builder,
                                                      source_struct,
                                                      instance,
                                                      1, // vtable pointer is at index 1
                                                      "vtable_ptr_ptr");
    printf("Debug: vtable_ptr_ptr: %s\n", LLVMPrintValueToString(vtable_ptr_ptr));

    // Get the actual vtable global (using the method class's vtable)
    char vtable_instance_name[256];
    snprintf(vtable_instance_name, sizeof(vtable_instance_name), "%s_vtable_instance", method_class->name);
    LLVMValueRef vtable_global = LLVMGetNamedGlobal(module, vtable_instance_name);
    if (!vtable_global)
    {
        printf("Error: Could not find vtable instance %s\n", vtable_instance_name);
        return NULL;
    }
    printf("Debug: Found vtable global: %s\n", LLVMPrintValueToString(vtable_global));

    // Load vtable pointer
    LLVMValueRef vtable_ptr = LLVMBuildLoad2(builder, LLVMTypeOf(vtable_global), vtable_ptr_ptr, "vtable_ptr");
    printf("Debug: vtable_ptr loaded: %s\n", LLVMPrintValueToString(vtable_ptr));

    // Get method from vtable
    LLVMValueRef method;
    char method_mangled_name[256];
    snprintf(method_mangled_name, sizeof(method_mangled_name), "%s_%s", method_class->name, method_name);
    printf("Debug: Looking for method: %s\n", method_mangled_name);

    // Find method index in vtable
    int method_index = -1;
    int current_method_index = 0; // Solo contar métodos, no campos
    ASTNode *class_def = method_class->dec;
    for (int i = 0; i < class_def->data.type_node.def_count; i++)
    {
        ASTNode *def = class_def->data.type_node.definitions[i];
        if (def->type == NODE_FUNC_DEC)
        {
            if (strcmp(def->data.func_node.name, method_name) == 0)
            {
                method_index = current_method_index;
                printf("Debug: Found method %s at index %d in vtable\n", method_name, method_index);
                break;
            }
            current_method_index++; // Incrementar solo cuando encontramos un método
        }
    }

    if (method_index >= 0)
    {
        printf("Debug: Accessing method at index %d in vtable\n", method_index);
        printf("Debug: vtable_ptr type = %s\n", LLVMPrintTypeToString(LLVMTypeOf(vtable_ptr)));

        // Get method pointer from vtable
        LLVMValueRef func_ptr_ptr = LLVMBuildStructGEP2(
            builder,
            LLVMGetElementType(LLVMTypeOf(vtable_ptr)), // Use actual vtable type
            vtable_ptr,
            method_index,
            "method_ptr_ptr");

        if (!func_ptr_ptr)
        {
            printf("Error: Failed to get method pointer from vtable at index %d\n", method_index);
            return NULL;
        }
        printf("Debug: Got function pointer pointer: %s\n", LLVMPrintValueToString(func_ptr_ptr));

        // Create the function type for method
        ASTNode *method_def = NULL;
        for (int i = 0; i < class_def->data.type_node.def_count; i++)
        {
            ASTNode *def = class_def->data.type_node.definitions[i];
            if (def->type == NODE_FUNC_DEC && strcmp(def->data.func_node.name, method_name) == 0)
            {
                method_def = def;
                break;
            }
        }

        if (!method_def)
        {
            printf("Error: Could not find method definition for %s\n", method_name);
            return NULL;
        }

        // Create function type with proper parameters
        int param_count = method_def->data.func_node.arg_count + 1; // +1 for this pointer
        LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
        param_types[0] = LLVMPointerType(source_struct, 0); // this pointer

        for (int i = 0; i < method_def->data.func_node.arg_count; i++)
        {
            param_types[i + 1] = get_llvm_type(method_def->data.func_node.args[i]->return_type);
        }

        // Get the correct return type from method definition
        LLVMTypeRef return_type;
        if (method_def->data.func_node.body->return_type)
        {
            return_type = get_llvm_type(method_def->data.func_node.body->return_type);
            printf("Debug: Method %s has return type %s\n",
                   method_def->data.func_node.name,
                   LLVMPrintTypeToString(return_type));
        }
        else
        {
            return_type = LLVMVoidType();
            printf("Debug: Method %s has void return type\n",
                   method_def->data.func_node.name);
        }

        // Create the function type and pointer type
        LLVMTypeRef func_type = LLVMFunctionType(return_type, param_types, param_count, 0);
        LLVMTypeRef func_ptr_type = LLVMPointerType(func_type, 0);

        printf("Debug: Method function type: %s\n", LLVMPrintTypeToString(func_ptr_type));

        // Load the method with the correct function pointer type
        method = LLVMBuildLoad2(
            builder,
            func_ptr_type, // Use the exact function pointer type we created
            func_ptr_ptr,
            "method_ptr");

        free(param_types);
        printf("Debug: Method loaded with type: %s\n", LLVMPrintTypeToString(LLVMTypeOf(method)));
    }
    else
    {
        // Direct method call if not virtual
        method = LLVMGetNamedFunction(module, method_mangled_name);
        printf("Debug: Using direct method call: %s\n", method_mangled_name);
    }

    if (!method)
    {
        printf("Error: Could not find method %s\n", method_mangled_name);
        return NULL;
    }

    // Create argument list with instance as first argument
    LLVMValueRef *call_args = malloc((arg_count + 1) * sizeof(LLVMValueRef));
    call_args[0] = instance;

    // Add remaining arguments
    for (int i = 0; i < arg_count; i++)
    {
        call_args[i + 1] = accept_gen(v, args[i]);
    }

    // Make the call
    LLVMTypeRef func_type = LLVMTypeOf(method);
    LLVMValueRef result;
    printf("Debug: Function type for call: %s\n", LLVMPrintTypeToString(func_type));

    // First get the actual function type from the function pointer type if needed
    LLVMTypeRef actual_func_type = func_type;
    if (LLVMGetTypeKind(func_type) == LLVMPointerTypeKind)
    {
        actual_func_type = LLVMGetElementType(func_type);
    }

    // Check if the function return type is void
    LLVMTypeRef return_type = LLVMGetReturnType(actual_func_type);
    printf("Debug: Function return type for call: %s\n", LLVMPrintTypeToString(return_type));
    if (LLVMGetTypeKind(return_type) == LLVMVoidTypeKind)
    {
        // For void methods, use LLVMBuildCall and completely ignore the result
        LLVMBuildCall2(builder, actual_func_type, method, call_args, arg_count + 1, "");
        result = NULL;
    }
    else
    {
        // For methods that return a value, use LLVMBuildCall2 and return the pointer directly
        result = LLVMBuildCall2(builder, actual_func_type, method, call_args, arg_count + 1, "call_result");

        // Important: We do NOT load the result - keep it as a pointer
        // The caller will handle dereferencing if needed
    }
    free(call_args);

    // Restore stack depth
    LLVMBuildStore(builder, current_depth, current_stack_depth_var);

    return result;
}

LLVMValueRef generate_set_attr(LLVM_Visitor *v, ASTNode *node)
{
    fprintf(stderr,"ESTO ES EL SETTER CODEGEN\n");

    LLVMValueRef instance = accept_gen(v, node->data.cond_node.cond);
    printf("Debug: Setting attribute node type: %s\n", node->data.cond_node.body_false->data.variable_name);
    const char *property_name = node->data.cond_node.body_true->data.variable_name;
    LLVMValueRef new_value = accept_gen(v, node->data.cond_node.body_false);
    LLVMTypeRef instanceType = LLVMTypeOf(instance);
    if (LLVMGetTypeKind(instanceType) == LLVMPointerTypeKind)
    {
        LLVMTypeRef elementType = LLVMGetElementType(instanceType);
        if (LLVMGetTypeKind(elementType) == LLVMPointerTypeKind)
        {
            LLVMTypeRef pointee_type = LLVMGetElementType(LLVMTypeOf(instance));
            instance = LLVMBuildLoad2(builder, pointee_type, instance, "loaded_instance");
        }
    }

    LLVMTypeRef instanceTypeRef = LLVMTypeOf(instance);
    Type *instance_type = node->data.cond_node.cond->return_type;

    int field_index = find_field_index(instance_type, property_name);

    LLVMTypeRef struct_type = LLVMGetElementType(instanceTypeRef);
    LLVMValueRef field_ptr = LLVMBuildStructGEP2(
        builder,
        struct_type,
        instance,
        field_index,
        "field_ptr");

    LLVMBuildStore(builder, new_value, field_ptr);

    return new_value;
}

// LLVMValueRef generate_test_type(LLVM_Visitor* v, ASTNode* node) {
//     // Generate code to get the instance
//     LLVMValueRef instance = accept_gen(v, node->data.cast_test.exp);

//     // Make sure we have a properly dereferenced instance pointer
//     if (LLVMGetTypeKind(LLVMTypeOf(instance)) == LLVMPointerTypeKind &&
//         LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(instance))) == LLVMPointerTypeKind) {
//         instance = LLVMBuildLoad2(builder, LLVMGetElementType(LLVMTypeOf(instance)), instance, "dereferenced_instance");
//     }

//     // Get the target type's ID
//     Type* target_type = node->data.cast_test.type;

//     if (is_builtin_type(target_type)) {
//         int eq = type_equals(
//             target_type, node->data.cast_test.exp->return_type
//         );
//         return LLVMConstInt(LLVMInt1Type(), eq, 0);
//     }

//     // Get the struct type of the instance
//     LLVMTypeRef struct_type = LLVMGetElementType(LLVMTypeOf(instance));

//     // Get the ID field pointer (ID is always at index 0)
//     LLVMValueRef id_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, 0, "type_id_ptr");

//     // Load the actual ID value
//     LLVMValueRef instance_id = LLVMBuildLoad2(builder, LLVMInt32Type(), id_ptr, "instance_type_id");

//     int target_type_id = target_type->dec->data.type_node.id;
//     LLVMValueRef target_id = LLVMConstInt(LLVMInt32Type(), target_type_id, 0);

//     // Compare IDs and return the i1 result directly
//     return LLVMBuildICmp(builder, LLVMIntEQ, instance_id, target_id, "type_test");
// }

// LLVMValueRef generate_cast_type(LLVM_Visitor* v, ASTNode* node) {
//     LLVMValueRef check = generate_test_type(v, node);
//     // Si check es falso, imprime error y termina ejecución
//     LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
//     LLVMValueRef current_func = LLVMGetBasicBlockParent(current_bb);
//     LLVMBasicBlockRef ok_block = LLVMAppendBasicBlock(current_func, "cast.ok");
//     LLVMBasicBlockRef err_block = LLVMAppendBasicBlock(current_func, "cast.error");

//     LLVMBuildCondBr(builder, check, ok_block, err_block);

//     // Bloque de error: imprime mensaje y termina
//     LLVMPositionBuilderAtEnd(builder, err_block);
//     char error_msg[256];
//     snprintf(error_msg, sizeof(error_msg),
//         RED"!!RUNTIME ERROR: Type cast failed. Line %d."RESET, node->line);
//     LLVMValueRef msg = LLVMBuildGlobalStringPtr(builder, error_msg, "cast_error_msg");
//     LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
//     LLVMBuildCall2(builder, LLVMGetElementType(LLVMTypeOf(puts_func)), puts_func, &msg, 1, "");
//     LLVMValueRef exit_func = LLVMGetNamedFunction(module, "exit");
//     LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 0, 0);
//     LLVMTypeRef exit_func_type = LLVMFunctionType(LLVMVoidType(), (LLVMTypeRef[]){LLVMInt32Type()}, 1, 0);
//     LLVMBuildCall2(builder, exit_func_type, exit_func, &exit_code, 1, "");

//     //LLVMBuildCall(builder, exit_func, &exit_code, 1, "");
//     LLVMBuildUnreachable(builder);

//     // Continuar en el bloque ok
//     LLVMPositionBuilderAtEnd(builder, ok_block);
//     // Generate code to get the instance
//     LLVMValueRef instance = accept_gen(v, node->data.cast_test.exp);

//     // Make sure we have a properly dereferenced instance pointer
//     if (LLVMGetTypeKind(LLVMTypeOf(instance)) == LLVMPointerTypeKind &&
//         LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(instance))) == LLVMPointerTypeKind) {
//         instance = LLVMBuildLoad2(builder, LLVMGetElementType(LLVMTypeOf(instance)), instance, "dereferenced_instance");
//     }

//     Type* target_type = node->data.cast_test.type;

//     if (is_builtin_type(target_type)) {
//         int eq = type_equals(
//             target_type, node->data.cast_test.exp->return_type
//         );
//         return instance;
//     }

//     // Get the struct type of the instance
//     LLVMTypeRef struct_type = LLVMGetElementType(LLVMTypeOf(instance));

//     // Get the ID field pointer (ID is always at index 0)
//     LLVMValueRef id_ptr = LLVMBuildStructGEP2(builder, struct_type, instance, 0, "type_id_ptr");

//     // Load the actual ID value
//     LLVMValueRef instance_id = LLVMBuildLoad2(builder, LLVMInt32Type(), id_ptr, "instance_type_id");

//     // Get the target type's ID and type
//     int target_type_id = target_type->dec->data.type_node.id;
//     LLVMValueRef target_id = LLVMConstInt(LLVMInt32Type(), target_type_id, 0);

//     // Get target struct type
//     LLVMTypeRef target_struct_type = LLVMGetTypeByName(module, target_type->name);

//     // Create basic blocks for the cast
//     LLVMValueRef current_function = LLVMGetBasicBlockParent(LLVMGetInsertBlock(builder));
//     LLVMBasicBlockRef success_block = LLVMAppendBasicBlock(current_function, "cast.success");
//     LLVMBasicBlockRef fail_block = LLVMAppendBasicBlock(current_function, "cast.fail");
//     LLVMBasicBlockRef merge_block = LLVMAppendBasicBlock(current_function, "cast.end");

//     // Compare IDs
//     LLVMValueRef id_match = LLVMBuildICmp(builder, LLVMIntEQ, instance_id, target_id, "type_match");
//     LLVMBuildCondBr(builder, id_match, success_block, fail_block);

//     // Success block: return the instance cast to target type
//     LLVMPositionBuilderAtEnd(builder, success_block);
//     LLVMValueRef cast_result = LLVMBuildBitCast(builder, instance, LLVMPointerType(target_struct_type, 0), "cast_result");
//     LLVMBuildBr(builder, merge_block);

//     // Failure block: return null of target type
//     LLVMPositionBuilderAtEnd(builder, fail_block);
//     LLVMValueRef null_result = LLVMConstNull(LLVMPointerType(target_struct_type, 0));
//     LLVMBuildBr(builder, merge_block);

//     // Merge block: phi node to select between success and failure results
//     LLVMPositionBuilderAtEnd(builder, merge_block);
//     LLVMValueRef phi = LLVMBuildPhi(builder, LLVMPointerType(target_struct_type, 0), "cast_phi");
//     LLVMValueRef incoming_values[] = {cast_result, null_result};
//     LLVMBasicBlockRef incoming_blocks[] = {success_block, fail_block};
//     LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

//     return phi;
// }

// LLVMValueRef generate_base_call(LLVM_Visitor* v, ASTNode* node) {
//     // Stack depth tracking - igual que en method_call
//     LLVMValueRef current_stack_depth_var = LLVMGetNamedGlobal(module, "current_stack_depth");
//     LLVMValueRef current_depth = LLVMBuildLoad2(builder, LLVMInt32Type(), current_stack_depth_var, "current_depth");
//     LLVMValueRef new_depth = LLVMBuildAdd(builder, current_depth, LLVMConstInt(LLVMInt32Type(), 1, 0), "new_depth");
//     LLVMBuildStore(builder, new_depth, current_stack_depth_var);

//     // Stack overflow check
//     LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT,
//                                     new_depth, LLVMConstInt(LLVMInt32Type(), MAX_STACK_DEPTH, 0),
//                                     "cmp_overflow_base");

//     // Create basic blocks for overflow handling
//     LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(builder);
//     LLVMValueRef current_function = LLVMGetBasicBlockParent(current_bb);
//     LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(current_function, "stack_overflow_base");
//     LLVMBasicBlockRef call_block = LLVMAppendBasicBlock(current_function, "base_call_body");

//     LLVMBuildCondBr(builder, cmp, error_block, call_block);

//     // Handle stack overflow
//     LLVMPositionBuilderAtEnd(builder, error_block);
//     handle_stack_overflow(
//         builder, module, current_stack_depth_var, node->line,
//         node->data.func_node.name
//     );

//     LLVMPositionBuilderAtEnd(builder, call_block);

//     // Get the current instance (this) - usando LLVMGetParam que es la API correcta
//     LLVMValueRef this_ptr = LLVMGetParam(current_function, 0);

//     // Get the parent instance (at index 2 in the struct)
//     LLVMValueRef parent_ptr = LLVMBuildStructGEP2(
//         builder,
//         LLVMGetElementType(LLVMTypeOf(this_ptr)),
//         this_ptr,
//         2, // parent instance index
//         "parent_instance"
//     );

//     // Load the parent instance
//     LLVMValueRef parent_instance = LLVMBuildLoad2(
//         builder,
//         LLVMGetElementType(LLVMTypeOf(parent_ptr)),
//         parent_ptr,
//         "loaded_parent"
//     );

//     // Get method info from the base type
//     const char* method_name = node->data.func_node.name;
//     Type* base_type = node->return_type->parent; // El tipo base es el padre del tipo actual

//     // Construct the method name for the parent class
//     char parent_method[256];
//     snprintf(parent_method, sizeof(parent_method), "_%s_%s", base_type->name, method_name);

//     // Look up the method in the module
//     LLVMValueRef base_method = LLVMGetNamedFunction(module, parent_method);
//     if (!base_method) {
//         fprintf(stderr, "Error: Base method %s not found in parent class %s\n",
//                 method_name, base_type->name);
//         return NULL;
//     }

//     // Prepare arguments for the call
//     unsigned int arg_count = node->data.func_node.arg_count + 1; // +1 for parent instance
//     LLVMValueRef* args = malloc(sizeof(LLVMValueRef) * arg_count);
//     args[0] = parent_instance; // First argument is the parent instance

//     // Generate code for the rest of the arguments using accept_gen
//     for (unsigned int i = 0; i < node->data.func_node.arg_count; i++) {
//         args[i + 1] = accept_gen(v, node->data.func_node.args[i]);
//         if (!args[i + 1]) {
//             free(args);
//             return NULL;
//         }
//     }

//     // Make the call to the base method
//     LLVMValueRef result = LLVMBuildCall2(
//         builder,
//         LLVMGetElementType(LLVMTypeOf(base_method)),
//         base_method,
//         args,
//         arg_count,
//         "base_call_result"
//     );

//     // Cleanup
//     free(args);

//     // Decrement stack depth
//     LLVMBuildStore(builder, current_depth, current_stack_depth_var);

//     return result;
// }