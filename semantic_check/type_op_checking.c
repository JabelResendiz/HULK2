#include "semantic.h"

// method to visit downcasting node
void visit_casting_type(Visitor *v, ASTNode *node)
{
    ASTNode *exp = node->data.cast_test.exp;
    char *type_name = node->data.cast_test.type_name;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);
    Type *dynamic_type = get_type(exp);
    Symbol *defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type)
    {
        ContextItem *item = find_context_item(
            node->context, type_name, 1, 0);

        if (item)
        {
            accept(v, item->declaration);
            defined_type = find_defined_type(node->scope, type_name);
        }

        if (!defined_type)
        {
            node->data.cast_test.type = &TYPE_ERROR;
            node->return_type = &TYPE_ERROR;
            node->dynamic_type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' is not a valid type. Line: %d.",
                type_name, node->line);
            return;
        }
    }

    // Checking both types are in the same branch of type hierarchy
    if (
        !type_equals(dynamic_type, &TYPE_ERROR) &&
        !type_equals(dynamic_type, &TYPE_ANY) &&
        !same_branch_in_type_hierarchy(
            dynamic_type, defined_type->type))
    {
        report_error(
            v, "Type '%s' can not be downcasted to type '%s'. Line: %d.",
            dynamic_type->name, type_name, node->line);
    }

    node->data.cast_test.type = defined_type->type;
    node->return_type = defined_type->type;
    node->dynamic_type = defined_type->type;
}

// method to visit type-testing node
void visit_test_type(Visitor *v, ASTNode *node)
{
    ASTNode *exp = node->data.cast_test.exp;
    char *type_name = node->data.cast_test.type_name;

    exp->scope->parent = node->scope;
    exp->context->parent = node->context;

    accept(v, exp);

    Symbol *defined_type = find_defined_type(node->scope, type_name);

    if (!defined_type)
    {
        ContextItem *item = find_context_item(
            node->context, type_name, 1, 0);

        if (item)
        {
            accept(v, item->declaration);
            defined_type = find_defined_type(node->scope, type_name);
        }

        if (!defined_type)
        {
            node->data.cast_test.type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' is not a valid type. Line: %d.",
                type_name, node->line);
            return;
        }
    }

    node->data.cast_test.type = defined_type->type;
}

