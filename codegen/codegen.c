

#include "codegen.h"
#include "init_codegen.h"
#include "visitor_llvm.h"
#include "../scope/llvm_scope.h"
#include <stdio.h>
#include <string.h>
#include "../type/type.h"
#include "types.h"

// Genera el LLVM IR a partir de un AST
void compile_to_llvm(ASTNode *ast, const char *filename)
{
    fprintf(stderr, "11111111111111111111111111111\n");

    // Inicializar LLVM (Targets, Printers, etc. - solo una vez al inicio de la compilación)
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllAsmParsers();

    LLVMCoreContext *ctx = llvm_core_context_create(); // Crea e inicializa el contexto LLVM
    LLVMVisitor visitor = {.ctx = ctx};                // Inicializa el visitor con el contexto

    // Configura las funciones de codegen en el visitor
    visitor.basic.program = codegen_program;
    visitor.basic.number = codegen_number;
    visitor.basic.string = codegen_string;
    visitor.basic.boolean = codegen_boolean;
    visitor.basic.variable = codegen_variable;
    visitor.expr.binary = codegen_binary_op;
    visitor.expr.assignment = codegen_assignments;
    visitor.expr.call_function = codegen_call_function;
    visitor.control.block = codegen_block;
    visitor.control.dec_function = codegen_dec_function;
    visitor.control.let_in = codegen_let_in;
    visitor.control.conditional = codegen_conditional;
    visitor.control.while_loop = codegen_while;
    visitor.types.type_dec = codegen_type_dec; // Placeholder, make_body_type_dec hace el trabajo
    visitor.types.type_instance = codegen_type_instance;
    visitor.attrs.attr_getter = codegen_attr_getter;
    visitor.attrs.attr_setter = codegen_attr_setter;
    visitor.attrs.method_getter = codegen_method_getter;

    // Declara funciones built-in (malloc, printf, etc.)
    llvm_declare_builtins(ctx);

    fprintf(stderr, "candeal\n");

    // build_vtable_table(&visitor, ast);

    // FASE 1: Recolector Declaraciones de Tipos (solo los nombres de struct y vtables)
    find_type_dec(&visitor, ast);

    // FASE 2: Definir cuerpos de tipos y vtables, declarar metodos llvm
    // Esto se hace iterando sobre la lista de user_types, no sobre el AST completo de nuevo.
    LLVMUserTypeInfo *current_user_type = ctx->user_types;

    while (current_user_type)
    {

        for (int i = 0; i < current_user_type->num_methods_virtual; i++)
        {
            codegen_dec_method(&visitor, current_user_type->methods[i]->node,current_user_type);
        }

        current_user_type = current_user_type->next;
    }

    // FASE 3: Registrar las firmas de funciones "libres" (no métodos de clase)
    find_function_dec(&visitor, ast);

    // FASE 4: Generar los cuerpos de las funciones (incluyendo métodos de clase)

    // // Generar cuerpos de MÉTODOS DE CLASE
    // LLVMUserTypeInfo *current_user_type_info_phase4 = ctx->user_types; // Reinicia el puntero
    // while(current_user_type_info_phase4 != NULL) {
    //     fprintf(stderr,"estamos aqui en compile to llvm\n");
    //     for(int i = 0; i < current_user_type_info_phase4->num_methods_virtual; i++) {
    //         // ¡Aquí es donde llamas a codegen_method_body!
    //         fprintf(stderr,"debe haber algun metodo\n");
    //         codegen_method_body(&visitor, // Pasar el visitor
    //                             current_user_type_info_phase4,
    //                             current_user_type_info_phase4->method_decl_nodes_ast[i],
    //                             current_user_type_info_phase4->method_llvm_funcs[i]);
    //     }
    //     current_user_type_info_phase4 = current_user_type_info_phase4->next;
    // }

    // Generar cuerpos de funciones "libres" o no-métodos
    make_body_function_dec(&visitor, ast);

    // Create scope for main function
    push_scope();

    // Crea la función main con retorno i32 0
    LLVMTypeRef main_type = LLVMFunctionType(ctx->i32_type, NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(ctx->module, "main", main_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(ctx->context, main_func, "entry");

    // Posiciona el builder en el bloque de entrada de main
    LLVMPositionBuilderAtEnd(ctx->builder, entry);

    if (ast)
    {
        fprintf(stderr, "vamos a generar el codigo \n");
        // Llama a codegen para recorrer el AST completo y generar el código principal de main
        // Esto incluirá las llamadas a codegen_type_instance si ocurren en el programa principal.
        codegen_accept(&visitor, ast);
    }

    // Asegura que el bloque de entrada de main termine con un retorno
    LLVMBasicBlockRef current_block_in_main = LLVMGetInsertBlock(ctx->builder);
    if (!LLVMGetBasicBlockTerminator(current_block_in_main))
    {
        LLVMBuildRet(ctx->builder, LLVMConstInt(ctx->i32_type, 0, 0));
    }

    // Pop scope for main function
    pop_scope();

    fprintf(stderr, GREEN "Compilación a LLVM IR finalizada.\n" RESET);

    // Escribe en el archivo .ll
    char *error = NULL;
    fprintf(stderr, "TODO PASA AQUI\n");

    fprintf(stderr, "EL nombre del filename es %s\n", filename);

    if (ctx->module == NULL)
    {
        fprintf(stderr, "es nulo el ctx module \n");
    }

    if (LLVMPrintModuleToFile(ctx->module, filename, &error))
    {
        fprintf(stderr, "O AQUI\n");
        fprintf(stderr, RED "Error writing IR: %s\n" RESET, error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    // Libera recursos
    fprintf(stderr, YELLOW "Hasta aqui todo bien\n" RESET);
    free_llvm_core_context(ctx);
    fprintf(stderr, YELLOW "se libero la memoria del ctx bien\n" RESET);
}

LLVMValueRef codegen_program(LLVMVisitor *v, ASTNode *node)
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

    pop_scope();
    return last ? last : LLVMConstInt(LLVMInt32Type(), 0, 0);
}

// FASE 0: find_type_dec (Solo recolecta nombres de tipos y los declara globalmente en LLVM)

void find_function_dec(LLVMVisitor *visitor, ASTNode *node)
{
    fprintf(stderr, "ESTOY EN EL CODEGEN.C - FIND_FUNCTION_DEC\n");

    if (!node)
        return;

    // Si es una declaración de función, procesarla y buscar dentro de su cuerpo
    if (node->type == NODE_FUNC_DEC)
    {
        // codegen_dec_function(v,node);

        make_function_dec(visitor, node);
        // Buscar funciones anidadas dentro del cuerpo de la función
        find_function_dec(visitor, node->data.func_node.body);
        return;
    }

    // Recursivamente buscar en los diferentes tipos de nodos
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
        // Buscar en las declaraciones
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT)
            {
                find_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
            }
        }
        // Buscar en el cuerpo
        find_function_dec(visitor, node->data.func_node.body);
        break;
    }
}

