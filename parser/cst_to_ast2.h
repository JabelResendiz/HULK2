#ifndef CST_TO_AST2_H
#define CST_TO_AST2_H

#include "../ast/ast.h"
#include "ll1_structures.h"

// ------------ EXPRESSION TRANSFORMATIONS ------------
ASTNode* conditional_to_ast(CSTNode* cst_node);
ASTNode* let_in_to_ast(CSTNode* cst_node);
ASTNode* assignment_to_ast(CSTNode* cst_node);
ASTNode* funCall_to_ast(CSTNode* cst_node);
ASTNode* while_to_ast(CSTNode* cst_node);

// ------------ HELPER FUNCTIONS ------------
ASTNode* if_body_to_ast(CSTNode* cst_node);
ASTNode* process_elif_list(CSTNode* elif_list_node, ASTNode* else_body);

#endif // CST_TO_AST2_H 