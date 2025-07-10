
// init_codegen.c

#include "init_codegen.h"
#include <stdlib.h>
#include <string.h>

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

#define I1_TYPE LLVMInt1Type()
#define I8_TYPE LLVMInt8Type()
#define I32_TYPE LLVMInt32Type()
#define I64_TYPE LLVMInt64Type()
#define DOUBLE_TYPE LLVMDoubleType()
#define VOID_TYPE LLVMVoidType()

// Punteros
#define I8_PTR_TYPE LLVMPointerType(I8_TYPE, 0)
#define I32_PTR_TYPE LLVMPointerType(I32_TYPE, 0)

// Funciones comunes
#define FUNC_TYPE(ret, params, count, vararg) \
    LLVMFunctionType(ret, params, count, vararg)

// Declara una función simple
#define DECLARE_FUNC(module, name, ret, ...)                                             \
    do                                                                                   \
    {                                                                                    \
        LLVMTypeRef param_types[] = {__VA_ARGS__};                                       \
        LLVMTypeRef func_type = FUNC_TYPE(ret, param_types,                              \
                                          sizeof(param_types) / sizeof(LLVMTypeRef), 0); \
        LLVMAddFunction(module, name, func_type);                                        \
    } while (0)

// Declara función variádica (como printf)
#define DECLARE_VARIADIC_FUNC(module, name, ret, ...)                                    \
    do                                                                                   \
    {                                                                                    \
        LLVMTypeRef param_types[] = {__VA_ARGS__};                                       \
        LLVMTypeRef func_type = FUNC_TYPE(ret, param_types,                              \
                                          sizeof(param_types) / sizeof(LLVMTypeRef), 1); \
        LLVMValueRef func = LLVMAddFunction(module, name, func_type);                    \
        LLVMSetLinkage(func, LLVMExternalLinkage);                                       \
    } while (0)

LLVMValueRef createGlobalString(LLVMCoreContext *ctx, const char *str, const char *name)
{
    size_t len = strlen(str);
    LLVMValueRef string_const = LLVMConstStringInContext(ctx->context, str, len, 0);
    LLVMTypeRef string_type = LLVMArrayType(ctx->i8_type, len + 1);
    LLVMValueRef global_var = LLVMAddGlobal(ctx->module, string_type, name);
    LLVMSetInitializer(global_var, string_const);
    LLVMSetLinkage(global_var, LLVMPrivateLinkage);
    LLVMSetUnnamedAddr(global_var, 1);
    return global_var;
}

int get_id_type(LLVMCoreContext *ctx, const char *name)
{
    LLVMUserTypeInfo *curr = ctx->user_types;

    while (curr)
    {
        if (strcmp(curr->name, name) == 0)
        {
            return curr->id;
        }
    }

    return -1;
}

LLVMUserTypeInfo *find_user_type(LLVMCoreContext *ctx, const char *name)
{
    fprintf(stderr, "VOY A BUSCAR EN EL FIND_USER_TYPE el nombre de varaible %s\n", name);
    LLVMUserTypeInfo *current = ctx->user_types;

    while (current)
    {
        fprintf(stderr, YELLOW "El current name es %s\n" RESET, current->name);

        if (strcmp(current->name, name) == 0)
        {
            fprintf(stderr, YELLOW "El current name  a devolver es %s\n" RESET, current->name);

            return current;
        }
        current = current->next;
    }
    return NULL;
}

int find_field_index(LLVMUserTypeInfo *type_info, const char *param_name)
{

    for (int i = 0; i < type_info->num_data_members; i++)
    {
        if (strcmp(type_info->members[i]->name, param_name) == 0)
        {
            return type_info->members[i]->index;
        }
    }

    return -1;
}

LLVMTypeMemberInfo *find_member_info(LLVMUserTypeInfo *type_info, const char *param_name)
{

    for (int i = 0; i < type_info->num_data_members; i++)
    {
        if (strcmp(type_info->members[i]->name, param_name) == 0)
        {
            return type_info->members[i];
        }
    }

    return NULL;
}