LLVMValueRef make_function_dec(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "vamos a declarar una funcion en el codegen \n");

    const char *name = node->data.func_node.name;
    Type *return_type = node->data.func_node.body->return_type;
    ASTNode **params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;
    LLVMTypeRef llvm_return_type = type_to_llvm(v->ctx, return_type);

    fprintf(stderr, "el return type de %s es %s\n", name, return_type->name);

    // Obtener tipos de parámetros
    LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++)
    {
        param_types[i] = type_to_llvm(v->ctx, params[i]->return_type);
    }

    fprintf(stderr, "vamos a declarar una funcion en el codegen \n");

    // Crear y registrar la firma de la función
    LLVMTypeRef func_type = LLVMFunctionType(
        llvm_return_type,
        param_types,
        param_count,
        0);

    fprintf(stderr, "vamos a declarar una funcion en el codegen \n");

    LLVMValueRef func = LLVMAddFunction(v->ctx->module, name, func_type);
    free(param_types);
    return func;
}

void make_body_function_dec(LLVMVisitor *visitor, ASTNode *node)
{
    fprintf(stderr, "estamos aqui en make body funcion dec\n");

    if (!node)
        return;

    // Si es una declaración de función, generar su cuerpo y procesar funciones anidadas
    if (node->type == NODE_FUNC_DEC)
    {
        codegen_accept(visitor, node);
        // Procesar funciones anidadas en el cuerpo de la función
        make_body_function_dec(visitor, node->data.func_node.body);
        return;
    }

    // Recursivamente procesar los diferentes tipos de nodos
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
        // Procesar declaraciones
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == NODE_ASSIGNMENT)
            {
                make_body_function_dec(visitor, node->data.func_node.args[i]->data.op_node.right);
            }
        }
        // Procesar el cuerpo
        make_body_function_dec(visitor, node->data.func_node.body);
        break;
    }

    fprintf(stderr, "salimos del make body funcion dec\n");
}

#pragma region TYPES

