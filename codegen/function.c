
#include "codegen.h"
#include <stdio.h>
#include <string.h>

// #define DEFINE_MATH_FUNC(name)                                 \
//     LLVMValueRef name##_handler(LLVMVisitor *v, ASTNode *node) \
//     {                                                          \
//         return handle_math(v, node, #name, #name "_tmp");      \
//     }

// DEFINE_MATH_FUNC(sqrt)
// DEFINE_MATH_FUNC(sin)
// DEFINE_MATH_FUNC(cos)
// DEFINE_MATH_FUNC(exp)

LLVMValueRef codegen_call_function(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ANDO EN EL CODEGEN DE CALL FUNCTION: nombre de la funcion %s\n", node->data.func_node.name);

    if (!strcmp(node->data.func_node.name, "print"))
    {
        return codegen_print(v, node);
    }
    if (!strcmp(node->data.func_node.name, "sqrt"))
    {
        return codegen_math_function(v, node, "sqrt", "sqrt_tmp");
    }
    if (!strcmp(node->data.func_node.name, "sin"))
    {
        return codegen_math_function(v, node, "sin", "sin_tmp");
    }
    if (!strcmp(node->data.func_node.name, "cos"))
    {
        return codegen_math_function(v, node, "cos", "cos_tmp");
    }
    if (!strcmp(node->data.func_node.name, "exp"))
    {
        return codegen_math_function(v, node, "exp", "exp_tmp");
    }
    if (!strcmp(node->data.func_node.name, "log"))
    {
        return codegen_log(v, node);
    }
    if (!strcmp(node->data.func_node.name, "rand"))
    {
        return codegen_rand(v, node);
    }

    fprintf(stderr, "VOY a entrar al codegen_custom_func\n");
    return codegen_custom_func(v, node);
}

LLVMValueRef codegen_print(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "JABEL RESENDIZ\n");
    LLVMValueRef printf_func = LLVMGetNamedFunction(v->ctx->module, "printf");

    // Si no hay argumentos, solo imprime una nueva línea
    if (node->data.func_node.arg_count == 0)
    {
        LLVMValueRef format_str = LLVMBuildGlobalStringPtr(v->ctx->builder, "\n", "newline");
        LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                                                   (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);
        return LLVMBuildCall2(v->ctx->builder, printf_type, printf_func,
                              (LLVMValueRef[]){format_str}, 1, "printf_call");
    }

    // Generar código para el argumento
    ASTNode *arg_node = node->data.func_node.args[0];

     fprintf(stderr, YELLOW "1_DEBUG_PRINT_CALL\n" RESET);

     fprintf(stderr,"el tipo de mi node es %d\n", arg_node->type);

    LLVMValueRef arg = codegen_accept(v, arg_node);

    fprintf(stderr, YELLOW "2_DEBUG_PRINT_CALL\n" RESET);

    if (!arg)
        return NULL;

    const char *format = "";
    LLVMValueRef format_str;
    LLVMValueRef args[2];
    int num_args = 2;

    // Seleccionar formato según el tipo del argumento
    if (type_equals(arg_node->return_type, &TYPE_NUMBER))
    {
        format = "%g\n";
        format_str = LLVMBuildGlobalStringPtr(v->ctx->builder, format, "fmt");
        args[0] = format_str;
        args[1] = arg;
    }
    else if (type_equals(arg_node->return_type, &TYPE_BOOLEAN))
    {
        format_str = LLVMBuildGlobalStringPtr(v->ctx->builder, "%s\n", "fmt");
        LLVMValueRef true_str = LLVMBuildGlobalStringPtr(v->ctx->builder, "true", "true_str");
        LLVMValueRef false_str = LLVMBuildGlobalStringPtr(v->ctx->builder, "false", "false_str");
        LLVMValueRef cond_str = LLVMBuildSelect(v->ctx->builder, arg, true_str, false_str, "bool_str");
        args[0] = format_str;
        args[1] = cond_str;
    }
    else if (type_equals(arg_node->return_type, &TYPE_STRING))
    {
        format = "%s\n";
        format_str = LLVMBuildGlobalStringPtr(v->ctx->builder, format, "fmt");
        args[0] = format_str;
        args[1] = arg;
    }
    else
    {

        format = "%s\n";
        format_str = LLVMBuildGlobalStringPtr(v->ctx->builder, format, "fmt");
        LLVMValueRef unknown_str = LLVMBuildGlobalStringPtr(v->ctx->builder, "<unknown>", "unknown_str");
        args[0] = format_str;
        args[1] = unknown_str;
    }

    // Construir llamada a printf
    LLVMTypeRef printf_type = LLVMFunctionType(LLVMInt32Type(),
                                               (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)}, 1, 1);

    fprintf(stderr, "JABEL RESENDIZ");

    return LLVMBuildCall2(v->ctx->builder, printf_type, printf_func, args, num_args, "printf_call");
}

