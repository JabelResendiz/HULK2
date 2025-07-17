


#ifndef CTS_TO_AST
#define CTS_TO_AST

#include "../ast/ast.h"
#include "ll1_structures.h"
#include "cst_to_ast2.h"

// Define colores para stderr
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

#define cst CSTNode* cst_node

ASTNode* cst_to_ast(cst);
ASTNode*  stmtList_to_ast(cst);
ASTNode* terminated_stmt_to_ast(cst);
ASTNode* stmt_to_ast(cst);

// a partir de aqui son las construcciones oficiales
ASTNode* program_to_ast(cst);

// ------------ STATEMENT ----------
ASTNode* expr_to_ast(cst);
ASTNode* block_to_ast(cst);
ASTNode* functionDef_to_ast(cst);
ASTNode* typeDef_to_ast(cst);
ASTNode* while_to_ast(cst);

// el for no esta implementado no es necesario

// ------------ EXPRESSION ------------
ASTNode* conditional_to_ast(cst);
ASTNode* let_in_to_ast(cst);
ASTNode* assignment_to_ast(cst);
ASTNode* funCall_to_ast(cst);

// ------------ PRIMARY_EXPR ----------
ASTNode* number_to_ast (cst);
ASTNode* string_to_ast(cst);
ASTNode* boolean_to_ast(cst);
ASTNode* variable_to_ast(cst);

// ------------- TYPE ----------------
ASTNode* casting_to_ast(cst); // is y as
ASTNode* instance_to_ast(cst);
ASTNode* getter_to_ast(cst);
ASTNode* setter_to_ast(cst);
ASTNode* base_to_ast(cst);

// ------------ OP --------------   
ASTNode* binary_expr_to_ast(cst);
ASTNode* unary_expr_to_ast(cst);


#endif