// method to visit attribute or method getter node
void visit_attr_getter(Visitor *v, ASTNode *node)
{

    fprintf(stderr, "ESTAMOS EN EL GETTER DE UNA FUNCION\n");

    ASTNode *instance = node->data.op_node.left;
    ASTNode *member = node->data.op_node.right;

    instance->scope->parent = node->scope;
    instance->context->parent = node->context;
    member->context->parent = node->context;
    member->scope->parent = node->scope;

    accept(v, instance);
    Type *instance_type = instance->dynamic_type;

    fprintf(stderr, "Instance_type es de tipo %s\n", instance_type->name);

    if (type_equals(instance_type, &TYPE_ERROR))
    {
        fprintf(stderr, "INSTANCE_TYPE ES UN ERROR\n");

        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        return;
    }
    else if (type_equals(instance_type, &TYPE_ANY))
    {

        fprintf(stderr, "INSTANCE_TYPE ES UN GENERIC\n");

        if (!unify_type_by_attr(v, node))
        {
            node->return_type = &TYPE_ERROR;
            node->dynamic_type = &TYPE_ERROR;
            return;
        }
        else
        {
            accept(v, instance); // visit again if unified
            instance_type = get_type(instance);
        }
    }

    fprintf(stderr, "TODO BIEN\n");

    // attributes are private, so they only can be gotten using self
    if (member->type == NODE_VARIABLE && ((
                                              instance->type == NODE_VARIABLE &&
                                              strcmp(instance->data.variable_name, "self")) ||
                                          instance->type != NODE_VARIABLE))
    {
        report_error(
            v, "Impossible to access to '%s' in type '%s' because all"
               " attributes are private. Line: %d.",
            member->data.variable_name, instance_type->name, node->line);
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        return;
    }
    else if (member->type == NODE_VARIABLE)
    {
        member->data.variable_name = concat_str_with_underscore(
            instance_type->name, member->data.variable_name);

        Symbol *sym = find_type_attr(
            instance_type,
            member->data.variable_name);

        if (sym)
        {
            member->return_type = sym->type;
            member->dynamic_type = sym->dyn;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        }
        else
        {
            ContextItem *item = find_item_in_type_hierarchy(
                instance_type->dec->context,
                member->data.variable_name,
                instance_type, 0);

            if (!item)
            {
                member->return_type = &TYPE_ERROR;
                member->dynamic_type = &TYPE_ERROR;
                report_error(
                    v, "Type '%s' does not have an attribute named '%s'. Line: %d",
                    instance_type->name, member->data.variable_name, node->line);
            }
            else
            {
                accept(v, item->declaration);
                sym = find_type_attr(instance_type, member->data.variable_name);
                member->return_type = sym->type;
                member->dynamic_type = sym->dyn;
                member->is_param = sym->is_param;
                member->derivations = sym->derivations;
            }
        }
    }

    if (member->type == NODE_FUNC_CALL && !type_equals(instance_type, &TYPE_ERROR))
    {
        Symbol *t = find_defined_type(node->scope, instance_type->name);

        if (!t)
        {
            ContextItem *t_item = find_context_item(node->context, instance_type->name, 1, 0);
            if (t_item)
            {
                accept(v, t_item->declaration);
                instance_type = t_item->return_type;
            }
            else
            {
                node->return_type = &TYPE_ERROR;
                if (!type_equals(instance_type, &TYPE_ERROR))
                {
                    report_error(
                        v, "Type '%s' does not have a method named '%s'. Line: %d",
                        instance_type->name, member->data.func_node.name, node->line);
                }
                return;
            }
        }
        else if (is_builtin_type(instance_type))
        {
            node->return_type = &TYPE_ERROR;
            node->dynamic_type = &TYPE_ERROR;
            if (!type_equals(instance_type, &TYPE_ERROR))
            {
                report_error(
                    v, "Type '%s' does not have a method named '%s'. Line: %d",
                    instance_type->name, member->data.func_node.name, node->line);
            }
            return;
        }
        else
        {
            instance_type = t->type;
        }

        member->data.func_node.name = concat_str_with_underscore(
            instance_type->name, member->data.func_node.name);
        check_function_call(v, member, instance_type);
    }

    node->return_type = get_type(member);
    node->dynamic_type = get_type(member);

    // if(node->derivations== NULL)
    // {
    //     fprintf(stderr,"es completamente nulo");
    //     exit(1);
    // }

    fprintf(stderr, "1223\n");
    // if (!node)
    // {
    //     fprintf(stderr, "Error: node es NULL\n");
    //     exit(1);
    // }
    // if (!node->derivations)
    // {
    //     fprintf(stderr, "Error: node->derivations es NULL\n");
    //     exit(1);
    // }
    // exit(1);

    node->derivations = add_node_list(member, node->derivations);
}

