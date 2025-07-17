#include "../cst_to_ast.h"

ASTNode *typeDef_to_ast(CSTNode *cst_node)
{
    if (!cst_node || strcmp(cst_node->symbol, "TypeDef") != 0)
    {
        return NULL;
    }

    // Extraer componentes básicos
    char *type_name = NULL;
    char *parent_name = NULL;
    ASTNode **params = NULL;
    int param_count = 0;
    ASTNode **p_params = NULL;
    int p_param_count = 0;
    ASTNode *body = NULL;
    int p_constructor = 0;

    // Procesar hijos del TypeDef
    for (int i = 0; i < cst_node->child_count; i++)
    {
        CSTNode *child = cst_node->children[i];
        if (!child)
            continue;

        if (!strcmp(child->symbol, "ID") && child->token)
        {
            // Primer ID es el nombre del tipo, segundo ID es el padre
            if (!type_name)
            {
                type_name = strdup(child->token->value);
            }
            else if (!parent_name)
            {
                parent_name = strdup(child->token->value);
            }
        }
        else if (!strcmp(child->symbol, "TypeParams"))
        {
            // Procesar parámetros del tipo
            for (int j = 0; j < child->child_count; j++)
            {
                CSTNode *param_child = child->children[j];
                if (param_child && strcmp(param_child->symbol, "ArgIdList") == 0)
                {
                    // Extraer parámetros de ArgIdList
                    for (int k = 0; k < param_child->child_count; k++)
                    {
                        CSTNode *arg = param_child->children[k];
                        if (arg && strcmp(arg->symbol, "ArgId") == 0)
                        {
                            ASTNode *param = variable_to_ast(arg);
                            if (param)
                            {
                                params = realloc(params, (param_count + 1) * sizeof(ASTNode *));
                                params[param_count++] = param;
                            }
                        }
                    }
                }
            }
        }
        else if (!strcmp(child->symbol, "TypeInheritance"))
        {
            // Procesar herencia
            for (int j = 0; j < child->child_count; j++)
            {
                CSTNode *inherit_child = child->children[j];
                if (inherit_child && strcmp(inherit_child->symbol, "TypeBaseArgs") == 0)
                {
                    // Procesar argumentos del padre
                    for (int k = 0; k < inherit_child->child_count; k++)
                    {
                        CSTNode *arg = inherit_child->children[k];
                        if (arg && strcmp(arg->symbol, "ArgList") == 0)
                        {
                            for (int l = 0; l < arg->child_count; l++)
                            {
                                CSTNode *expr = arg->children[l];
                                if (expr && strcmp(expr->symbol, "Expr") == 0)
                                {
                                    ASTNode *p_param = expr_to_ast(expr);
                                    if (p_param)
                                    {
                                        p_params = realloc(p_params, (p_param_count + 1) * sizeof(ASTNode *));
                                        p_params[p_param_count++] = p_param;
                                    }
                                }
                            }
                            p_constructor = 1;
                        }
                    }
                }
            }
        }
        else if (!strcmp(child->symbol, "TypeBody"))
        {
            printf("DEBUG: Procesando TypeBody con %d hijos\n", child->child_count);
            // Procesar cuerpo del tipo - buscar recursivamente los TypeMember
            ASTNode **members = NULL;
            int member_count = 0;

            // Recursivamente buscar todos los TypeMember en el TypeBody
            for (int j = 0; j < child->child_count; j++)
            {
                CSTNode *member_child = child->children[j];
                if (member_child && strcmp(member_child->symbol, "TypeMember") == 0)
                {
                    printf("DEBUG: Encontrado TypeMember\n");
                    // Procesar cada TypeMember como una declaración de función
                    ASTNode *member = process_type_member(member_child);
                    if (member)
                    {
                        printf("DEBUG: TypeMember procesado exitosamente\n");
                        members = realloc(members, (member_count + 1) * sizeof(ASTNode *));
                        members[member_count++] = member;
                    }
                    else
                    {
                        printf("DEBUG: TypeMember falló al procesarse\n");
                    }
                }
                else if (member_child)
                {
                    printf("DEBUG: Hijo de TypeBody: %s\n", member_child->symbol);
                }
            }

            printf("DEBUG: Total de miembros procesados: %d\n", member_count);
            // Crear un nodo bloque con los miembros
            if (member_count > 0)
            {
                body = create_program_node(members, member_count, NODE_BLOCK);
                printf("DEBUG: Cuerpo del tipo creado exitosamente\n");
            }
            else
            {
                printf("DEBUG: No se encontraron miembros válidos\n");
            }
        }
    }

    // Validar componentes mínimos
    if (!type_name || !body)
    {
        // Limpiar memoria en caso de error
        if (type_name)
            free(type_name);
        if (parent_name)
            free(parent_name);
        if (params)
            free(params);
        if (p_params)
            free(p_params);
        return NULL;
    }

    // Crear nodo AST
    ASTNode *result = create_type_dec_node(
        type_name, params, param_count,
        parent_name, p_params, p_param_count,
        body, p_constructor);

    // Limpiar arrays temporales
    if (params)
        free(params);
    if (p_params)
        free(p_params);

    return result;
}