void build_vtable_table(LLVMVisitor *v, ASTNode *node)
{

    const char *type_name = node->data.type_node.name;
    LLVMUserTypeInfo *parent_info = NULL;

    if (strcmp(node->data.type_node.parent_name, ""))
    {
        fprintf(stderr, BLUE "DEBUG: Clase '%s' hereda de '%s'.\n" RESET, type_name, node->data.type_node.parent_name);
        parent_info = find_user_type(v->ctx, node->data.type_node.parent_name);
        if (!parent_info)
        {
            fprintf(stderr, RED "Error: Tipo padre '%s' no encontrado para la clase '%s'.\n" RESET, node->data.type_node.parent_name, type_name);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, BLUE "DEBUG: Clase '%s' no tiene padre (es una clase base).\n" RESET, type_name);
    }

    LLVMUserTypeInfo *type_info = malloc(sizeof(LLVMUserTypeInfo));
    type_info->id = v->ctx->user_types ? (v->ctx->user_types->id + 1) : 0;
    type_info->name = strdup(type_name);
    type_info->parent_info = parent_info;

    // numero de metodos y de asignaciones de mi tipo
    int function_context = 0;
    int assigments_context = 0;

    for (int i = 0; i < node->data.type_node.def_count; i++)
    {
        if (node->data.type_node.definitions[i]->type == NODE_FUNC_DEC)
        {
            function_context++;
        }
        if (node->data.type_node.definitions[i]->type == NODE_ASSIGNMENT)
        {
            assigments_context++;
        }
    }

    int parent_assigments = parent_info ? parent_info->num_data_members : 0;
    int parent_methods = parent_info ? parent_info->num_methods_virtual : 0;

    // Contar la cantidad de metodos sobreescritos de cada uno
    int overriden_methods = 0;

    for (int i = 0; parent_info && i < node->data.type_node.def_count; i++)
    {
        ASTNode *child = node->data.type_node.definitions[i];

        if (child->type == NODE_FUNC_DEC)
        {

            char *base_name = delete_underscore_from_str(child->data.func_node.name, type_name);

            for (int j = 0; j < parent_info->num_methods_virtual; j++)
            {
                char *parent_method_name = delete_underscore_from_str(parent_info->methods[j]->name, parent_info->name);

                if (strcmp(parent_method_name, base_name) == 0)
                {
                    child->data.func_node.flag_overriden = j + 1;
                    overriden_methods++;
                    free(parent_method_name);
                    break;
                }

                free(parent_method_name);
            }

            free(base_name);
        }
    }

    fprintf(stderr, GREEN "el numero de overriden emthods de %s es %d\n" RESET, type_name, overriden_methods);

    char vtable_name[256];
    snprintf(vtable_name, sizeof(vtable_name), "%s_vtable", type_name);

    LLVMTypeRef vtable_struct_type = LLVMStructCreateNamed(v->ctx->context, vtable_name);
    LLVMTypeRef class_struct_type = LLVMStructCreateNamed(v->ctx->context, type_name);

    type_info->struct_type = class_struct_type;
    type_info->vtable_struct_type = vtable_struct_type;
    type_info->num_data_members = assigments_context + parent_assigments;
    type_info->num_methods_virtual = function_context + parent_methods - overriden_methods;

    fprintf(stderr, "Een el tipo %s el  numero de metodos de virtuales es %d y de miembros es %d\n", type_info->name, type_info->num_methods_virtual, type_info->num_data_members);
    type_info->methods = (LLVMMethodInfo **)malloc(type_info->num_methods_virtual * sizeof(LLVMMethodInfo *));
    type_info->members = (LLVMTypeMemberInfo **)malloc(type_info->num_data_members * sizeof(LLVMTypeMemberInfo *));

    type_info->next = v->ctx->user_types;
    v->ctx->user_types = type_info;

    LLVMTypeRef *vtable_slot_ptr_types = (LLVMTypeRef *)malloc(type_info->num_methods_virtual * sizeof(LLVMTypeRef));
    LLVMTypeRef *vtable_slot_types = (LLVMTypeRef *)malloc(type_info->num_methods_virtual * sizeof(LLVMTypeRef));

    fprintf(stderr, "2-OEEEE SIIIII\n");

    for (int i = 0; i < parent_methods; i++)
    {
        vtable_slot_ptr_types[i] = parent_info->vtable_slot_ptr_types[i];
        vtable_slot_types[i] = parent_info->vtable_slot_types[i];
    }

    for (int i = 0, j = parent_methods; i < node->data.type_node.def_count; i++)
    {
        ASTNode *child = node->data.type_node.definitions[i];

        if (child->type == NODE_FUNC_DEC && child->data.func_node.flag_overriden == 0)
        {
            fprintf(stderr, GREEN "El tipo de retonor de mi funcion %s es %s\n" RESET, child->data.func_node.name, child->data.func_node.body->return_type->name);

            LLVMValueRef llvm_ret_type = type_to_llvm(v->ctx, child->data.func_node.body->return_type);

            // 1. Calcular le numero total de parametros LLVM : 'this' + parametros explicito
            int num_explicit_params = child->data.func_node.arg_count;
            int total_llvm_params = 1 + num_explicit_params;

            // 2. Asignar memoria para el array de tipo de paramtros LLVM
            LLVMTypeRef *llvm_param_types = (LLVMTypeRef *)malloc(total_llvm_params * sizeof(LLVMTypeRef));

            if (!llvm_param_types)
            {
                perror("Error en malloc para llvm_param_types");
                exit(EXIT_FAILURE);
            }

            // 3. El primer paramtros siempre es el puntero 'this'
            llvm_param_types[0] = LLVMPointerType(class_struct_type, 0);

            // 4. Agregar los tipos de los apramtros explcitios
            for (int k = 0; k < num_explicit_params; k++)
            {
                llvm_param_types[k+1] = type_to_llvm(v->ctx,child->data.func_node.args[k]->return_type);
            }

            // 5. Crear el tipo de la funcion LLVM con el array de parametros correctos
            LLVMTypeRef method_func_type = LLVMFunctionType(llvm_ret_type,llvm_param_types,total_llvm_params,0);
            
            vtable_slot_types[j] = method_func_type;

            LLVMTypeRef method_func_ptr_type = LLVMPointerType(method_func_type, 0); // Puntero a ese tipo de función

            vtable_slot_ptr_types[j++] = method_func_ptr_type;

            free(llvm_param_types);
        }

        if (child->type == NODE_FUNC_DEC && child->data.func_node.flag_overriden >= 1)
        {
            int index = child->data.func_node.flag_overriden - 1;

            LLVMValueRef llvm_ret_type = type_to_llvm(v->ctx, child->data.func_node.body->return_type);

            // 1. Calcular le numero total de parametros LLVM : 'this' + parametros explicito
            int num_explicit_params = child->data.func_node.arg_count;
            int total_llvm_params = 1 + num_explicit_params;

            // 2. Asignar memoria para el array de tipo de paramtros LLVM
            LLVMTypeRef *llvm_param_types = (LLVMTypeRef *)malloc(total_llvm_params * sizeof(LLVMTypeRef));

            if (!llvm_param_types)
            {
                perror("Error en malloc para llvm_param_types");
                exit(EXIT_FAILURE);
            }

            // 3. El primer paramtros siempre es el puntero 'this'
            llvm_param_types[0] = LLVMPointerType(class_struct_type, 0);

            // 4. Agregar los tipos de los apramtros explcitios
            for (int k = 0; k < num_explicit_params; k++)
            {
                llvm_param_types[k+1] = type_to_llvm(v->ctx,child->data.func_node.args[k]->return_type);
            }

            // 5. Crear el tipo de la funcion LLVM con el array de parametros correctos
            LLVMTypeRef method_func_type = LLVMFunctionType(llvm_ret_type,llvm_param_types,total_llvm_params,0);
            
            vtable_slot_types[index] = method_func_type;

            LLVMTypeRef method_func_ptr_type = LLVMPointerType(method_func_type, 0); // Puntero a ese tipo de función

            vtable_slot_ptr_types[index] = method_func_ptr_type;
        }
    }

    LLVMStructSetBody(vtable_struct_type, vtable_slot_ptr_types, type_info->num_methods_virtual, 0);

    type_info->vtable_slot_ptr_types = vtable_slot_ptr_types;
    type_info->vtable_slot_types = vtable_slot_types;

    fprintf(stderr, "3-OEEEE SIIIII\n");

    LLVMTypeRef vtable_ptr_type = LLVMPointerType(vtable_struct_type, 0);
    type_info->vtable_ptr_type = vtable_ptr_type;

    LLVMTypeRef *struct_fields = build_struct_fields(v->ctx, node, type_info);

    type_info->struct_fields = struct_fields;

    // fprintf(stderr, RED" el numero de assignments es %d\n",assigments_context);
    LLVMStructSetBody(class_struct_type, struct_fields, 2 + type_info->num_data_members, 0); // aqui donde hay un 4 iba un 3

    LLVMTypeRef ptr_type = LLVMPointerType(class_struct_type, 0); // Puntero a %A

#pragma region MET_TYPES

    // ------------------------------------L LA PASION ---------------------------------------------
    // LLVMValueRef *vtable_initializer_values = build_struct_method(v->ctx,node,type_info);

    fprintf(stderr, "3-OEEEE SIIIII\n");

    LLVMValueRef *vtable_initializer_values = build_vtable_initializer(v->ctx, node, type_info);
    type_info->vtable_initializer_values = vtable_initializer_values;

#pragma endregion

    // --- 6. Definición d  e la Instancia Global de Vtable (@A_vtable_instance) ---
    // @A_vtable_instance = internal global %A_vtable { i32 (%A*)* @_A_getX }

    // aqui el segundo 0 debe ser relacionado con los metodos de vtable_initilizer_value
    fprintf(stderr, GREEN "Debug : el error es aqui \n" RESET);

    LLVMValueRef vtable_constant = LLVMConstStruct(vtable_initializer_values, type_info->num_methods_virtual, 0);

    fprintf(stderr, "Aguirre\n");

    char vtable_name_type[256];
    snprintf(vtable_name_type, sizeof(vtable_name_type), "%s_vtable_instance", type_name);

    LLVMValueRef vtable_instance_global = LLVMAddGlobal(v->ctx->module, vtable_struct_type, vtable_name_type);
    LLVMSetInitializer(vtable_instance_global, vtable_constant);
    LLVMSetLinkage(vtable_instance_global, LLVMInternalLinkage);
    LLVMSetUnnamedAddr(vtable_instance_global, 1);

    type_info->vtable_global = vtable_instance_global; // Se asigna en make_body_type_dec
}

