
#include "scope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../ast/ast.h"

Env *create_env(Env *parent)
{
    Env *env = malloc(sizeof(Env));
    env->start = NULL;
    env->parent = parent;
    env->env_count = 0;

    return env;
}

void free_env(Env *env)
{
    if (env)
    {
        EnvItem *curr = env->start;

        while (curr)
        {
            EnvItem *next = curr->next;
            free(curr);
            curr = next;
        }

        free(env);
    }
}

// guarda un item dentro de un entorno asociado a un tipo (save_context_fr_tyep)
int register_type_member_in_env(Env *env, struct ASTNode *node, char *type_name)
{

    if (!env || !node || !type_name)
    {
        return 0;
    }

    // Detect if the node represents a function or variable
    int is_function = node->type == AST_DECL_FUNC;

    // Extract original member name
    char *member_name = is_function ? node->static_type : node->data.binary_op.left->static_type;

    char *name = is_function ? node->data.func_node.name : node->data.binary_op.left->data.var_name;

    // Build qualified name: _<type_name>_<name>
    char *qualified_name = generate_underscored_name(type_name, name);

    // Check if an item with this name and kind already exists
    EnvItem *existing = find_env_item_by_kind(env, qualified_name, is_function);
    if (existing)
    {
        existing->computed_type = is_function ? &TYPE_GENERIC : &TYPE_VOID;
        //free(qualified_name); // avoid memory leak
        return 0;
    }

    // Create new environment entry
    EnvItem *new_item = malloc(sizeof(EnvItem));
    new_item->usages = node;

    // Rename the node in AST to the qualified name
    if (is_function)
        new_item->usages->data.func_node.name = qualified_name;
    else
        new_item->usages->data.binary_op.left->data.var_name = qualified_name;

    // Try to determine its declared type
    Symbol *known_type = find_type_scopes(node->scope, member_name);

    if (known_type)
        new_item->computed_type = known_type->type;
    // else
    //     new_item->computed_type = NULL;

    // Insert at the head of the environment
    new_item->next = env->start;
    env->start = new_item;
    env->env_count++;

    return 1;
}

/// @brief Toma dos cadena A y B retorna una nuvea cadena en el formato _A_B
/// @param prefix
/// @param base
/// @return
char *generate_underscored_name(const char *prefix, const char *base)
{
    if (!prefix || !base)
        return NULL;

    if (base[0] == '_')
    {
        // Devuelve una copia de base para evitar efectos colaterales si se libera después
        char *copy = malloc(strlen(base) + 1);
        if (copy)
            strcpy(copy, base);
        return copy;
    }

    size_t total_length = strlen(prefix) + strlen(base) + 3; // "_", "_" y '\0'
    char *result = malloc(total_length);
    if (!result)
        return NULL;

    snprintf(result, total_length, "_%s_%s", prefix, base);
    return result;
}

/// @brief Busca un EnvItem* dentro de un ENv que contenga un node (usage) con cierto nombre y luego como se puede mejorar y renombrar
/// @param env
/// @param name
/// @param is_function
/// @return
EnvItem *find_env_item_by_kind(Env *env, const char *name, int is_function)
{
    if (!env)
        return NULL;

    EnvItem *curr = env->start;

    while (curr)
    {
        ASTNode *node = curr->usages;

        if (is_function && node->type == AST_DECL_FUNC &&
            strcmp(node->data.func_node.name, name) == 0)
        {
            return curr;
        }

        if (!is_function && node->type == AST_ASSIGNMENT &&
            strcmp(node->data.binary_op.left->data.var_name, name) == 0)
        {
            return curr;
        }

        curr = curr->next;
    }

    return NULL;
}

