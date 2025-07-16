


#ifndef CTS_TO_AST
#define CTS_TO_AST

#include "../ast/ast.h"
#include "ll1_structures.h"

// Define colores para stderr
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

ASTNode* cst_to_ast(CSTNode* cts_node);
ASTNode*  stmtList_to_ast(CSTNode* cst_node);
ASTNode* terminated_stmt_to_ast(CSTNode* cst_node);

ASTNode* program_to_ast(CSTNode* cst_node);
ASTNode* number_to_ast (CSTNode* cst_node);
ASTNode* string_to_ast(CSTNode* cst_node);
ASTNode* boolean_to_ast(CSTNode* cst_node);
ASTNode* block_to_ast(CSTNode* cst_node);
ASTNode* binary_expr_to_ast(CSTNode* cst_node);



#endif