LLVMValueRef codegen_math_function(LLVMVisitor *v, ASTNode *node, const char *name, const char *tmp_name)
{
    LLVMValueRef arg = codegen_accept(v, node->data.func_node.args[0]);
    LLVMTypeRef type = LLVMFunctionType(LLVMDoubleType(),
                                        (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
    LLVMValueRef func = LLVMGetNamedFunction(v->ctx->module, name);
    return LLVMBuildCall2(v->ctx->builder, type, func,
                          (LLVMValueRef[]){arg}, 1, tmp_name);
}

LLVMValueRef codegen_rand(LLVMVisitor *v, ASTNode *node)
{
    LLVMTypeRef rand_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef rand_func = LLVMGetNamedFunction(v->ctx->module, "rand");

    // Convertir el resultado entero a double dividiendo por RAND_MAX
    LLVMValueRef rand_val = LLVMBuildCall2(v->ctx->builder, rand_type, rand_func, NULL, 0, "rand_tmp");
    LLVMValueRef rand_max = LLVMConstReal(LLVMDoubleType(), RAND_MAX);
    LLVMValueRef rand_double = LLVMBuildSIToFP(v->ctx->builder, rand_val, LLVMDoubleType(), "rand_double");

    return LLVMBuildFDiv(v->ctx->builder, rand_double, rand_max, "rand_result");
}

LLVMValueRef codegen_log(LLVMVisitor *v, ASTNode *node)
{

    if (node->data.func_node.arg_count == 1)
    {
        LLVMValueRef arg = codegen_accept(v, node->data.func_node.args[0]);
        LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
                                                (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef log_func = LLVMGetNamedFunction(v->ctx->module, "log");
        return LLVMBuildCall2(v->ctx->builder, log_type, log_func,
                              (LLVMValueRef[]){arg}, 1, "log_tmp");
    }
    // tiene 2 argumetnos
    else
    {
        // log(base, x) = log(x) / log(base)
        LLVMValueRef base = codegen_accept(v, node->data.func_node.args[0]);
        LLVMValueRef x = codegen_accept(v, node->data.func_node.args[1]);

        LLVMTypeRef log_type = LLVMFunctionType(LLVMDoubleType(),
                                                (LLVMTypeRef[]){LLVMDoubleType()}, 1, 0);
        LLVMValueRef log_func = LLVMGetNamedFunction(v->ctx->module, "log");

        LLVMValueRef log_x = LLVMBuildCall2(v->ctx->builder, log_type, log_func,
                                            (LLVMValueRef[]){x}, 1, "log_x");
        LLVMValueRef log_base = LLVMBuildCall2(v->ctx->builder, log_type, log_func,
                                               (LLVMValueRef[]){base}, 1, "log_base");

        return LLVMBuildFDiv(v->ctx->builder, log_x, log_base, "log_result");
    }
}


LLVMValueRef codegen_custom_func(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "estamos en el codegen call function\n");

    const char *name = node->data.func_node.name;
    int arg_count = node->data.func_node.arg_count;
    ASTNode **args = node->data.func_node.args;
    Type *return_type = node->return_type;

    fprintf(stderr, "1-ADNDA MIERDAAA\n");

    LLVMValueRef func = LLVMGetNamedFunction(v->ctx->module, name);

    if (!func)
    {
        fprintf(stderr, "Function '%s' not found.\n", name);
        exit(1);
    }

    fprintf(stderr, "2-ADNDA MIERDAAA\n");

    LLVMValueRef *arg_values = malloc(sizeof(LLVMValueRef) * arg_count);
    for (int i = 0; i < arg_count; i++)
    {
        arg_values[i] = codegen_accept(v, args[i]);
        if (!arg_values[i])
        {
            fprintf(stderr, "ERROR: arg_values[%d] is NULL\n", i);
            exit(1);
        }
    }

    fprintf(stderr, "3-ADNDA MIERDAAA\n");

    LLVMTypeRef *param_types = malloc(arg_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < arg_count; i++)
    {
        param_types[i] = type_to_llvm(v->ctx, args[i]->return_type);
    }

    fprintf(stderr, "4-ADNDA MIERDAAA\n");

    LLVMTypeRef return_llvm_type = type_to_llvm(v->ctx, return_type);

    LLVMTypeRef func_type = LLVMFunctionType(return_llvm_type, param_types, arg_count, 0);

    fprintf(stderr, "5-ADNDA MIERDAAA\n");

    const char *tmp_name = type_equals(return_type, &TYPE_VOID) ? "" : "calltmp";

    fprintf(stderr, "CHECKER\n");

    LLVMValueRef call = LLVMBuildCall2(
        v->ctx->builder,
        func_type,
        func,
        arg_values,
        arg_count,
        tmp_name);

    fprintf(stderr, "6-ADNDA MIERDAAA\n");

    free(arg_values);
    return call;
}

LLVMValueRef get_or_create_function(LLVMModuleRef module, const char *name,
                                    LLVMTypeRef return_type, LLVMTypeRef *param_types, int param_count)
{
    LLVMValueRef func = LLVMGetNamedFunction(module, name);
    if (!func)
    {
        LLVMTypeRef func_type = LLVMFunctionType(return_type, param_types, param_count, 0);

        if (LLVMGetTypeKind(func_type) != LLVMFunctionTypeKind)
        {
            fprintf(stderr, "ERROR: func_type is not a function type, sino %d\n", LLVMGetTypeKind(func_type));
            exit(1);
        }
        fprintf(stderr, "es una func_type \n");

        fprintf(stderr, "todo correcto\n");
        func = LLVMAddFunction(module, name, func_type);
    }

    fprintf(stderr, "es una funcion type, tl vez no\n");

    return func;
}

LLVMValueRef codegen_dec_function(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "estamos en el decl_function_codegen \n");

    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;
    LLVMValueRef current_stack_depth_var = v->ctx->current_stack_depth_var;
    int max_stack_depth = v->ctx->max_stack_depth;

    const char *name = node->data.func_node.name;
    Type *return_type = node->data.func_node.body->return_type;
    ASTNode **params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;
    ASTNode *body = node->data.func_node.body;

    // Obtener tipos de parámetros
    LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++)
    {
        param_types[i] = type_to_llvm(v->ctx, params[i]->return_type);
    }

    LLVMTypeRef ret_llvm_type = type_to_llvm(v->ctx, return_type);

    fprintf(stderr, "el tipo de retorno es %s\n", return_type->name);

    LLVMValueRef func = get_or_create_function(module, name, ret_llvm_type, param_types, param_count);
    v->current_function = func;
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(func, "function_exit");

    // Configurar builder
    LLVMPositionBuilderAtEnd(builder, entry);

    // 1. Stack depth handling
    // Increment counter
    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);

    fprintf(stderr, "2-HOOLAAAAAAAAAAAAA\n");

    // Verificar overflow
    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT, new_depth,
                                     LLVMConstInt(LLVMInt32Type(), max_stack_depth, 0), "cmp_overflow");

    fprintf(stderr, "3-HOOLAAAAAAAAAAAAA\n");

    // Crear bloques para manejo de error
    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(func, "stack_overflow");
    LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(func, "func_body");
    LLVMBuildCondBr(builder, cmp, error_block, continue_block);
    
     fprintf(stderr, "4-HOOLAAAAAAAAAAAAA\n");
    // Bloque de error
    LLVMPositionBuilderAtEnd(builder, error_block);
    llvm_handle_stack_overflow(v->ctx, node);

     fprintf(stderr, "5-HOOLAAAAAAAAAAAAA\n");
    // Continuar con función normal
    LLVMPositionBuilderAtEnd(builder, continue_block);

    push_scope();
         fprintf(stderr, "6-HOOLAAAAAAAAAAAAA\n");
    // Almacenar parámetros en variables locales
    for (int i = 0; i < param_count; i++)
    {
        LLVMValueRef param = LLVMGetParam(func, i);
        LLVMValueRef alloca = LLVMBuildAlloca(builder, param_types[i], params[i]->data.variable_name);
        LLVMBuildStore(builder, param, alloca);
        declare_variable(params[i]->data.variable_name, alloca);
    }
     fprintf(stderr, "8-HOOLAAAAAAAAAAAAA\n");
    // Generar código para el cuerpo
    LLVMValueRef body_val = codegen_accept(v, body);
    
     fprintf(stderr, "9-HOOLAAAAAAAAAAAAA\n");
    // Branch al bloque de salida
    LLVMBuildBr(builder, exit_block);

    // Bloque de salida
    LLVMPositionBuilderAtEnd(builder, exit_block);
     fprintf(stderr, "7-HOOLAAAAAAAAAAAAA\n");
    // Decrementar contador antes de retornar
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

    fprintf(stderr, "TODO CON EXITO EN EL DEC_FUCNTION\n");

    return func;
}



