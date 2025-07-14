
// check.h

#ifndef CHECK_SEMANTIC_H
#define CHECK_SEMANTIC_H
#include <stdlib.h>
#include "../ast/ast.h"
#include "../visitor/visitor.h"
#include "../scope/scope.h"
#include "../error/error.h"


int make_checker(ASTNode *node);
void visit_program(ASTVisitor *v, ASTNode *node);
void visit_number(ASTVisitor *v, ASTNode *node);
void visit_string(ASTVisitor *v, ASTNode *node);
void visit_boolean(ASTVisitor *v, ASTNode *node);
void visit_variable(ASTVisitor *v, ASTNode *node);
void visit_binary_op(ASTVisitor *v, ASTNode *node);
void visit_block(ASTVisitor* v, ASTNode* node);
void visit_let(ASTVisitor *v, ASTNode *node);
void visit_assignment(ASTVisitor *v, ASTNode *node);
void visit_call_function(ASTVisitor *v, ASTNode *node);
void check_call_function(ASTVisitor* v,ASTNode* node,TypeValue* type);
void visit_dec_function(ASTVisitor *v, ASTNode *node);
void check_dec_function(ASTVisitor *v, ASTNode *node, TypeValue *type);
void visit_conditional(ASTVisitor *v, ASTNode *node);
void visit_while_loop(ASTVisitor *v, ASTNode *node);
void visit_type_dec(ASTVisitor *v, ASTNode *node);
void visit_type_instance(ASTVisitor *v, ASTNode *node);
void visit_getter(ASTVisitor *v, ASTNode *node);
void visit_setter(ASTVisitor *v, ASTNode *node);


void initialize_program_environment(ASTVisitor *v, ASTNode *node);


#endif