// Función auxiliar para procesar un TypeMember
ASTNode *process_type_member(CSTNode *member_node)
{
    if (!member_node || strcmp(member_node->symbol, "TypeMember") != 0)
    {
        return NULL;
    }

    char *member_name = NULL;
    ASTNode **params = NULL;
    int param_count = 0;
    ASTNode *body = NULL;
    char *ret_type = NULL;

    // Procesar hijos del TypeMember
    for (int i = 0; i < member_node->child_count; i++)
    {
        CSTNode *child = member_node->children[i];
        if (!child)
            continue;

        if (!strcmp(child->symbol, "ID") && child->token)
        {
            member_name = strdup(child->token->value);
        }
        else if (!strcmp(child->symbol, "TypeMemberTail"))
        {
            // Procesar TypeMemberTail para determinar si es método o atributo
            for (int j = 0; j < child->child_count; j++)
            {
                CSTNode *tail_child = child->children[j];
                if (!tail_child)
                    continue;

                if (!strcmp(tail_child->symbol, "TypeMemberTail'"))
                {
                    // Procesar TypeMemberTail' para determinar si es método o atributo
                    for (int k = 0; k < tail_child->child_count; k++)
                    {
                        CSTNode *tail_prime_child = tail_child->children[k];
                        if (!tail_prime_child)
                            continue;

                        if (!strcmp(tail_prime_child->symbol, "LPAREN"))
                        {
                            // Es un método - buscar parámetros y cuerpo
                            for (int l = k + 1; l < tail_child->child_count; l++)
                            {
                                CSTNode *method_part = tail_child->children[l];
                                if (!method_part)
                                    continue;

                                if (!strcmp(method_part->symbol, "ArgIdList"))
                                {
                                    // Procesar parámetros
                                    for (int m = 0; m < method_part->child_count; m++)
                                    {
                                        CSTNode *arg = method_part->children[m];
                                        if (arg && strcmp(arg->symbol, "ArgId") == 0)
                                        {
                                            ASTNode *param = variable_to_ast(arg);
                                            if (param)
                                            {
                                                params = realloc(params, (param_count + 1) * sizeof(ASTNode *));
                                                params[param_count++] = param;
                                            }
                                        }
                                    }
                                }
                                else if (!strcmp(method_part->symbol, "FunctionBody"))
                                {
                                    // Procesar cuerpo del método
                                    body = process_function_body(method_part);
                                }
                            }
                        }
                        else if (!strcmp(tail_prime_child->symbol, "EQUALS"))
                        {
                            // Es un atributo - buscar la expresión
                            for (int l = k + 1; l < tail_child->child_count; l++)
                            {
                                CSTNode *attr_part = tail_child->children[l];
                                if (!attr_part)
                                    continue;

                                if (!strcmp(attr_part->symbol, "Expr"))
                                {
                                    body = expr_to_ast(attr_part);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!member_name)
    {
        // Limpiar memoria en caso de error
        if (params)
            free(params);
        return NULL;
    }

    // Crear nodo de función para el miembro
    ASTNode *result = create_func_dec_node(member_name, params, param_count, body, ret_type);

    // Limpiar array temporal
    if (params)
        free(params);

    return result;
}

// Función auxiliar para procesar FunctionBody
ASTNode *process_function_body(CSTNode *body_node)
{
    if (!body_node || strcmp(body_node->symbol, "FunctionBody") != 0)
    {
        return NULL;
    }

    for (int i = 0; i < body_node->child_count; i++)
    {
        CSTNode *child = body_node->children[i];
        if (!child)
            continue;

        if (!strcmp(child->symbol, "FunctionBodyExpr"))
        {
            return expr_to_ast(child);
        }
        else if (!strcmp(child->symbol, "BlockStmt"))
        {
            return block_to_ast(child);
        }
    }

    return NULL;
}