void find_type_dec(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTO EN EL FIND_TYPE_DECL\n");

    if (!node)
        return;

    if (node->type == NODE_TYPE_DEC)
    {
        const char *type_name = node->data.type_node.name;
        if (!find_user_type(v->ctx, type_name)) // Si no ha sido declarado aún
        {
            build_vtable_table(v, node);
        }
    }

    if (node->type == NODE_PROGRAM || node->type == NODE_BLOCK)
    {
        for (int i = 0; i < node->data.program_node.count; i++)
        {
            find_type_dec(v, node->data.program_node.statements[i]);
        }
    }

    fprintf(stderr, "ESTOY FINALIZANDO FIND_TYPE_DEC QUE SEA ASI TAMBIEN\n");
}

// LLVMValueRef *build_struct_method(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *type_info)
// {
//     int aux = 1;
//     // contador de metodos virtuales
//     LLVMValueRef *vtable_initializer_values = (LLVMValueRef *)malloc((type_info->num_methods_virtual) * sizeof(LLVMValueRef));

//     for (int i = 0; i < node->data.type_node.def_count; i++)
//     {
//         ASTNode *child = node->data.type_node.definitions[i];

//         if (child->type == NODE_FUNC_DEC)
//         {

//             aux++;

//             const char *func_name = child->data.func_node.name;

