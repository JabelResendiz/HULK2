#ifndef AST_COMPAT_H
#define AST_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include "ast.h"
#include "type_value/type.h"

// Alias para TypeInfo (usando TypeValue del sistema existente)
typedef TypeValue TypeInfo;

// Estructura para representar un binding de variable en C
typedef struct {
    char* name;
    ASTNode* initializer;
    TypeInfo* declaredType;
} VarBinding;

// Tipos de compatibilidad para el parser C (todos son ASTNode)
typedef ASTNode Program;
typedef ASTNode Stmt;
typedef ASTNode Expr;
typedef ASTNode FunctionDecl;
typedef ASTNode TypeDecl;
typedef ASTNode ExprStmt;
typedef ASTNode BinaryExpr;
typedef ASTNode UnaryExpr;
typedef ASTNode LetExpr;
typedef ASTNode IfExpr;
typedef ASTNode WhileExpr;
typedef ASTNode ForExpr;
typedef ASTNode CallExpr;
typedef ASTNode NewExpr;
typedef ASTNode IsExpr;
typedef ASTNode AsExpr;
typedef ASTNode BaseCallExpr;
typedef ASTNode AssignExpr;
typedef ASTNode ExprBlock;
typedef ASTNode AttributeDecl;
typedef ASTNode MethodDecl;

// Enums para operadores (usando los existentes)
typedef enum {
    BINARY_OP_UNKNOWN = -1,
    BINARY_OP_PLUS = 0,  // OP_ADD
    BINARY_OP_MINUS = 1, // OP_SUB
    BINARY_OP_MULT = 2,  // OP_MULT
    BINARY_OP_DIV = 3,   // OP_DIV
    BINARY_OP_MOD = 7,   // OP_MOD
    BINARY_OP_POW = 6,   // OP_POW
    BINARY_OP_CONCAT = 4, // OP_CONCAT
    BINARY_OP_CONCAT_WS = 5, // OP_DCONCAT
    BINARY_OP_EQ = 13,   // OP_EQ
    BINARY_OP_NEQ = 10,  // OP_NEQ
    BINARY_OP_LESS_THAN = 12, // OP_LT
    BINARY_OP_GREATER_THAN = 11, // OP_GT
    BINARY_OP_LE = 15,   // OP_LE
    BINARY_OP_GE = 14,   // OP_GE
    BINARY_OP_AND = 8,   // OP_AND
    BINARY_OP_OR = 9,    // OP_OR
    BINARY_OP_ADD = 0,   // Alias for PLUS
    BINARY_OP_SUB = 1,   // Alias for MINUS
    BINARY_OP_MUL = 2,   // Alias for MULT
    BINARY_OP_DIV_OP = 3 // Alias for DIV
} BinaryOp;

typedef enum {
    UNARY_OP_UNKNOWN = -1,
    UNARY_OP_PLUS = 0,   // OP_ADD
    UNARY_OP_MINUS = 1,  // OP_SUB
    UNARY_OP_NOT = 10    // OP_NEQ (usado como NOT)
} UnaryOp;

// Funciones de creación de AST compatibles
Program* create_program(void);
void program_add_stmt(Program* program, Stmt* stmt);
void free_program(Program* program);

Stmt* create_expr_stmt(Expr* expr);
void free_stmt(Stmt* stmt);

Expr* create_variable_expr(const char* name);
Expr* create_number_expr(double value);
Expr* create_string_expr(const char* value);
Expr* create_bool_expr(bool value);
Expr* create_binary_expr(Expr* left, Expr* right, BinaryOp op);
Expr* create_unary_expr(Expr* operand, UnaryOp op);
Expr* create_call_expr(const char* func_name, Expr** args, size_t arg_count);
Expr* create_method_call_expr(Expr* object, const char* method_name, Expr** args, size_t arg_count);
Expr* create_get_attr_expr(Expr* object, const char* attr_name);
Expr* create_new_expr(const char* type_name, Expr** args, size_t arg_count);
Expr* create_let_expr(VarBinding* bindings, size_t binding_count, Stmt* body);
Expr* create_if_expr(Expr* condition, Stmt* then_body, Stmt* else_body);
Expr* create_while_expr(Expr* condition, Stmt* body);
Expr* create_for_expr(const char* var_name, Expr* collection, Stmt* body);
Expr* create_for_loop_node(const char* var_name, Expr** range_args, ASTNode* body, size_t range_arg_count);
Expr* create_is_expr(Expr* object, const char* type_name);
Expr* create_as_expr(Expr* object, const char* type_name);
Expr* create_base_call_expr(Expr** args, size_t arg_count);
Expr* create_assign_expr(const char* var_name, Expr* value);
Expr* create_expr_block(Stmt** stmts, size_t stmt_count);

Stmt* create_block_stmt(Stmt** stmts, size_t stmt_count);

Stmt* create_return_stmt_wrapper(Expr* value);

FunctionDecl* create_function_decl(const char* name, char** params, size_t param_count, 
                                 Stmt* body, TypeValue** param_types, TypeValue* return_type);
TypeDecl* create_type_decl(const char* name, char** params, size_t param_count,
                          const char* base_type, Expr** base_args, size_t base_arg_count,
                          AttributeDecl** attrs, size_t attr_count,
                          MethodDecl** methods, size_t method_count);

AttributeDecl* create_attribute_decl(const char* name, Expr* initializer, TypeInfo* type);
MethodDecl* create_method_decl(const char* name, char** params, size_t param_count,
                              Stmt* body, TypeInfo** param_types, TypeInfo* return_type);

void free_expr(Expr* expr);
void free_function_decl(FunctionDecl* func);
void free_type_decl(TypeDecl* type);
void free_attribute_decl(AttributeDecl* attr);
void free_method_decl(MethodDecl* method);

#endif // AST_COMPAT_H 