// method to visit attribute setter node
void visit_attr_setter(Visitor *v, ASTNode *node)
{

    fprintf(stderr, "ESTAMOS EN EL SETTER DE UNA FUNCION\n");

    ASTNode *instance = node->data.cond_node.cond;
    ASTNode *member = node->data.cond_node.body_true;
    ASTNode *value = node->data.cond_node.body_false;

    value->scope->parent = node->scope;
    value->context->parent = node->context;
    instance->scope->parent = node->scope;
    instance->context->parent = node->context;
    member->context->parent = node->context;
    member->scope->parent = node->scope;

    accept(v, instance);
    Type *instance_type = instance->dynamic_type;

    if (type_equals(instance_type, &TYPE_ERROR))
    {
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        return;
    }

    // attributes can only be set using self
    if ((instance->type == NODE_VARIABLE &&
         strcmp(instance->data.variable_name, "self")) ||
        instance->type != NODE_VARIABLE)
    {
        report_error(
            v, "Impossible to access to '%s'  in type '%s' because all"
               " attributes are private. Line: %d.",
            member->data.variable_name, instance_type->name, node->line);
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        return;
    }

    member->data.variable_name = concat_str_with_underscore(
        instance_type->name, member->data.variable_name);

    Symbol *sym = find_type_attr(
        instance_type,
        member->data.variable_name);

    fprintf(stderr, "TODO BIEN EN EL SETTER 1\n");

    if (sym)
    {
        fprintf(stderr, "ESTA EN EL SCOPE\n");
        member->return_type = sym->type;
        member->dynamic_type = sym->dyn;
        member->is_param = sym->is_param;
        member->derivations = sym->derivations;
    }
    else
    {
        ContextItem *item = find_item_in_type_hierarchy(
            instance_type->dec->context,
            member->data.variable_name,
            instance_type, 0);

        if (!item)
        {
            member->return_type = &TYPE_ERROR;
            member->dynamic_type = &TYPE_ERROR;
            report_error(
                v, "Type '%s' does not have an attribute named '%s'. Line: %d",
                instance_type->name, member->data.variable_name, node->line);
        }
        else
        {
            accept(v, item->declaration);
            sym = find_type_attr(instance_type, member->data.variable_name);
            member->return_type = sym->type;
            member->dynamic_type = sym->dyn;
            member->is_param = sym->is_param;
            member->derivations = sym->derivations;
        }
    }

    accept(v, value);

    Type *inferried_type = get_type(value);

    // checking whether or not the new type is consistent with the initialization type
    if (sym)
    {
        if (
            is_ancestor_type(sym->type, inferried_type) ||
            type_equals(inferried_type, &TYPE_ANY) ||
            type_equals(sym->type, &TYPE_ANY))
        {
            if (type_equals(inferried_type, &TYPE_ANY) &&
                !type_equals(sym->type, &TYPE_ANY))
            {
                if (unify(v, value, sym->type))
                {
                    accept(v, value);
                    inferried_type = sym->type;
                }
            }
            else if (
                type_equals(sym->type, &TYPE_ANY) &&
                !type_equals(inferried_type, &TYPE_ANY))
            {
                sym->type = inferried_type;
                for (int i = 0; i < sym->derivations->count; i++)
                {
                    ASTNode *value = at(i, sym->derivations);
                    if (value && type_equals(value->return_type, &TYPE_ANY))
                    {
                        unify(v, value, inferried_type);
                    }
                }
            }

            sym->derivations = add_node_list(value, sym->derivations);
        }
        else
        {
            report_error(
                v, "Variable '%s' was initializated as "
                   "'%s', but reassigned as '%s'. Line: %d.",
                sym->name, sym->type->name, inferried_type->name, node->line);
        }
    }

    node->return_type = get_type(value);
    node->dynamic_type = get_type(value);
    node->derivations = add_node_list(value, node->derivations);
    node->derivations = add_node_list(member, node->derivations);
}

// // method to visit base function
// void visit_base_func(Visitor *v, ASTNode *node)
// {
//     ASTNode *args = node->data.func_node.args;
//     char *current_func = v->current_function;
//     Type *current_type = v->current_type;

//     if (!current_func)
//     {
//         node->return_type = &TYPE_ERROR;
//         node->dynamic_type = &TYPE_ERROR;
//         report_error(
//             v, "Keyword 'base' only can be used when referring to an ancestor"
//                " implementation of a function. Line: %d.",
//             node->line);
//         return;
//     }

//     char *f_name = find_base_func_dec(current_type, current_func);

//     if (!f_name)
//     {
//         if (!is_builtin_type(current_type->parent))
//         {
//             ContextItem *item = find_item_in_type_hierarchy(
//                 current_type->parent->dec->context, current_func, current_type->parent, 1);

//             if (item)
//             {
//                 accept(v, item->declaration);
//                 f_name = item->declaration->data.func_node.name;
//             }
//         }

//         if (!f_name)
//         {
//             node->return_type = &TYPE_ERROR;
//             node->dynamic_type = &TYPE_ERROR;
//             report_error(
//                 v, "No ancestor of type '%s' has a definition for '%s'. Line: %d.",
//                 current_type->name,
//                 delete_underscore_from_str(current_func, current_type->name), node->line);
//             return;
//         }
//     }

//     // helper node that can be checked as a function call
//     ASTNode *call = create_func_call_node(f_name, args, node->data.func_node.arg_count);
//     call->context->parent = node->context;
//     call->scope->parent = node->scope;
//     call->line = node->line;

//     check_function_call(v, call, current_type->parent);
//     node->derivations = add_node_list(call, node->derivations);
//     node->return_type = &TYPE_ERROR;
//     node->dynamic_type = &TYPE_ERROR;
//     report_error(
//         v, "Unfortunately, 'base' call is not available in this version of the compiler. Line: %d.",
//         node->line);
//     node->data.func_node.name = f_name;
// }

