

#ifndef SCOPE_H
#define SCOPE_H

#include "../type_value/type.h"
#include "function.h"

typedef struct Symbol Symbol;
typedef struct Function Function;
typedef struct Scope Scope;
typedef struct EnvItem EnvItem;
typedef struct Env Env;
typedef struct UsageElemental UsageElemental;
typedef struct Usage Usage;

// Representa una variable o tipo definido el lenguaje
typedef struct Symbol
{
    char *name;
    int param;
    int is_type_param;
    TypeValue* type;
    Symbol *next;
    Usage* usage;
} Symbol;


// TIEMPO DE EJECUCION
typedef struct Scope
{
    int s_count;
    int t_count;
    Symbol *symbols;
    FuncTable* functions;
    Symbol* types;  // para los tipos del scope
    Scope *parent;
} Scope;



Scope* create_scope(Scope *parent);

void free_scope(Scope *scope);

void scope_add_symbol(Scope *scope,
               const char *name,
               TypeValue* type,
               struct ASTNode *value , int param);

void scope_add_function (Scope* scope,
                         int arg_count,
                         TypeValue** args_types,
                         TypeValue* result_tpyes,
                         char* name);

void scope_add_type(Scope * scope, TypeValue* type);

Symbol* find_symbol_in_scope(Scope* scope,const char* name);
Symbol* scope_lookup(Scope* scope,const char* name);


void init(Scope* scope);

void free_scope(Scope* scope);


// void free_symbol(Symbol* symbol);



Symbol* find_type_scopes(Scope* scope, const char* type_name);

Symbol* lookup_parameter (Scope* scope, const char * name);




Symbol* lookup_type_attr(TypeValue* type,char* attr_name);




#endif
