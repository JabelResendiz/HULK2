

#ifndef UNIFIEDINDEX_H
#define UNIFIEDINDEX_H

#include "../visitor/visitor.h"

typedef struct UnifiedIndex
{
    int value;
    struct UnifiedIndex* next;
}UnifiedIndex;


void free_unified_index(UnifiedIndex* u);


UnifiedIndex *try_unified_type(ASTVisitor *v, 
                               ASTNode **args, 
                               Scope *scope, 
                               int arg_count, 
                               const char *t_name, 
                               EnvItem *item);

UnifiedIndex *try_unified_func(ASTVisitor *v, 
                               ASTNode **args, 
                               Scope *scope, 
                               int arg_count, 
                               const char *f_name, 
                               EnvItem *item);                          
UnifiedIndex* add_unified_index(UnifiedIndex* list, int index);

int try_unify_binary_op(ASTVisitor *v, ASTNode *left,
                        ASTNode *right, Operator op,
                        char *op_name);

int try_unify_operand(ASTVisitor *v,
                      ASTNode *node,
                      TypeValue *type);

int try_unify_conditional(ASTVisitor *v,
                          ASTNode *node,
                          TypeValue *type);

int try_unify_attr(ASTVisitor* v,ASTNode* node);

#endif