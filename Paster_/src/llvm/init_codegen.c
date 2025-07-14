
// init_codegen.c

#include "init_codegen.h"
#include <stdlib.h>


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

LLVMCoreContext *llvm_core_context_create()
{
    LLVMCoreContext *ctx = malloc(sizeof(LLVMCoreContext));

    if (!ctx)
        return NULL;

    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    ctx->context = LLVMGetGlobalContext();
    ctx->module = LLVMModuleCreateWithNameInContext("program", ctx->context);
    ctx->builder = LLVMCreateBuilderInContext(ctx->context);
    ctx->max_stack_depth = 100000;
    
    // Inicializar la variable global de profundidad de stack
    ctx->current_stack_depth_var = LLVMAddGlobal(
        ctx->module,
        LLVMInt32Type(),
        "current_stack_depth");
    LLVMSetInitializer(ctx->current_stack_depth_var, LLVMConstInt(LLVMInt32Type(), 0, 0));
    LLVMSetLinkage(ctx->current_stack_depth_var, LLVMPrivateLinkage);

    return ctx;
}

void free_llvm_core_context(LLVMCoreContext *ctx)
{
    if (!ctx)
        return;

    LLVMDisposeBuilder(ctx->builder);
    LLVMDisposeModule(ctx->module);
    free(ctx);
}

void llvm_declare_builtins(LLVMCoreContext *ctx)
{
    // Funciones de strings
    DECLARE_FUNC(ctx->module, "strcpy", VOID_TYPE, I8_PTR_TYPE, I8_PTR_TYPE);
    DECLARE_FUNC(ctx->module, "strcat", VOID_TYPE, I8_PTR_TYPE, I8_PTR_TYPE);
    DECLARE_FUNC(ctx->module, "strlen", I64_TYPE, I8_PTR_TYPE);

    // Funciones matemáticas
    DECLARE_FUNC(ctx->module, "sqrt", DOUBLE_TYPE, DOUBLE_TYPE);
    DECLARE_FUNC(ctx->module, "sin", DOUBLE_TYPE, DOUBLE_TYPE);
    DECLARE_FUNC(ctx->module, "cos", DOUBLE_TYPE, DOUBLE_TYPE);
    
    
    DECLARE_FUNC(ctx->module,"log", DOUBLE_TYPE,DOUBLE_TYPE);
    DECLARE_FUNC(ctx->module,"exp",DOUBLE_TYPE,DOUBLE_TYPE);
    DECLARE_FUNC(ctx->module,"rand",I32_TYPE);
    DECLARE_FUNC(ctx->module,"pow",DOUBLE_TYPE,DOUBLE_TYPE,DOUBLE_TYPE);
    DECLARE_FUNC(ctx->module,"fmod",DOUBLE_TYPE,DOUBLE_TYPE,DOUBLE_TYPE);
    // Memoria
    DECLARE_FUNC(ctx->module, "malloc", I8_PTR_TYPE, I64_TYPE);

    // I/O (variádicas)
    DECLARE_VARIADIC_FUNC(ctx->module, "printf", I32_TYPE, I8_PTR_TYPE);
    DECLARE_FUNC(ctx->module, "puts", I32_TYPE, I8_PTR_TYPE);
    DECLARE_VARIADIC_FUNC(ctx->module,"snprintf",I32_TYPE,I8_PTR_TYPE,I64_TYPE,I8_PTR_TYPE);
    DECLARE_FUNC(ctx->module,"strcmp",I32_TYPE,I8_PTR_TYPE,I8_PTR_TYPE);
    
    // Control
    DECLARE_FUNC(ctx->module, "exit", VOID_TYPE, I32_TYPE);
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