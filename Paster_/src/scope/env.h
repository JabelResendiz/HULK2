
#ifndef ENV_H
#define ENV_H

#include "../type_value/type.h"


typedef struct EnvItem
{
    struct ASTNode *usages;
    TypeValue *computed_type;
    struct EnvItem *next;
} EnvItem;

typedef struct Env
{
    int env_count;
    EnvItem * start;
    struct Env *parent;
} Env;


Env * create_env(Env* parent);

void free_env(Env* env);

int register_type_member_in_env(Env *env, struct ASTNode *node, char *type_name);

EnvItem* find_env_item_by_kind(Env* env, const char* name, int is_function);

EnvItem* lookup_type_member_recursive(Env* context, const char* full_name, TypeValue* type, int is_function) ;

char* generate_underscored_name(const char* prefix, const char* base);
char* remove_type_prefix(const char* full_name,const char * type_name);

EnvItem *find_env_item(Env *env, char *name, int is_type, int var);

int create_env_item(Env *env, ASTNode *node, char *name, int is_type_decl);

#endif