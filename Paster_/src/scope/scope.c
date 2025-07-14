
// scope.c

#include "scope.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "../ast/ast.h"

/// @brief Creacion de un scope
/// @param parent El scope que sera anidado
/// @return El nuevo scope
Scope *create_scope(Scope *parent)
{
    Scope *scope = (Scope *)malloc(sizeof(Scope));

    scope->symbols = NULL;
    scope->functions = NULL;
    scope->types = NULL;
    scope->parent = parent;
    scope->functions = (FuncTable *)malloc(sizeof(FuncTable));
    scope->functions->count = 0;
    scope->functions->first = NULL;
    scope->s_count = 0;
    scope->t_count = 0;

    return scope;
}

/// @brief Busqueda de un simbolo en un cierto scope con un nombre
/// @param scope El scope donde buscar (no es recursivo)
/// @param name El nombre del simbolo
/// @return NULL si no hay un symbol con ese nombre , el Symbol si existe
Symbol *find_symbol_in_scope(Scope *scope, const char *name)
{
    // solo busca en el entorno actual, no accede a los padres de ese entorno

    Symbol *current = scope->symbols;

    for (int i = 0; i < scope->s_count; i++)
    {
        if (!strcmp(current->name, name))
            return current;

        current = current->next;
    }

    return NULL;
}

/// @brief buscar una variable en el scope de forma recursiva
/// @param scope el scope por donde empieza
/// @param name el nombre de la variable a buscar
/// @return NULL si no hay un symbol con ese nombre, el Symbol si existe
Symbol *scope_lookup(Scope *scope, const char *name)
{
    // es el find_symbol
    // busca en el actual y en todos los padres

    // if (!scope)
    // {
    //     return NULL;
    // }

    Scope *current = scope;

    while (current)
    {
        // for (Symbol *s = current->symbols; s; s = s->next)
        // {
        //     if (!strcmp(s->name, name))
        //         return s;
        // }

        Symbol *s = find_symbol_in_scope(current, name);

        if (s)
            return s;

        current = current->parent;
    }

    return NULL;
}

// agrega un simbolo a un scope o actualiza si ya existe el symbolo
void scope_add_symbol(Scope *scope,
                      const char *name,
                      TypeValue *type,
                      struct ASTNode *value,
                      int param)
{
    Symbol *s = find_symbol_in_scope(scope, name);

    if (s)
    {
        s->type = type;
        s->param = param;
        return;
    }

    Symbol *symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->param = param;
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    scope->s_count++;
}

// agrega una function a un scope
void scope_add_function(Scope *scope,
                        int arg_count,
                        TypeValue **args_types,
                        TypeValue *result_tpyes,
                        char *name)
{
    Function *func = malloc(sizeof(Function));
    func->name = name;
    func->count_args = arg_count;
    func->next = NULL;
    func->result_types = result_tpyes;

    if (arg_count > 0)
    {
        func->args_types = malloc(arg_count * sizeof(TypeValue *));
        for (int i = 0; i < arg_count; i++)
        {
            func->args_types[i] = args_types[i];
        }
    }
    else
    {
        func->args_types = NULL;
    }

    func->next = scope->functions->first;
    scope->functions->first = func;
    scope->functions->count++;
}

/// @brief Agregar un tipo al Scope dado
/// @param scope  Scopa a agregar el tipo
/// @param type El tipo que se agregar al Scope
void scope_add_type(Scope *scope, TypeValue *type)
{
    Symbol *def_type = malloc(sizeof(Symbol));
    def_type->name = type->name;
    def_type->type = type;
    def_type->next = scope->types;

    scope->types = def_type;
    scope->t_count++;
}

