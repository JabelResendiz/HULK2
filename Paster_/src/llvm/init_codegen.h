#ifndef INIT_CODEGEN_H
#define INIT_CODEGEN_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include "ast/ast.h"
#include <stdio.h>



typedef struct LLVMCoreContext
{
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
    LLVMValueRef current_stack_depth_var;
    int max_stack_depth;

}LLVMCoreContext;


// Inicializacion/limpieza
LLVMCoreContext* llvm_core_context_create();
void  free_llvm_core_context(LLVMCoreContext* ctx);

// Funciones runtime
void llvm_declare_builtins(LLVMCoreContext* ctx);

// Manejor de errores
void llvm_handle_stack_overflow(
    LLVMCoreContext* ctx,
    ASTNode* node
);



// void init_llvm(void);
// void free_llvm_resources(void);
// void declare_external_functions(void);

// static inline void handle_stack_overflow(
//     LLVMBuilderRef builder, LLVMModuleRef module,
//     LLVMValueRef current_stack_depth_var, ASTNode *node)
// {
//     // Construir mensaje de error con nombre de función y línea
//     char error_msg[256];
//     snprintf(error_msg, sizeof(error_msg),
//              RED "!!RUNTIME ERROR: Stack overflow detected in function '%s'. Line: %d.\n" RESET,
//              node->data.func_node.name,
//              node->line);

//     LLVMValueRef error_msg_global = LLVMBuildGlobalStringPtr(builder, error_msg, "error_msg");

//     // Llamar a puts para imprimir el mensaje
//     LLVMValueRef puts_func = LLVMGetNamedFunction(module, "puts");
//     // LLVMBuildCall(builder, puts_func, &error_msg_global, 1, "");

//     LLVMTypeRef puts_type = LLVMFunctionType(LLVMInt32Type(),
//                                              (LLVMTypeRef[]){LLVMPointerType(LLVMInt8Type(), 0)},
//                                              1, 0);
//     LLVMBuildCall2(builder, puts_type, puts_func, &error_msg_global, 1, "");

//     // Llamar a exit(1)
//     LLVMValueRef exit_func = LLVMGetNamedFunction(module, "exit");
//     LLVMValueRef exit_code = LLVMConstInt(LLVMInt32Type(), 0, 0);
//     // LLVMBuildCall(builder, exit_func, &exit_code, 1, "");

//     LLVMTypeRef exit_type = LLVMFunctionType(LLVMVoidType(),
//                                              (LLVMTypeRef[]){LLVMInt32Type()},
//                                              1, 0);
//     LLVMBuildCall2(builder, exit_type, exit_func, &exit_code, 1, "");
//     // Marcar como unreachable
//     LLVMBuildUnreachable(builder);
// }

#endif