LLVMMethodInfo *find_method_info(LLVMUserTypeInfo *type_info, const char *name)
{

    for (int i = 0; i < type_info->num_methods_virtual; i++)
    {
        if (strcmp(type_info->methods[i]->name, name) == 0)
        {
            return type_info->methods[i];
        }
    }

    return NULL;
}

LLVMCoreContext *llvm_core_context_create()
{
    LLVMCoreContext *ctx = malloc(sizeof(LLVMCoreContext));

    if (!ctx)
        return NULL;

    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    // LLVMInitializeNativeTarget();
    // LLVMInitializeNativeAsmPrinter();
    // LLVMInitializeNativeAsmParser();
    // LLVMInitializeAllTargetInfos();
    // LLVMInitializeAllTargets();
    // LLVMInitializeAllTargetMCs();
    // LLVMInitializeAllAsmPrinters();
    // LLVMInitializeAllAsmParsers();

    ctx->context = LLVMGetGlobalContext();
    ctx->module = LLVMModuleCreateWithName("MyModule");
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);

    LLVMSetSourceFileName(ctx->module, "MyModule", strlen("MyModule")); // Set source filename

    // Inicializar la variable global de profundidad de stack
    ctx->max_stack_depth = 100000;
    ctx->current_stack_depth_var = LLVMAddGlobal(
        ctx->module,
        LLVMInt32Type(),
        "current_stack_depth");
    LLVMSetInitializer(ctx->current_stack_depth_var, LLVMConstInt(LLVMInt32Type(), 0, 0));
    LLVMSetLinkage(ctx->current_stack_depth_var, LLVMPrivateLinkage);

    // Initialize basic LLVM types (using context-specific types)
    ctx->i1_type = LLVMInt1TypeInContext(ctx->context);
    ctx->i8_type = LLVMInt8TypeInContext(ctx->context);
    ctx->i32_type = LLVMInt32TypeInContext(ctx->context);
    ctx->i64_type = LLVMInt64TypeInContext(ctx->context);
    ctx->double_type = LLVMDoubleTypeInContext(ctx->context);
    ctx->void_type = LLVMVoidTypeInContext(ctx->context);
    ctx->ptr_type = LLVMPointerType(ctx->i8_type, 0); // General purpose i8* pointer

    ctx->user_types = NULL;

    return ctx;
}

void free_llvm_core_context(LLVMCoreContext *ctx)
{
    if (!ctx)
        return;

    LLVMUserTypeInfo *current_type = ctx->user_types;
    while (current_type != NULL)
    {
        LLVMUserTypeInfo *next_type = current_type->next;
        free(current_type->name);
        if (current_type->num_methods_virtual)
        {
            free_llvm_method(current_type->methods);
        }
        if (current_type->num_data_members)
        {
            free_llvm_type_member(current_type->members);
        }
        free(current_type);

        current_type = next_type;
    }

    // if(ctx->user_types)free_llvm_user_type(ctx->user_types);

    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    free(ctx);
}

void free_llvm_type_member(LLVMTypeMemberInfo **member)
{
    if (!member)
        return;

    for (int i=0;i< sizeof(member)/sizeof(member[0]);i++)
    {
        free(member[i]);
    }

    free(member);
    
}

void free_llvm_method(LLVMMethodInfo **method)
{
    if (!method)
        return;

    for (int i = 0; i < sizeof(method) / sizeof(method[0]); i++)
    {
        free(method[i]->name);
    }

    free(method);
}

