
// visitor_llvm

#ifndef VISITOR_LLVM_H
#define VISITOR_LLVM_H

#include "../ast/ast.h"
#include "init_codegen.h"
#include <llvm-c/Core.h>
#include <string.h>

typedef struct LLVMVisitor LLVMVisitor;

// --- Funciones de visita agrupadas por tipo ---
typedef struct {
    LLVMValueRef (*program)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*number)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*string)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*boolean)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*variable)(LLVMVisitor*, ASTNode*);
} LLVM_BasicVisitors;

typedef struct {
    LLVMValueRef (*binary)(LLVMVisitor*, ASTNode*);
    //LLVMValueRef (*unary)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*assignment)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*call_function)(LLVMVisitor*, ASTNode*);
} LLVM_ExpressionVisitors;

typedef struct {
    LLVMValueRef (*block)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*dec_function)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*let_in)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*conditional)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*while_loop)(LLVMVisitor*, ASTNode*);
} LLVM_ControlVisitors;

typedef struct {
    LLVMValueRef (*type_dec)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*type_instance)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*casting)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*test_type)(LLVMVisitor*, ASTNode*);
} LLVM_TypeVisitors;

typedef struct {
    LLVMValueRef (*attr_getter)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*attr_setter)(LLVMVisitor*, ASTNode*);
    LLVMValueRef (*method_getter)(LLVMVisitor*,ASTNode*);
    LLVMValueRef (*base_func)(LLVMVisitor*, ASTNode*);
} LLVM_AttributeVisitors;

// --- Estructura del visitor ---
typedef struct LLVMVisitor {
    
    LLVMCoreContext* ctx;

    // Visitadores
    LLVM_BasicVisitors basic;
    LLVM_ExpressionVisitors expr;
    LLVM_ControlVisitors control;
    LLVM_TypeVisitors types;
    LLVM_AttributeVisitors attrs;

    LLVMValueRef current_function;

}LLVMVisitor;

// --- Funciones de utilidades ---


LLVMValueRef codegen_accept(LLVMVisitor* visitor, ASTNode* node);


#endif