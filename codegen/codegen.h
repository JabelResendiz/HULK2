

#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>
#include "../ast/ast.h"
#include "init_codegen.h"
#include "visitor_llvm.h"
#include "../scope/llvm_scope.h"
#include <stdlib.h>

// Define colores para stderr
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

typedef LLVMValueRef (*BuiltinFuncHandler)(LLVMVisitor *, ASTNode *);
typedef struct
{
    const char *name;
    BuiltinFuncHandler handler;
    const char *llvm_name;
} BuiltinFunction;




// Funcion para obtnener el tipo LLVM correspondiente a un tipo de lenguaje
LLVMTypeRef type_to_llvm(LLVMCoreContext*ctx,Type *type);

// Generar la funcion main en LLVM IR
void compile_to_llvm(ASTNode *node, const char *filename);

void find_function_dec(LLVMVisitor *visitor, ASTNode *node);
LLVMValueRef make_function_dec(LLVMVisitor *v, ASTNode *node);
void make_body_function_dec(LLVMVisitor *visitor, ASTNode *node);

// Genera el cuerpo principal del program
LLVMValueRef codegen_program(LLVMVisitor *v, ASTNode *node);

// Funciones especificas
LLVMValueRef codegen_number(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_string(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_boolean(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_variable(LLVMVisitor *v, ASTNode *node);

LLVMValueRef codegen_binary_op(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_assignments(LLVMVisitor *v, ASTNode *node);

LLVMValueRef codegen_block(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_let_in(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_dec_function(LLVMVisitor* v,ASTNode* node);
LLVMValueRef codegen_conditional(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_while(LLVMVisitor *v, ASTNode *node);


// Genera código LLVM para funciones built-in como print, sqrt, sin, etc.
LLVMValueRef codegen_call_function(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_print(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_math_function(LLVMVisitor *v, ASTNode *node, const char *name, const char *tmp_name);
LLVMValueRef codegen_rand(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_log(LLVMVisitor *v, ASTNode *node);
LLVMValueRef codegen_custom_func(LLVMVisitor *v, ASTNode *node);
LLVMValueRef handle_object_operation(LLVMVisitor *v, LLVMValueRef left, LLVMValueRef right, int op);


// DE TIPOS
LLVMValueRef codegen_type_dec(LLVMVisitor* v,ASTNode*node);
// void generate_type_methods(LLVMVisitor *v, const char *type_name, LLVMTypeRef struct_type, ASTNode *type_node);

LLVMValueRef codegen_type_instance(LLVMVisitor* v, ASTNode* node);
LLVMValueRef codegen_attr_setter(LLVMVisitor*v , ASTNode* node);
LLVMValueRef codegen_attr_getter(LLVMVisitor* v,ASTNode* node);

LLVMValueRef codegen_method_getter(LLVMVisitor* v, ASTNode* node);


LLVMValueRef codegen_dec_method(LLVMVisitor* v, ASTNode* node, LLVMUserTypeInfo* type_info);


void find_type_dec(LLVMVisitor *v, ASTNode *node);
//void make_body_type_dec(LLVMVisitor *v, ASTNode *node);
// LLVMTypeRef get_llvm_method_func_type(LLVMCoreContext *ctx, ASTNode *func_decl_node);
// void codegen_method_body(LLVMVisitor *v, LLVMUserTypeInfo *type_info, ASTNode *method_decl_node, LLVMValueRef llvm_func);
void build_vtable_table(LLVMVisitor *v, ASTNode *node);

#endif