void llvm_declare_builtins(LLVMCoreContext *ctx)
{
#define DECLARE_FUNC_INTERNAL(module_ref, name, ret_type, num_params, ...)              \
    do                                                                                  \
    {                                                                                   \
        LLVMTypeRef param_types[] = {__VA_ARGS__};                                      \
        LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, num_params, 0); \
        LLVMValueRef func = LLVMAddFunction(module_ref, name, func_type);               \
        /* Store function and its type in ctx */                                        \
        if (strcmp(name, "strcpy") == 0)                                                \
        {                                                                               \
            ctx->strcpy_func = func;                                                    \
            ctx->strcpy_func_type = func_type;                                          \
        }                                                                               \
        else if (strcmp(name, "strcat") == 0)                                           \
        {                                                                               \
            ctx->strcat_func = func;                                                    \
            ctx->strcat_func_type = func_type;                                          \
        }                                                                               \
        else if (strcmp(name, "strlen") == 0)                                           \
        {                                                                               \
            ctx->strlen_func = func;                                                    \
            ctx->strlen_func_type = func_type;                                          \
        }                                                                               \
        else if (strcmp(name, "sqrt") == 0)                                             \
        {                                                                               \
            ctx->sqrt_func = func;                                                      \
            ctx->sqrt_func_type = func_type;                                            \
        }                                                                               \
        else if (strcmp(name, "sin") == 0)                                              \
        {                                                                               \
            ctx->sin_func = func;                                                       \
            ctx->sin_func_type = func_type;                                             \
        }                                                                               \
        else if (strcmp(name, "cos") == 0)                                              \
        {                                                                               \
            ctx->cos_func = func;                                                       \
            ctx->cos_func_type = func_type;                                             \
        }                                                                               \
        else if (strcmp(name, "log") == 0)                                              \
        {                                                                               \
            ctx->log_func = func;                                                       \
            ctx->log_func_type = func_type;                                             \
        }                                                                               \
        else if (strcmp(name, "exp") == 0)                                              \
        {                                                                               \
            ctx->exp_func = func;                                                       \
            ctx->exp_func_type = func_type;                                             \
        }                                                                               \
        else if (strcmp(name, "rand") == 0)                                             \
        {                                                                               \
            ctx->rand_func = func;                                                      \
            ctx->rand_func_type = func_type;                                            \
        }                                                                               \
        else if (strcmp(name, "pow") == 0)                                              \
        {                                                                               \
            ctx->pow_func = func;                                                       \
            ctx->pow_func_type = func_type;                                             \
        }                                                                               \
        else if (strcmp(name, "fmod") == 0)                                             \
        {                                                                               \
            ctx->fmod_func = func;                                                      \
            ctx->fmod_func_type = func_type;                                            \
        }                                                                               \
        else if (strcmp(name, "malloc") == 0)                                           \
        {                                                                               \
            ctx->malloc_func = func;                                                    \
            ctx->malloc_func_type = func_type;                                          \
        }                                                                               \
        else if (strcmp(name, "puts") == 0)                                             \
        {                                                                               \
            ctx->puts_func = func;                                                      \
            ctx->puts_func_type = func_type;                                            \
        }                                                                               \
        else if (strcmp(name, "strcmp") == 0)                                           \
        {                                                                               \
            ctx->strcmp_func = func;                                                    \
            ctx->strcmp_func_type = func_type;                                          \
        }                                                                               \
        LLVMSetLinkage(func, LLVMExternalLinkage);                                      \
    } while (0)