//             fprintf(stderr, RED "func_name es %s\n" RESET, func_name);

//             LLVMValueRef getY_func = LLVMAddFunction(ctx->module, func_name, type_info->method_func_type);

//             // codegen_accept(v,child);
//             LLVMBasicBlockRef getY_entry = LLVMAppendBasicBlockInContext(ctx->context, getY_func, "entry");
//             LLVMPositionBuilderAtEnd(ctx->builder, getY_entry);

//             LLVMValueRef param_1 = LLVMGetParam(getY_func, 0); // %0 es el puntero 'this'
//             fprintf(stderr, RED "1-TODO BIEN\n" RESET);

//             const char *arg_name = node->data.type_node.args[aux - 2]->data.variable_name;

//             char y_ptr_str[256];
//             snprintf(y_ptr_str, sizeof(y_ptr_str), "%s_ptr", arg_name);
//             fprintf(stderr, RED "1.5-TODO BIEN %s\n" RESET, arg_name);

//             LLVMValueRef y_ptr = LLVMBuildStructGEP2(ctx->builder, type_info->struct_type, param_1, aux, y_ptr_str);

//             fprintf(stderr, RED "2-TODO BIEN\n" RESET);

//             char y_val_str[256];
//             snprintf(y_val_str, sizeof(y_val_str), "%s_val", arg_name);

//             LLVMValueRef y_val = LLVMBuildLoad2(ctx->builder, ctx->i32_type, y_ptr, y_val_str);
//             fprintf(stderr, RED "3-TODO BIEN\n" RESET);

//             LLVMSetAlignment(y_val, 4);
//             fprintf(stderr, RED "4-TODO BIEN\n" RESET);

//             LLVMBuildRet(ctx->builder, y_val);
//             fprintf(stderr, RED "5-TODO BIEN\n" RESET);

//             fprintf(stderr, GREEN "aux - 2 es %d\n" RESET, aux - 2);

//             vtable_initializer_values[aux - 2] = getY_func;

//             LLVMMethodInfo *new_method = malloc(sizeof(LLVMMethodInfo));
//             new_method->name = func_name;
//             new_method->vtable_index = aux - 2;
//             fprintf(stderr, "El tipo de retorno de mi funcion es %s\n", child->return_type->name);

//             new_method->llvm_func_type = type_to_llvm(ctx, child->return_type);
//             new_method->llvm_func_value = getY_func;
//             new_method->next = type_info->methods;

//             type_info->methods = new_method;
//         }
//     }

//     return vtable_initializer_values;
// }

// // FASE 1: make_body_type_dec (Construye el cuerpo de los tipos y vtables, declara funciones de métodos)
// // Esta es tu función `make_type_dec` refactorizada.
// void make_body_type_dec(LLVMVisitor *v, ASTNode *node)
// {
//     fprintf(stderr, "ESTOY EN EL MAKE BODY TYPE DEC\n");
//     if (!node || node->type != NODE_TYPE_DEC)
//         return;

//     const char *type_name = node->data.type_node.name;
//     LLVMUserTypeInfo *type_info = find_user_type(v->ctx, type_name);

//     if (!type_info)
//     {
//         fprintf(stderr, RED "ERROR: Tipo '%s' no encontrado en el contexto para make_body_type_dec.\n" RESET, type_name);
//         exit(1);
//     }

//     // --- 1. Determinar los miembros de datos de la clase y definir el cuerpo de la estructura ---
//     int data_field_count = 0;
//     LLVMTypeMemberInfo *last_member_info = NULL;
//     int current_llvm_field_index = 2; // Campo 0: ID (i32), Campo 1: VtablePtr (%A_vtable*)

//     for (int i = 0; i < node->data.type_node.def_count; i++)
//     {
//         ASTNode *body_element = node->data.type_node.definitions[i];
//         if (body_element->type == NODE_ASSIGNMENT) // Esto es un miembro de datos con inicialización
//         {

//             const char *member_name = body_element->data.op_node.left->data.variable_name; // CORREGIDO: Acceso a `assignment`

//             LLVMTypeRef member_llvm_type;
//             if (body_element->data.op_node.right && body_element->data.op_node.right->return_type)
//             {
//                 member_llvm_type = type_to_llvm(v->ctx, body_element->data.op_node.right->return_type); // CORREGIDO: Llamada a type_to_llvm
//             }
//             else
//             {
//                 fprintf(stderr, YELLOW "WARNING: Tipo de miembro '%s' no inferido del AST, asumiendo double. (Línea: %d)\n" RESET, member_name, body_element->line);
//                 member_llvm_type = v->ctx->double_type; // Fallback
//             }

