#ifndef CST_TO_AST_H
#define CST_TO_AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "derivation_tree.h"
#include "../ast/ast_compat.h"

// Función principal para convertir CST a AST
Program* cst_to_ast_convert(DerivationNode* cst_root);

// Funciones auxiliares para conversión
Stmt* convert_stmt_node(DerivationNode* node);
Expr* convert_expr_node(DerivationNode* node);
FunctionDecl* convert_function_decl_node(DerivationNode* node);
TypeDecl* convert_type_decl_node(DerivationNode* node);
AttributeDecl* convert_attribute_decl_node(DerivationNode* node);
MethodDecl* convert_method_decl_node(DerivationNode* node);

// Funciones para conversión de expresiones específicas
Expr* convert_binary_expr_node(DerivationNode* node);
Expr* convert_unary_expr_node(DerivationNode* node);
Expr* convert_call_expr_node(DerivationNode* node);
Expr* convert_get_attr_expr_node(DerivationNode* node);
Expr* convert_new_expr_node(DerivationNode* node);
Expr* convert_is_expr_node(DerivationNode* node);
Expr* convert_as_expr_node(DerivationNode* node);
Expr* convert_base_call_expr_node(DerivationNode* node);
Expr* convert_let_expr_node(DerivationNode* node);
Expr* convert_if_expr_node(DerivationNode* node);
Expr* convert_while_expr_node(DerivationNode* node);
Expr* convert_for_expr_node(DerivationNode* node);
Expr* convert_block_expr_node(DerivationNode* node);
Expr* convert_assignment_expr_node(DerivationNode* node);

// Funciones para conversión de tipos
TypeInfo* convert_type_node(DerivationNode* node);
VarBinding* convert_var_binding_node(DerivationNode* node);

// Funciones auxiliares
char* extract_string_value(DerivationNode* node);
double extract_number_value(DerivationNode* node);
bool extract_bool_value(DerivationNode* node);
BinaryOp extract_binary_op(DerivationNode* node);
UnaryOp extract_unary_op(DerivationNode* node);

// Funciones para manejo de listas
Stmt** convert_stmt_list(DerivationNode* node, size_t* count);
Expr** convert_expr_list(DerivationNode* node, size_t* count);
char** convert_string_list(DerivationNode* node, size_t* count);
TypeInfo** convert_type_list(DerivationNode* node, size_t* count);
VarBinding* convert_var_binding_list(DerivationNode* node, size_t* count);
AttributeDecl** convert_attribute_list(DerivationNode* node, size_t* count);
MethodDecl** convert_method_list(DerivationNode* node, size_t* count);

// Funciones de utilidad
bool is_terminal_node(DerivationNode* node);
bool is_non_terminal_node(DerivationNode* node);
DerivationNode* find_child_by_symbol(DerivationNode* node, const char* symbol);
DerivationNode** find_children_by_symbol(DerivationNode* node, const char* symbol, size_t* count);

// Constantes para nombres de símbolos gramaticales
#define SYMBOL_PROGRAM "Program"
#define SYMBOL_STMT "Stmt"
#define SYMBOL_EXPR "Expr"
#define SYMBOL_FUNCTION_DECL "FunctionDecl"
#define SYMBOL_TYPE_DECL "TypeDecl"
#define SYMBOL_ATTRIBUTE_DECL "AttributeDecl"
#define SYMBOL_METHOD_DECL "MethodDecl"
#define SYMBOL_BINARY_EXPR "BinaryExpr"
#define SYMBOL_UNARY_EXPR "UnaryExpr"
#define SYMBOL_CALL_EXPR "CallExpr"
#define SYMBOL_GET_ATTR_EXPR "GetAttrExpr"
#define SYMBOL_NEW_EXPR "NewExpr"
#define SYMBOL_IS_EXPR "IsExpr"
#define SYMBOL_AS_EXPR "AsExpr"
#define SYMBOL_BASE_CALL_EXPR "BaseCallExpr"
#define SYMBOL_LET_EXPR "LetExpr"
#define SYMBOL_IF_EXPR "IfExpr"
#define SYMBOL_WHILE_EXPR "WhileExpr"
#define SYMBOL_FOR_EXPR "ForExpr"
#define SYMBOL_BLOCK_EXPR "BlockExpr"
#define SYMBOL_ASSIGN_EXPR "AssignExpr"
#define SYMBOL_TYPE "Type"
#define SYMBOL_VAR_BINDING "VarBinding"
#define SYMBOL_STMT_LIST "StmtList"
#define SYMBOL_EXPR_LIST "ExprList"
#define SYMBOL_STRING_LIST "StringList"
#define SYMBOL_TYPE_LIST "TypeList"
#define SYMBOL_VAR_BINDING_LIST "VarBindingList"
#define SYMBOL_ATTRIBUTE_LIST "AttributeList"
#define SYMBOL_METHOD_LIST "MethodList"

#endif // CST_TO_AST_H