#define DECLARE_VARIADIC_FUNC_INTERNAL(module_ref, name, ret_type, num_params, ...)     \
    do                                                                                  \
    {                                                                                   \
        LLVMTypeRef param_types[] = {__VA_ARGS__};                                      \
        LLVMTypeRef func_type = LLVMFunctionType(ret_type, param_types, num_params, 1); \
        LLVMValueRef func = LLVMAddFunction(module_ref, name, func_type);               \
        /* Store function and its type in ctx */                                        \
        if (strcmp(name, "printf") == 0)                                                \
        {                                                                               \
            ctx->printf_func = func;                                                    \
            ctx->printf_func_type = func_type;                                          \
        }                                                                               \
        else if (strcmp(name, "snprintf") == 0)                                         \
        {                                                                               \
            ctx->snprintf_func = func;                                                  \
            ctx->snprintf_func_type = func_type;                                        \
        }                                                                               \
        LLVMSetLinkage(func, LLVMExternalLinkage);                                      \
    } while (0)

    // Using LLVMCoreContext's types for consistency
    // Funciones de strings
    DECLARE_FUNC_INTERNAL(ctx->module, "strcpy", ctx->void_type, 2, ctx->ptr_type, ctx->ptr_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "strcat", ctx->void_type, 2, ctx->ptr_type, ctx->ptr_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "strlen", ctx->i64_type, 1, ctx->ptr_type);

    // Funciones matemáticas
    DECLARE_FUNC_INTERNAL(ctx->module, "sqrt", ctx->double_type, 1, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "sin", ctx->double_type, 1, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "cos", ctx->double_type, 1, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "log", ctx->double_type, 1, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "exp", ctx->double_type, 1, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "rand", ctx->i32_type, 0); // No parameters
    DECLARE_FUNC_INTERNAL(ctx->module, "pow", ctx->double_type, 2, ctx->double_type, ctx->double_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "fmod", ctx->double_type, 2, ctx->double_type, ctx->double_type);
    // Memoria
    DECLARE_FUNC_INTERNAL(ctx->module, "malloc", ctx->ptr_type, 1, ctx->i64_type);

    // I/O (variádicas)
    DECLARE_VARIADIC_FUNC_INTERNAL(ctx->module, "printf", ctx->i32_type, 1, ctx->ptr_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "puts", ctx->i32_type, 1, ctx->ptr_type);
    DECLARE_VARIADIC_FUNC_INTERNAL(ctx->module, "snprintf", ctx->i32_type, 3, ctx->ptr_type, ctx->i64_type, ctx->ptr_type);
    DECLARE_FUNC_INTERNAL(ctx->module, "strcmp", ctx->i32_type, 2, ctx->ptr_type, ctx->ptr_type);

    // Control
    // Note: exit takes an i32 and returns void
    LLVMTypeRef exit_param_types[] = {ctx->i32_type};
    ctx->exit_func_type = LLVMFunctionType(ctx->void_type, exit_param_types, 1, 0);
    ctx->exit_func = LLVMAddFunction(ctx->module, "exit", ctx->exit_func_type);
    LLVMSetLinkage(ctx->exit_func, LLVMExternalLinkage);

// Clean up temporary macros
#undef DECLARE_FUNC_INTERNAL
#undef DECLARE_VARIADIC_FUNC_INTERNAL

    // // Funciones de strings
    // DECLARE_FUNC(ctx->module, "strcpy", VOID_TYPE, I8_PTR_TYPE, I8_PTR_TYPE);
    // DECLARE_FUNC(ctx->module, "strcat", VOID_TYPE, I8_PTR_TYPE, I8_PTR_TYPE);
    // DECLARE_FUNC(ctx->module, "strlen", I64_TYPE, I8_PTR_TYPE);

    // // Funciones matemáticas
    // DECLARE_FUNC(ctx->module, "sqrt", DOUBLE_TYPE, DOUBLE_TYPE);
    // DECLARE_FUNC(ctx->module, "sin", DOUBLE_TYPE, DOUBLE_TYPE);
    // DECLARE_FUNC(ctx->module, "cos", DOUBLE_TYPE, DOUBLE_TYPE);

    // DECLARE_FUNC(ctx->module,"log", DOUBLE_TYPE,DOUBLE_TYPE);
    // DECLARE_FUNC(ctx->module,"exp",DOUBLE_TYPE,DOUBLE_TYPE);
    // DECLARE_FUNC(ctx->module,"rand",I32_TYPE);
    // DECLARE_FUNC(ctx->module,"pow",DOUBLE_TYPE,DOUBLE_TYPE,DOUBLE_TYPE);
    // DECLARE_FUNC(ctx->module,"fmod",DOUBLE_TYPE,DOUBLE_TYPE,DOUBLE_TYPE);
    // // Memoria
    // DECLARE_FUNC(ctx->module, "malloc", I8_PTR_TYPE, I64_TYPE);

    // // I/O (variádicas)
    // DECLARE_VARIADIC_FUNC(ctx->module, "printf", I32_TYPE, I8_PTR_TYPE);
    // DECLARE_FUNC(ctx->module, "puts", I32_TYPE, I8_PTR_TYPE);
    // DECLARE_VARIADIC_FUNC(ctx->module,"snprintf",I32_TYPE,I8_PTR_TYPE,I64_TYPE,I8_PTR_TYPE);
    // DECLARE_FUNC(ctx->module,"strcmp",I32_TYPE,I8_PTR_TYPE,I8_PTR_TYPE);

    // // Control
    // DECLARE_FUNC(ctx->module, "exit", VOID_TYPE, I32_TYPE);
}