//             LLVMTypeMemberInfo *new_member_info = (LLVMTypeMemberInfo *)malloc(sizeof(LLVMTypeMemberInfo));
//             if (!new_member_info)
//             {
//                 perror(RED "Error en malloc para LLVMTypeMemberInfo" RESET);
//                 exit(EXIT_FAILURE);
//             }
//             new_member_info->name = strdup(member_name);
//             new_member_info->type = member_llvm_type;
//             new_member_info->index = current_llvm_field_index;
//             new_member_info->next = NULL;

//             if (type_info->members == NULL)
//             {
//                 type_info->members = new_member_info;
//             }
//             else
//             {
//                 last_member_info->next = new_member_info;
//             }
//             last_member_info = new_member_info;

//             data_field_count++;
//             current_llvm_field_index++;
//         }
//     }
//     type_info->num_data_members = data_field_count; // Almacena el número de miembros de datos

//     // Construir el array de LLVMTypeRef para los campos de la estructura de la clase
//     int total_class_struct_fields = 2 + data_field_count;
//     LLVMTypeRef *class_struct_field_types = (LLVMTypeRef *)malloc(total_class_struct_fields * sizeof(LLVMTypeRef));
//     if (!class_struct_field_types)
//     {
//         perror(RED "Error en malloc para class_struct_field_types" RESET);
//         exit(EXIT_FAILURE);
//     }

//     class_struct_field_types[0] = v->ctx->i32_type;           // Campo 0: ID del tipo
//     class_struct_field_types[1] = type_info->vtable_ptr_type; // Campo 1: Puntero a vtable

//     // Llenar los campos de datos usando la información recopilada en type_info->members
//     LLVMTypeMemberInfo *current_mem_info_for_body = type_info->members;
//     int field_array_idx_for_body = 2;
//     while (current_mem_info_for_body != NULL)
//     {
//         class_struct_field_types[field_array_idx_for_body++] = current_mem_info_for_body->type;
//         current_mem_info_for_body = current_mem_info_for_body->next;
//     }

//     LLVMStructSetBody(type_info->struct_type, class_struct_field_types, total_class_struct_fields, 0);

//     fprintf(stderr, YELLOW "DEBUG: EL NUMERO DE CAMPOS ES %d\n" RESET, total_class_struct_fields);

//     free(class_struct_field_types);
//     fprintf(stderr, "DEBUG: Cuerpo de la clase '%s' definido con %d campos (ID + VtablePtr + %d miembros de datos).\n", type_name, total_class_struct_fields, data_field_count);

//     // --- 2. Determinar los métodos virtuales y Declarar Funciones LLVM ---
//     ASTNode **method_decl_nodes_temp = NULL; // Almacenará punteros a los nodos NODE_FUNC_DEC
//     int current_num_methods = 0;

//     for (int i = 0; i < node->data.type_node.def_count; i++)
//     {
//         if (node->data.type_node.definitions[i]->type == NODE_FUNC_DEC)
//         {
//             method_decl_nodes_temp = (ASTNode **)realloc(method_decl_nodes_temp, (current_num_methods + 1) * sizeof(ASTNode *));
//             if (!method_decl_nodes_temp)
//             {
//                 perror(RED "Error en realloc para method_decl_nodes_temp" RESET);
//                 exit(EXIT_FAILURE);
//             }
//             method_decl_nodes_temp[current_num_methods] = node->data.type_node.definitions[i];
//             current_num_methods++;
//         }
//     }
//     type_info->num_methods_virtual = current_num_methods; // Almacena el número de métodos virtuales

//     // Asignar memoria para almacenar las referencias a las funciones LLVM y sus nodos AST
//     type_info->method_llvm_funcs = (LLVMValueRef *)malloc(current_num_methods * sizeof(LLVMValueRef));
//     if (!type_info->method_llvm_funcs)
//     {
//         perror(RED "Error en malloc para method_llvm_funcs" RESET);
//         exit(EXIT_FAILURE);
//     }
//     type_info->method_decl_nodes_ast = (ASTNode **)malloc(current_num_methods * sizeof(ASTNode *));
//     if (!type_info->method_decl_nodes_ast)
//     {
//         perror(RED "Error en malloc para method_decl_nodes_ast" RESET);
//         exit(EXIT_FAILURE);
//     }

//     LLVMTypeRef *vtable_member_types = (LLVMTypeRef *)malloc(current_num_methods * sizeof(LLVMTypeRef));
//     if (!vtable_member_types)
//     {
//         perror(RED "Error en malloc para vtable_member_types" RESET);
//         exit(EXIT_FAILURE);
//     }

//     for (int i = 0; i < current_num_methods; i++)
//     {
//         ASTNode *method_node = method_decl_nodes_temp[i];

//         type_info->method_decl_nodes_ast[i] = method_node; // Guarda el nodo AST del método

//         // Obtener el tipo de la firma de la función (ej. double (ptr), void (ptr, double, double), etc.)
//         LLVMTypeRef method_signature_type = get_llvm_method_func_type(v->ctx, method_node); // CORREGIDO: get_llvm_method_func_type declarada arriba
//         LLVMTypeRef method_signature_ptr_type = LLVMPointerType(method_signature_type, 0);  // Puntero a esa firma