/// @brief Agregar los tipos y funciones builtins del program
/// @param scope El scope base del programa
void init(Scope *scope)
{

    scope_add_type(scope, &TYPE_NUM);
    scope_add_type(scope, &TYPE_STRING);
    scope_add_type(scope, &TYPE_BOOLEAN);
    scope_add_type(scope, &TYPE_OBJ);
    scope_add_type(scope, &TYPE_VOID);
    scope_add_type(scope, &TYPE_ERROR);

    scope_add_function(
        scope, 0, NULL, &TYPE_VOID, "print");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_VOID, "print");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_BOOLEAN}, &TYPE_VOID, "print");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_STRING}, &TYPE_VOID, "print");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_NUM, "sqrt");

    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_NUM, "sin");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_NUM, "cos");
    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_NUM, "exp");

    scope_add_function(
        scope, 1, (TypeValue *[]){&TYPE_NUM}, &TYPE_NUM, "log");

    scope_add_function(
        scope, 2, (TypeValue *[]){&TYPE_NUM, &TYPE_NUM}, &TYPE_NUM, "log");
    scope_add_function(
        scope, 0, NULL, &TYPE_NUM, "rand");
}

/// @brief liberar un symbol
/// @param symbol EL symbol a liberar
void free_symbol(Symbol *symbol, int count)
{
    for (int i = 0; i < count; i++)
    {
        Symbol *next = symbol->next;
        free(symbol);
        symbol = next;
    }
}

/// @brief LIberar un scope
/// @param scope El scope a liberar
void free_scope(Scope *scope)
{
    if (scope == NULL)
    {
        return;
    }

    free_symbol(scope->symbols, scope->s_count);
    free_symbol(scope->types, scope->t_count);
    free_func_table(scope->functions);
    free(scope);
}

/// @brief (find_defined_type)Busca un tipo de dato definido por su nombre dentro de una jerarquia de ambito
/// @param scope Se parece mucho al scope_lookup
/// @param name
/// @return El symbol que coincide con el nombre en el Scope (recursivo)
Symbol *find_type_scopes(Scope *scope, const char *type_name)
{
    fprintf(stderr,"ESTOY EN EL FIND_TYPE_SCOPES con %s\n",type_name);
    // si alguno es nulo, retorna NULL

    if (!scope || !type_name)
    {
        fprintf(stderr, "es nulo\n");
        return NULL;
    }

    // fprintf(stderr,"el nombre es : %s\n", name);
    //  busqueda en el ambito actual
    Symbol *current = scope->types;
    int i = 0;
    while (i < scope->t_count)
    {
        if (!strcmp(current->name, type_name))
        {
            return current;
        }
        current = current->next;
        i++;
    }

    // si no se encontro en el ambito actual buscar en el padre
    if (scope->parent)
    {
        return find_type_scopes(scope->parent, type_name);
    }

    // si no hay padre retornar NULL
    return NULL;
}

/// @brief Busca un parametro en el scope actual y sus ancestros (recursivo)
/// @param scope El scope donde empieza la busqueda (parecido al scope_lookup)
/// @param name El nombre del parametro
/// @return El Symbol encontrado o NULL (eoc)
Symbol *lookup_parameter(Scope *scope, const char *name)
{
    fprintf(stderr,"estoy en lookup parameter , la variable que busco se llama %s\n",name);

         fprintf(stderr,"12-hay %d simbolos\n",scope->s_count);
    if (!scope) {
        return NULL;
    }

    Symbol* current = scope->symbols;
    int i = 0;
    fprintf(stderr,"%d\n",scope->s_count);
    while (i < scope->s_count) {
        fprintf(stderr,"La current name es %s\n",current->name);
        fprintf(stderr,"el valor es %d\n", current->param);

        if (current->param && !strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
        i++;
    }

    if (current)
        return current;
    
    if (scope->parent) {
        return lookup_parameter(scope->parent, name);
    }
    
    return NULL;
}

/// @brief Busca un atributo en un type , incluyendo sus padres
/// @param type El tipo asociado
/// @param attr_name El nombre del atributo
/// @return El simbolo que representa el atributo
Symbol *lookup_type_attr(TypeValue *type, char *attr_name)
{
    if (!type->def_node)
        return NULL;

    Symbol *s = find_symbol_in_scope(type->def_node->scope, attr_name);

    if (!s && type->super_type)
    {
        char *name = remove_type_prefix(attr_name, type->name);
        name = generate_underscored_name(type->super_type->name, name);
        return lookup_type_attr(type->super_type, name);
    }

    return s;
}