void llvm_handle_stack_overflow(
    LLVMCoreContext *ctx,
    ASTNode *node)
{
    // Construir mensaje de error con nombre de función y línea
    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg),
             RED "!!RUNTIME ERROR: Stack overflow detected in function '%s'. Line: %d.\n" RESET,
             node->data.func_node.name,
             node->line);

    LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(ctx->builder, error_msg, "error_msg");

    // Llamar a puts para imprimir el mensaje
    LLVMValueRef puts_func = LLVMGetNamedFunction(ctx->module, "puts");
    // LLVMBuildCall(builder, puts_func, &error_msg_global, 1, "");

    LLVMTypeRef puts_type = LLVMFunctionType(LLVMInt32Type(),
                                             (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)},
                                             1, 0);
    LLVMBuildCall2(ctx->builder, puts_type, puts_func, &error_msg_global, 1, "");

    // Llamar a exit(1)
    LLVMValueRef exit_func = LLVMGetNamedFunction(ctx->module, "exit");
    LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 0, 0);
    // LLVMBuildCall(builder, exit_func, &exit_code, 1, "");

    LLVMTypeRef exit_type = LLVMFunctionType(LLVMVoidType(),
                                             (LLVMTypeRef[]){LLVMInt32Type()},
                                             1, 0);
    LLVMBuildCall2(ctx->builder, exit_type, exit_func, &exit_code, 1, "");
    // Marcar como unreachable
    LLVMBuildUnreachable(ctx->builder);
}

LLVMTypeRef type_to_llvm(LLVMCoreContext *ctx, Type *type) // <--- FIRMA CAMBIADA
{
    if (!type)
    {
        fprintf(stderr, RED "Error: Tipo nulo en type_to_llvm.\n" RESET);
        exit(1);
    }

    if (type_equals(type, &TYPE_NUMBER))
    {
        fprintf(stderr, "devolucion es un ctx->double_type\n");

        return ctx->double_type; // HULK Number a LLVM double
    }
    else if (type_equals(type, &TYPE_STRING))
    {
        return ctx->ptr_type; // HULK String a LLVM i8* (puntero a char)
    }
    else if (type_equals(type, &TYPE_BOOLEAN))
    {
        return ctx->i1_type; // HULK Boolean a LLVM i1 (Asegúrate que i1_type esté en LLVMCoreContext)
    }
    else if (type_equals(type, &TYPE_VOID))
    {
        return ctx->void_type; // HULK Void a LLVM void
    }
    else if (type_equals(type, &TYPE_OBJECT)) // Tipo base para objetos, puntero genérico
    {
        return ctx->ptr_type; // HULK Object a LLVM i8* (puntero genérico)
    }
    else if (type->dec != NULL) // Si es un tipo de clase definido por el usuario
    {
        LLVMUserTypeInfo *user_type_info = find_user_type(ctx, type->name); // Usa ctx directamente
        if (!user_type_info)
        {
            fprintf(stderr, RED "Error: Tipo de usuario '%s' no encontrado en el contexto de LLVM para type_to_llvm.\n" RESET, type->name);
            exit(1);
        }
        return LLVMPointerType(user_type_info->struct_type, 0); // Puntero al tipo estructurado de la clase
    }
    fprintf(stderr, RED "Error: Tipo desconocido '%s' en type_to_llvm.\n" RESET, type->name ? type->name : "NULL");
    exit(1);
}