//         vtable_member_types[i] = method_signature_ptr_type; // Tipo del slot en la vtable

//         char llvm_func_name[256];
//         snprintf(llvm_func_name, sizeof(llvm_func_name), "_%s_%s", type_name, method_node->data.func_node.name);

//         LLVMValueRef llvm_func = LLVMAddFunction(v->ctx->module, llvm_func_name, method_signature_type);
//         type_info->method_llvm_funcs[i] = llvm_func; // Guarda la referencia a la función LLVM
//     }

//     // Establecer el cuerpo de la vtable (ej. %A_vtable = type { double (%A*)*, void (%A*)*, ... })
//     LLVMStructSetBody(type_info->vtable_struct_type, vtable_member_types, current_num_methods, 0);
//     free(vtable_member_types);
//     fprintf(stderr, "DEBUG: Cuerpo de la vtable para '%s' definido con %d métodos.\n", type_name, current_num_methods);

//     // --- 3. Crear y Inicializar la Instancia Global de la Vtable (@A_vtable_instance) ---
//     LLVMValueRef *vtable_initializer_values = (LLVMValueRef *)malloc(current_num_methods * sizeof(LLVMValueRef));
//     if (!vtable_initializer_values)
//     {
//         perror(RED "Error en malloc para vtable_initializer_values" RESET);
//         exit(EXIT_FAILURE);
//     }

//     for (int i = 0; i < current_num_methods; i++)
//     {
//         vtable_initializer_values[i] = type_info->method_llvm_funcs[i]; // Los LLVMValueRef de las funciones declaradas
//     }

//     char vtable_global_name[256];
//     snprintf(vtable_global_name, sizeof(vtable_global_name), "%s_vtable_instance", type_name);

//     LLVMValueRef vtable_constant = LLVMConstStruct(vtable_initializer_values, current_num_methods, 0);
//     type_info->vtable_global = LLVMAddGlobal(v->ctx->module, type_info->vtable_struct_type, vtable_global_name);
//     LLVMSetInitializer(type_info->vtable_global, vtable_constant);
//     LLVMSetLinkage(type_info->vtable_global, LLVMInternalLinkage);
//     LLVMSetUnnamedAddr(type_info->vtable_global, 1);
//     fprintf(stderr, "DEBUG: Instancia global de vtable '@%s' creada.\n", vtable_global_name);

//     free(vtable_initializer_values);
//     free(method_decl_nodes_temp); // Liberar el array temporal de nodos de método

//     // La recursión para AST_PROGRAM/AST_BLOCK es para procesar múltiples TypeDefs en un programa.
//     if (node->type == NODE_PROGRAM || node->type == NODE_BLOCK)
//     {
//         for (int i = 0; i < node->data.program_node.count; i++)
//         {

//             make_body_type_dec(v, node->data.program_node.statements[i]);
//         }
//     }

//     fprintf(stderr, "Sali de make boddy type dec\n");
// }

// // FASE 2: `get_llvm_method_func_type` (Corregida para usar type_to_llvm)
// LLVMTypeRef get_llvm_method_func_type(LLVMCoreContext *ctx, ASTNode *func_decl_node)
// {
//     if (!func_decl_node || func_decl_node->type != NODE_FUNC_DEC)
//     {
//         fprintf(stderr, RED "ERROR: Se esperaba un nodo NODE_FUNC_DEC para get_llvm_method_func_type.\n" RESET);
//         exit(EXIT_FAILURE);
//     }

//     // Obtener el tipo de retorno de la función (del return_type del cuerpo)
//     // Asumimos que el cuerpo tiene un return_type válido que representa el tipo de retorno.
//     // Si el cuerpo es nulo (función abstracta/externa) o no tiene tipo, por defecto a void.
//     // Pasa ctx directamente a type_to_llvm
//     Type *return_type_value = func_decl_node->data.func_node.body && func_decl_node->data.func_node.body->return_type
//                                   ? func_decl_node->data.func_node.body->return_type
//                                   : &TYPE_VOID;                          // Fallback
//     LLVMTypeRef return_llvm_type = type_to_llvm(ctx, return_type_value); // CORREGIDO: Pasa ctx directamente.

//     // El primer parámetro siempre es el puntero 'this'
//     int param_count_after_this = func_decl_node->data.func_node.arg_count;

//     LLVMTypeRef *param_types = (LLVMTypeRef *)malloc((1 + param_count_after_this) * sizeof(LLVMTypeRef));
//     if (!param_types)
//     {
//         perror(RED "Error en malloc para param_types (get_llvm_method_func_type)" RESET);
//         exit(EXIT_FAILURE);
//     }

//     param_types[0] = ctx->ptr_type; // El puntero 'this' (i8*)

