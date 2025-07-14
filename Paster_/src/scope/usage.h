


#ifndef USAGE_H
#define USAGE_H

#include "env.h"
typedef struct UsageElemental
{
    struct ASTNode* node;
    struct UsageElemental* next;
}UsageElemental;

typedef struct Usage {
    UsageElemental* first;   // una lista de astnodes
    UsageElemental* last;
    int count;              // Contador de referencias
} Usage;



Usage* add_usages(struct ASTNode* node , Usage* list);


Usage* find_type_by_method(Env* env,char * name);

int type_contains_method_in_context(ASTNode* node,char* name);
int type_contains_method_in_scope(TypeValue* type,char*name,int see_parent);


#endif