LLVMTypeRef *build_struct_fields(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *type_info)
{
    LLVMUserTypeInfo *parent_info = type_info->parent_info;
    int parent_members = parent_info ? parent_info->num_data_members : 0;
    LLVMTypeRef *struct_fields = (LLVMTypeRef *)malloc((type_info->num_data_members + 2) * sizeof(LLVMTypeRef));
    struct_fields[0] = ctx->i32_type;                                     // Campo 0: ID del tipo
    struct_fields[1] = LLVMPointerType(type_info->vtable_struct_type, 0); // Campo 1: Puntero a vtable

    // type_info->members = (LLVMTypeMemberInfo **)malloc(type_info->num_data_members * sizeof(LLVMTypeMemberInfo *));

    for (int i = 0; i < parent_members; i++)
    {
        struct_fields[i + 2] = parent_info->struct_fields[i + 2];

        LLVMTypeMemberInfo *new_member = malloc(sizeof(LLVMTypeMemberInfo));

        char *base_member_name = delete_underscore_from_str(parent_info->members[i], parent_info->name);

        new_member->name = concat_str_with_underscore(type_info->name, base_member_name);
        new_member->llvm_type = parent_info->members[i]->llvm_type;
        new_member->index = parent_info->members[i]->index;
        new_member->default_value_node = parent_info->members[i]->default_value_node;

        type_info->members[i] = new_member;
    }

    for (int i = 0, index = 2 + parent_members; i < node->data.type_node.def_count; i++)
    {
        ASTNode *child = node->data.type_node.definitions[i];

        if (child->type == NODE_ASSIGNMENT)
        {
            fprintf(stderr, "El tipo de mi nodo es %s\n", child->data.op_node.right->return_type->name);
            const char *member_name = child->data.op_node.left->data.variable_name; // e.g., 'x' in 'x = 12'
            fprintf(stderr, "el nombre de mi varaibles es %s en el indice %d\n", member_name, index);

            LLVMTypeRef member_llvm_type = type_to_llvm(ctx, child->data.op_node.right->return_type);

            LLVMTypeMemberInfo *member = malloc(sizeof(LLVMTypeMemberInfo));

            fprintf(stderr, "El nombre de mi parametro es %s y el valor es de %d\n", child->data.op_node.left->data.variable_name, child->data.op_node.right->data.number_value);

            member->name = strdup(member_name);
            member->index = index;
            struct_fields[index] = member_llvm_type;
            member->llvm_type = member_llvm_type;
            member->default_value_node = child->data.op_node.right;
            type_info->members[index-2] = member;
            index++;
        }
    }

    // LLVMTypeMemberInfo** f = type_info->members;

    // for(int i =0;i<type_info->num_data_members;i++)
    // {
    //     fprintf(stderr,"el nombr de mi varaibles es f[]")
    // }



    return struct_fields;
}