void visit_base_func(Visitor *v, ASTNode *node)
{
    ASTNode *base_call_args = node->data.func_node.args;

    unsigned int base_call_arg_count = node->data.func_node.arg_count;

    // v->current_function debe contener el nombre completo del método actual (ej. "_Knight_name")
    // v->current_type debe contener la información del tipo de la clase actual (ej. Knight)
    char *current_method_llvm_name = v->current_function;
    Type *current_class_type = v->current_type;

    // 1. Verificaciones iniciales: ¿Estamos dentro de un método?
    if (!current_method_llvm_name || !current_class_type || !current_class_type->parent)
    {
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        report_error(v, "La palabra clave 'base' solo se puede usar dentro de un método en una clase que herede de otra. Línea: %d.", node->line);
        return;
    }

    // Extraer el nombre "original" del método (sin prefijo de clase, ej. "name")
    // Asumiendo que tus nombres de funciones LLVM son como "_ClassName_MethodName"
    char *original_method_name = NULL;
    // Debes tener una forma de mapear de _ClassName_MethodName a MethodName
    // Una opción es que current_method_info ya contenga el nombre original
    // o que lo pases al nodo AST de 'base' al parsearlo.
    // Por simplicidad aquí, asumiremos que node->data.func_node.name ya tiene el nombre "name" (sin prefijo)
    // Si no es así, necesitarías extraerlo de current_method_llvm_name (ej. buscar el último '_').
    original_method_name = node->data.func_node.name; // Asumiendo que el parser lo establece.

    if (!original_method_name)
    {
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        report_error(v, "Error interno: No se pudo determinar el nombre original del método para la llamada a 'base'. Línea: %d.", node->line);
        return;
    }

     // 2. Encontrar el método correspondiente en la clase padre
    Type *parent_type = current_class_type->parent;
    ContextItem *base_method_item = find_item_in_type_hierarchy(parent_type->dec->context, original_method_name, parent_type, 1);

    // if (!base_method_item || base_method_item->kind != CONTEXT_FUNCTION)
    // {
    //     node->return_type = &TYPE_ERROR;
    //     node->dynamic_type = &TYPE_ERROR;
    //     report_error(v, "No se encontró un método '%s' en la jerarquía de la clase padre '%s'. Línea: %d.",
    //                  original_method_name, parent_type->name, node->line);
    //     return;
    // }

    ASTNode *base_method_declaration_node = base_method_item->declaration;
    // Asegúrate de que el nodo de declaración del método base ha sido visitado
    // para que sus tipos de parámetros y retorno estén resueltos.
    accept(v, base_method_declaration_node); // Esto resuelve el return_type del método base

    // 3. Validar los argumentos pasados a 'base()' contra la firma del método base
    // Vamos a crear un "nodo de llamada simulado" para reusar check_function_call.
    ASTNode *simulated_call_node = create_func_call_node(original_method_name, base_call_args, base_call_arg_count);
    simulated_call_node->context = node->context; // Usa el contexto actual de la llamada a base
    simulated_call_node->scope = node->scope;
    simulated_call_node->line = node->line;
    // La clave es pasar el tipo del *padre* para la verificación de la función.
    check_function_call(v, simulated_call_node, parent_type); // Ahora check_function_call sabe que buscar en 'parent_type'

    // Si check_function_call encuentra un error, ya lo reportará y el return_type de simulated_call_node será TYPE_ERROR.
    if (simulated_call_node->return_type == &TYPE_ERROR) {
        node->return_type = &TYPE_ERROR;
        node->dynamic_type = &TYPE_ERROR;
        free_ast(simulated_call_node); // Libera el nodo temporal
        return;
    }

    // 4. Establecer el tipo de retorno del nodo 'base'
    node->return_type = simulated_call_node->return_type;
    node->dynamic_type = simulated_call_node->dynamic_type;

    // Guardar el nombre de la función LLVM del método base para la generación de código.
    // Esto es crucial para que codegen_base_func_call sepa qué función LLVM llamar directamente.
    // Puedes almacenar esto en algún campo de 'node->data.func_node'
    //node->data.func_node.llvm_func_name = base_method_declaration_node->data.func_node.llvm_func_name;

    free_ast(simulated_call_node); // Libera el nodo temporal
    // Ya no reportamos un error aquí. Si llegamos hasta aquí, la base call es semánticamente válida.
}