LLVMValueRef codegen_dec_method(LLVMVisitor* v, ASTNode* node, LLVMUserTypeInfo* type_info)
{
    fprintf(stderr, "Estamos en codegen_dec_method para el método: %s\n", node->data.func_node.name);

    LLVMModuleRef module = v->ctx->module;
    LLVMBuilderRef builder = v->ctx->builder;
    LLVMValueRef current_stack_depth_var = v->ctx->current_stack_depth_var;
    int max_stack_depth = v->ctx->max_stack_depth;

    const char *method_name = node->data.func_node.name;
    Type *return_type = node->data.func_node.body->return_type;
    ASTNode **explicit_params = node->data.func_node.args; // Parámetros declarados por el usuario
    int explicit_param_count = node->data.func_node.arg_count;
    ASTNode *body = node->data.func_node.body;

    // --- Parte específica para MÉTODOS ---
    // Necesitamos el tipo de la estructura de la clase a la que pertenece este método.
    // Asumo que 'node->data.func_node.parent_class_name' contiene el nombre de la clase.
    //const char *class_name = node->data.func_node.parent_class_name;
    LLVMUserTypeInfo *parent_class_type_info = type_info;

    LLVMTypeRef class_struct_ptr_type = LLVMPointerType(parent_class_type_info->struct_type, 0);

    // 1. Calcular el número total de parámetros LLVM: 'this' + parámetros explícitos
    int total_llvm_params = 1 + explicit_param_count;

    // 2. Asignar memoria para el array de tipos de parámetros LLVM
    LLVMTypeRef *llvm_param_types = malloc(total_llvm_params * sizeof(LLVMTypeRef));
    if (!llvm_param_types) {
        perror("Error en malloc para llvm_param_types en method_codegen");
        exit(EXIT_FAILURE);
    }

    // 3. El primer parámetro siempre es el puntero 'self' ('this')
    llvm_param_types[0] = class_struct_ptr_type;

    // 4. Agregar los tipos de los parámetros explícitos
    for (int i = 0; i < explicit_param_count; i++) {
        llvm_param_types[i + 1] = type_to_llvm(v->ctx, explicit_params[i]->return_type);
    }
    // --- Fin de la parte específica para MÉTODOS ---


    LLVMTypeRef ret_llvm_type = type_to_llvm(v->ctx, return_type);

    fprintf(stderr, "El tipo de retorno del método %s es %s\n", method_name, return_type->name);

    // Obtener o crear la función LLVM (usa el nombre decorado si es necesario, e.g., "_A_setX")
    // Para los métodos, el nombre de la función LLVM debe ser único, como "_<ClassName>_<MethodName>"
    // char decorated_method_name[256]; // Ajusta el tamaño según tus necesidades
    // snprintf(decorated_method_name, sizeof(decorated_method_name), "_%s_%s", type_info->name, method_name);

    LLVMValueRef func = get_or_create_function(module, method_name, ret_llvm_type, llvm_param_types, total_llvm_params);
    v->current_function = func; // Establece la función actual en el visitor
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
    LLVMBasicBlockRef exit_block = LLVMAppendBasicBlock(func, "function_exit");

    LLVMPositionBuilderAtEnd(builder, entry);

    // 1. Stack depth handling (igual que en tu codegen_dec_function)
    LLVMTypeRef int32_type = LLVMInt32Type();
    LLVMValueRef depth_val = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth");
    LLVMValueRef new_depth = LLVMBuildAdd(builder, depth_val, LLVMConstInt(int32_type, 1, 0), "inc_depth");
    LLVMBuildStore(builder, new_depth, current_stack_depth_var);

    LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSGT, new_depth,
                                     LLVMConstInt(LLVMInt32Type(), max_stack_depth, 0), "cmp_overflow");

    LLVMBasicBlockRef error_block = LLVMAppendBasicBlock(func, "stack_overflow");
    LLVMBasicBlockRef continue_block = LLVMAppendBasicBlock(func, "func_body");
    LLVMBuildCondBr(builder, cmp, error_block, continue_block);

    LLVMPositionBuilderAtEnd(builder, error_block);
    llvm_handle_stack_overflow(v->ctx, node);

    LLVMPositionBuilderAtEnd(builder, continue_block);

    push_scope();

    // 2. Almacenar parámetros en variables locales
    // --- ESTA PARTE ES CRÍTICA Y DIFERENTE ---
    int param_llvm_index = 0;

    // Almacenar el puntero 'self' en el ámbito
    const char *self_var_name = "self"; // O "this", según la sintaxis de tu lenguaje
    LLVMValueRef self_param_val = LLVMGetParam(func, param_llvm_index); // Obtiene el valor del primer parámetro LLVM (self)
    LLVMValueRef self_alloca = LLVMBuildAlloca(builder, LLVMTypeOf(self_param_val), self_var_name);
    LLVMBuildStore(builder, self_param_val, self_alloca);
    declare_variable(self_var_name, self_alloca); // Agrega 'self' a tu tabla de símbolos actual
    param_llvm_index++; // Avanza al siguiente índice de parámetro LLVM

    // Almacenar los parámetros explícitos en variables locales
    for (int i = 0; i < explicit_param_count; i++) {
        LLVMValueRef param_val = LLVMGetParam(func, param_llvm_index);
        LLVMValueRef alloca_var = LLVMBuildAlloca(builder, llvm_param_types[param_llvm_index], explicit_params[i]->data.variable_name);
        LLVMBuildStore(builder, param_val, alloca_var);
        declare_variable(explicit_params[i]->data.variable_name, alloca_var);
        param_llvm_index++;
    }
    // --- FIN DE LA PARTE CRÍTICA Y DIFERENTE ---

    // Generar código para el cuerpo del método
    LLVMValueRef body_val = codegen_accept(v, body);
    
    LLVMBuildBr(builder, exit_block);

    // Bloque de salida
    LLVMPositionBuilderAtEnd(builder, exit_block);
    
    // Decrementar contador antes de retornar
    LLVMValueRef final_depth = LLVMBuildLoad2(builder, int32_type, current_stack_depth_var, "load_depth_final");
    LLVMValueRef dec_depth = LLVMBuildSub(builder, final_depth, LLVMConstInt(int32_type, 1, 0), "dec_depth");
    LLVMBuildStore(builder, dec_depth, current_stack_depth_var);

    // Manejo del valor de retorno
    if (type_equals(return_type, &TYPE_VOID)) {
        LLVMBuildRetVoid(builder);
    } else if (body_val) {
        LLVMBuildRet(builder, body_val);
    } else {
        // Retorno por defecto 0.0 como double (puede necesitar lógica para otros tipos)
        LLVMBuildRet(builder, LLVMConstReal(LLVMDoubleType(), 0.0));
    }

    pop_scope();
    free(llvm_param_types);

    fprintf(stderr, "Método %s generado con éxito.\n", method_name);

    return func;
}