LLVMValueRef *build_vtable_initializer(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *type_info)
{
    LLVMUserTypeInfo *parent_info = type_info->parent_info;
    int parent_methods = parent_info ? parent_info->num_methods_virtual : 0;
    LLVMValueRef *vtable_initializer_values = (LLVMValueRef *)malloc(type_info->num_methods_virtual * sizeof(LLVMValueRef));
    LLVMTypeRef *vtable_slot_types = type_info->vtable_slot_types;

    for (int i = 0; i < parent_methods; i++)
    {
        vtable_initializer_values[i] = parent_info->vtable_initializer_values[i];

        LLVMMethodInfo *new_method = malloc(sizeof(LLVMMethodInfo));
        
        char *base_method_name = delete_underscore_from_str(parent_info->methods[i]->name, parent_info->name);

        fprintf(stderr,GREEN"el base funcion es %s\n"RESET, base_method_name);

        char* new_name = concat_str_with_underscore(type_info->name,base_method_name);

        fprintf(stderr, GREEN "El nueov nombre es de %s\n"RESET,new_name );
        new_method->name = new_name;
        new_method->llvm_func_type = parent_info->methods[i]->llvm_func_type;
        new_method->llvm_func_value = parent_info->methods[i]->llvm_func_value;
        new_method->vtable_index = parent_info->methods[i]->vtable_index;
        new_method->node = parent_info->methods[i]->node;

        type_info->methods[i] = new_method;
    }

    for (int i = 0, aux = parent_methods; i < node->data.type_node.def_count; i++)
    {
        ASTNode *child = node->data.type_node.definitions[i];

        if (child->type == NODE_FUNC_DEC && child->data.func_node.flag_overriden == 0)
        {

            const char *func_name = child->data.func_node.name;

            LLVMValueRef getY_func = LLVMAddFunction(ctx->module, func_name, vtable_slot_types[aux]);

            fprintf(stderr, RED "func_name es %s\n" RESET, func_name);

            vtable_initializer_values[aux] = getY_func;

            LLVMMethodInfo *new_method = malloc(sizeof(LLVMMethodInfo));
            new_method->name = strdup(func_name);
            new_method->vtable_index = aux;
            new_method->llvm_func_type = vtable_slot_types[aux];
            new_method->llvm_func_value = getY_func;
            new_method->node = child;

            type_info->methods[aux++] = new_method;
        }
        else if (child->type == NODE_FUNC_DEC && child->data.func_node.flag_overriden >= 1)
        {
            int index = child->data.func_node.flag_overriden -1;

            const char *func_name = child->data.func_node.name;

            LLVMValueRef getY_func = LLVMAddFunction(ctx->module, func_name, vtable_slot_types[index]);

            fprintf(stderr, RED "func_name es %s\n" RESET, func_name);

            vtable_initializer_values[index] = getY_func;

            LLVMMethodInfo *new_method = malloc(sizeof(LLVMMethodInfo));
            new_method->name = strdup(func_name);
            new_method->vtable_index = index;
            new_method->llvm_func_type = vtable_slot_types[index];
            new_method->llvm_func_value = getY_func;
            new_method->node = child;

            type_info->methods[index] = new_method;
        }
    }

    return vtable_initializer_values;
}



int get_max_type_id(LLVMCoreContext* ctx)
{
    
    return ctx->user_types->id;
}


LLVMUserTypeInfo** get_type_info_array(LLVMCoreContext* ctx)
{
    LLVMUserTypeInfo* types = ctx->user_types;
    int cantidad =0;

    while(types)
    {
        cantidad++;
        types = types->next;
    }

    LLVMUserTypeInfo** array_types = (LLVMUserTypeInfo**) malloc(cantidad*sizeof(LLVMUserTypeInfo*));

    LLVMUserTypeInfo* new_types = ctx->user_types;

    for(int i=0;i<cantidad;i++)
    {
        LLVMUserTypeInfo* new_types2 = malloc(sizeof(LLVMUserTypeInfo));
        new_types2->id = new_types->id;
        new_types2->name = strdup(new_types->name);
        new_types2->struct_type = new_types;
        new_types2->vtable_struct_type =  new_types->vtable_struct_type;
        new_types2->class_ptr_type= new_types->class_ptr_type;
        new_types2->vtable_ptr_type=new_types->vtable_ptr_type;
        new_types2->vtable_global=new_types->vtable_global;
        new_types2->vtable_slot_ptr_types=new_types->vtable_slot_ptr_types;
        new_types2->vtable_slot_types=new_types->vtable_slot_types;
        new_types2->struct_fields= new_types->struct_fields;
        new_types2->vtable_initializer_values=new_types->vtable_initializer_values;

        new_types2->members =new_types->members ;
        new_types2->num_data_members= new_types->num_data_members;

        new_types2->methods=new_types->methods;
        new_types2->num_methods_virtual= new_types->num_methods_virtual;

        new_types2->parent_info=new_types->parent_info;
        new_types2->next =new_types->next ;
        array_types[i] = new_types2;

        new_types = new_types->next;
        
    }

    return array_types;
}