//     // Llenar los tipos de los parámetros adicionales usando type_to_llvm y return_type
//     for (int i = 0; i < param_count_after_this; i++)
//     {
//         ASTNode *param_node = func_decl_node->data.func_node.args[i];
//         if (param_node && param_node->return_type)
//         {
//             param_types[1 + i] = type_to_llvm(ctx, param_node->return_type); // CORREGIDO: Pasa ctx directamente.
//         }
//         else
//         {
//             fprintf(stderr, YELLOW "WARNING: Parámetro de método sin tipo inferido, asumiendo i8* (Línea: %d).\n" RESET, func_decl_node->line);
//             param_types[1 + i] = ctx->ptr_type; // Fallback
//         }
//     }

//     LLVMTypeRef func_type = LLVMFunctionType(return_llvm_type, param_types, 1 + param_count_after_this, 0);
//     free(param_types);
//     return func_type;
// }

// // FASE 3: codegen_method_body (Genera el cuerpo de UN método)
// void codegen_method_body(LLVMVisitor *v, LLVMUserTypeInfo *type_info, ASTNode *method_decl_node, LLVMValueRef llvm_func)
// {

//     fprintf(stderr, "CODEGEN METHOS BODY\n");

//     if (!method_decl_node || method_decl_node->type != NODE_FUNC_DEC)
//     {
//         fprintf(stderr, RED "ERROR: Expected NODE_FUNC_DEC node for codegen_method_body.\n" RESET);
//         return;
//     }

//     const char *method_name = method_decl_node->data.func_node.name;
//     LLVMBasicBlockRef entry_block = LLVMAppendBasicBlockInContext(v->ctx->context, llvm_func, "entry");
//     LLVMPositionBuilderAtEnd(v->ctx->builder, entry_block);

//     // Obtener y castrear el puntero 'this'
//     LLVMValueRef self_ptr = LLVMGetParam(llvm_func, 0);
//     LLVMValueRef specific_self_ptr = LLVMBuildBitCast(v->ctx->builder, self_ptr, LLVMPointerType(type_info->struct_type, 0), "specific_self_ptr");

//     push_scope(); // Nuevo ámbito para el método

//     // Declarar 'this' ('self') en el ámbito
//     declare_variable("self", specific_self_ptr); // O 'this' dependiendo de tu convención

//     fprintf(stderr, "1-CODEGEN DEBUG\n");

//     // Declarar parámetros del método en el ámbito
//     for (int j = 0; j < method_decl_node->data.func_node.arg_count; j++)
//     {
//         LLVMValueRef param_val = LLVMGetParam(llvm_func, j + 1); // Parámetros de método LLVM (después de 'this')
//         ASTNode *param_ast_node = method_decl_node->data.func_node.args[j];
//         if (param_ast_node && param_ast_node->data.variable_name)
//         {
//             declare_variable(param_ast_node->data.variable_name, param_val);
//         }
//         else
//         {
//             fprintf(stderr, YELLOW "WARNING: Parámetro de método sin nombre en AST (Línea: %d).\n" RESET, method_decl_node->line);
//         }
//     }

//     fprintf(stderr, "2-CODEGEN DEBUG\n");

//     // --- GENERAR EL CUERPO REAL DEL MÉTODO ---
//     LLVMValueRef body_result = codegen_accept(v, method_decl_node->data.func_node.body); // CORREGIDO: body_result ahora se usa

//     fprintf(stderr, "4-CODEGEN DEBUG\n");

//     // Asegurar que el bloque termine con una instrucción de retorno
//     LLVMBasicBlockRef current_block = LLVMGetInsertBlock(v->ctx->builder);
//     if (!LLVMGetBasicBlockTerminator(current_block))
//     {
//         fprintf(stderr, "3-CODEGEN DEBUG\n");

//         // CORREGIDO: Uso de LLVMTypeOf y LLVMGetElementType
//         LLVMTypeRef return_type = LLVMGetReturnType(LLVMGetElementType(LLVMTypeOf(llvm_func)));
//         if (LLVMGetTypeKind(return_type) == LLVMVoidTypeKind)
//         {
//             LLVMBuildRetVoid(v->ctx->builder);
//         }
//         else
//         {
//             fprintf(stderr, YELLOW "WARNING: Método '%s' termina sin instrucción de retorno explícita. Insertando retorno predeterminado (Línea: %d).\n" RESET, method_name, method_decl_node->line);
//             // Insertar un valor de retorno predeterminado basado en el tipo
//             if (LLVMGetTypeKind(return_type) == LLVMDoubleTypeKind)
//             {
//                 LLVMBuildRet(v->ctx->builder, body_result); // Usa body_result si es el tipo correcto
//             }
//             else if (LLVMGetTypeKind(return_type) == LLVMIntegerTypeKind)
//             {                                               // i1, i32, i64
//                 LLVMBuildRet(v->ctx->builder, body_result); // Usa body_result si es el tipo correcto
//             }
//             else if (LLVMGetTypeKind(return_type) == LLVMPointerTypeKind)
//             {
//                 LLVMBuildRet(v->ctx->builder, body_result); // Usa body_result si es el tipo correcto
//             }
//             else
//             {
//                 LLVMBuildUnreachable(v->ctx->builder); // Fallback extremo
//             }
//         }
//     }

//     pop_scope(); // Salir del ámbito del método
//     fprintf(stderr, "DEBUG: Cuerpo de método '%s' generado dinámicamente.\n", method_name);
// }

#pragma endregion