/// @brief Implementa una busqueda recursiva jerarquica de un miembro dentro de un entorno asociado a un tipo
/// @param context El entorno actual
/// @param full_name EL nombre del miembro
/// @param type TIpo en el que se esta buscando el miembro
/// @param is_function  Indica si se busca una funcion (1) o una variable (0)
/// @return
EnvItem *lookup_type_member_recursive(Env *context, const char *full_name, TypeValue *type, int is_function)
{
    if (!context || !type || !full_name)
        return NULL;

    // Intentar encontrar directamente en este contexto
    EnvItem *found = find_env_item_by_kind(context, full_name, is_function);
    if (found)
        return found;

    // Si no se encuentra, buscar en la clase padre
    if (type->super_type && type->super_type->def_node && type->super_type->def_node->env)
    {
        // Quitar el prefijo del tipo actual
        char *base_name = remove_type_prefix(full_name, type->name);
        if (!base_name)
            return NULL;

        // Crear nuevo nombre con el prefijo del padre
        char *parent_prefixed = generate_underscored_name(type->super_type->name, base_name);
        free(base_name);

        // Búsqueda recursiva
        EnvItem *parent_result = lookup_type_member_recursive(
            type->super_type->def_node->env, parent_prefixed, type->super_type, is_function);

        free(parent_prefixed);
        return parent_result;
    }

    return NULL;
}

/// @brief Toma un string que empieza con un prefijo y guiones basjo y elimina el prefijo ,devolvieno el sufijo
/// @param full_name
/// @param type_name
/// @return
char *remove_type_prefix(const char *full_name, const char *type_name)
{
    size_t prefix_len = strlen(type_name) + 2; // +2 for the leading and trailing underscores
    if (strlen(full_name) <= prefix_len)
        return NULL; // Invalid input

    // Verifica que el formato empiece como debe
    if (full_name[0] != '_' || strncmp(full_name + 1, type_name, strlen(type_name)) != 0 || full_name[strlen(type_name) + 1] != '_')
        return NULL;

    return strdup(full_name + prefix_len);
}

/// @brief Busca un elemento EnvItem por su nombre dentro de un entorno asociado a un tipo
/// @param env El entorno en donde buscar
/// @param name EL nombre del item a buscar
/// @param is_type 1 si esta buscando un tipo, 0 si esta buscando una func
/// @param var Si esta buscando una varaible (de asignacion)
/// @return Devuelve un ptro EnvItem* si encuentra o NULL
EnvItem *find_env_item(Env *env, char *name, int is_type, int var)
{
    fprintf(stderr, "Voy a buscar %s que es tipo: %d y es un var %d\n", name, is_type, var);

    if (!env)
    {
        fprintf(stderr, "CUBAAAA\n");
        return NULL;
    }

    EnvItem *current = env->start;
    int i = 0;
    while (i < env->env_count && current != NULL)
    {
        if ((!is_type && !var && current->usages && current->usages->type == AST_DECL_FUNC &&
             !strcmp(current->usages->data.func_node.name, name)) ||
            (is_type && !var && current->usages && current->usages->type == AST_TYPE &&
             !strcmp(current->usages->data.typeDef.name_type, name)) ||
            (var && current->usages && current->usages->type == AST_ASSIGNMENT &&
             !strcmp(current->usages->data.binary_op.left->data.var_name, name)))
        {
            fprintf(stderr, "CUBAAAA\n");
            return current;
        }
        current = current->next;
        i++;
    }

    if (env->parent)
    {
        return find_env_item(env->parent, name, is_type, var);
    }

    fprintf(stderr, "No esta en el entorno\n");
    return NULL;
}

/// @brief Crear un environment item nuevo con al func o el tipo
/// @param env
/// @param node
/// @return Retorna 0 si ya existe en el entorno, 1 si acaba de crearlo
int create_env_item(Env *env, ASTNode *node, char *name, int is_type_decl)
{
    // Check for null pointers
    if (!env || !node || !name)
    {
        return 0;
    }

    // buscamos el entorno anteriormente
    EnvItem *f = find_env_item(env, name, is_type_decl, 0);

    // si existe
    if (f)
    {
        f->computed_type = !is_type_decl ? &TYPE_GENERIC : &TYPE_VOID;
        return 0;
    }

    EnvItem *env_item = malloc(sizeof(EnvItem));
    if (!env_item)
    {
        return 0;
    }
    
    env_item->usages = node;

    // // busca el tipo declarado por el usuario del nodo en el scope
    // Symbol *defined_type = find_type_scopes(node->scope, node->static_type);

    // Si ya exsite ese tipo

    if (!is_type_decl)
    {
        Symbol* defined_type = find_type_scopes(node->scope,node->static_type);

        if (defined_type)
        {
            env_item->computed_type = defined_type->type;
        }
    }

    env_item->next = env->start;
    env->start = env_item;
    env->env_count++;

    return 1;
}