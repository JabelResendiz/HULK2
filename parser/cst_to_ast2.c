#include "cst_to_ast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

#define DEBUG(...) \
    fprintf(stderr,YELLOW "DEBUG:" RESET  __VA_ARGS__ )

#define ERROR(...) \
    fprintf(stderr,RED "ERROR:" RESET  __VA_ARGS__ )

#define ACCEPT(...) \
    fprintf(stderr,GREEN "ACCEPT:" RESET  __VA_ARGS__ )

#define FINISH(...) \
    fprintf(stderr,BLUE "FINISH:" RESET  __VA_ARGS__ )


ASTNode* conditional_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    if (!cst_node || strcmp(cst_node->symbol, "IfExpr") != 0) {
        DEBUG("conditional_to_ast - nodo inválido o no es IfExpr\n");
        return NULL;
    }
    DEBUG("conditional_to_ast - procesando IfExpr con %d hijos\n", cst_node->child_count);
    // IfExpr → IF LPAREN Expr RPAREN IfBody ElifList ELSE IfBody
    if (cst_node->child_count >= 8) {
        CSTNode* condition_node = cst_node->children[2];
        CSTNode* then_body_node = cst_node->children[4];
        CSTNode* elif_list_node = cst_node->children[5];
        CSTNode* else_body_node = cst_node->children[7];
        if (!condition_node || !then_body_node || !else_body_node) {
            DEBUG("conditional_to_ast - hijos nulos\n");
            return NULL;
        }
        DEBUG("conditional_to_ast - convirtiendo condición\n");
        ASTNode* condExpr = expr_to_ast(condition_node);
        if (!condExpr) {
            DEBUG("conditional_to_ast - error al convertir condición\n");
            return NULL;
        }
        DEBUG("conditional_to_ast - convirtiendo then body\n");
        ASTNode* thenStmt = if_body_to_ast(then_body_node);
        if (!thenStmt) {
            DEBUG("conditional_to_ast - error al convertir then body\n");
            return NULL;
        }
        ASTNode* thenExpr = thenStmt; // En C, no hay ExprStmt, usamos el ASTNode directamente
        // Procesar ElifList para construir la cadena de if-elif-else
        DEBUG("conditional_to_ast - procesando ElifList\n");
        ASTNode* elseStmt = if_body_to_ast(else_body_node);
        if (!elseStmt) {
            DEBUG("conditional_to_ast - error al convertir else body\n");
            return NULL;
        }
        ASTNode* elseExpr = elseStmt;
        // Procesar elif branches en orden inverso (anidando)
        if (elif_list_node && strcmp(elif_list_node->symbol, "ElifList") == 0 && elif_list_node->child_count > 0) {
            // Recursivamente anidar elifs en el else
            elseExpr = process_elif_list(elif_list_node, elseExpr);
        }
        DEBUG("conditional_to_ast - creando IfExpr principal\n");
        ASTNode* ifExpr = create_conditional_node(condExpr, thenExpr, elseExpr);
        return ifExpr;
    }
    DEBUG("conditional_to_ast - estructura inválida\n");
    return NULL;
}

ASTNode* let_in_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    if (!cst_node || strcmp(cst_node->symbol, "LetExpr") != 0) {
        DEBUG("let_in_to_ast - nodo inválido o no es LetExpr\n");
        return NULL;
    }
    DEBUG("let_in_to_ast - procesando LetExpr con %d hijos\n", cst_node->child_count);
    // LetExpr → LET VarBindingList IN LetBody
    if (cst_node->child_count >= 4) {
        CSTNode* var_binding_list_node = cst_node->children[1];
        CSTNode* let_body_node = cst_node->children[3];
        if (!var_binding_list_node || !let_body_node) {
            DEBUG("let_in_to_ast - hijos nulos\n");
            return NULL;
        }
        DEBUG("let_in_to_ast - procesando VarBindingList\n");
        // Procesar VarBindingList en un array de ASTNode* (bindings)
        int binding_count = 0;
        ASTNode** bindings = NULL;
        // Contar cuántos bindings hay
        CSTNode* current = var_binding_list_node;
        while (current && strcmp(current->symbol, "VarBindingList") == 0 && current->child_count > 0) {
            binding_count++;
            if (current->child_count > 1) {
                current = current->children[1]; // VarBindingListTail
            } else {
                break;
            }
        }
        if (binding_count == 0) {
            DEBUG("let_in_to_ast - no se encontraron bindings\n");
            return NULL;
        }
        bindings = malloc(sizeof(ASTNode*) * binding_count);
        int idx = 0;
        current = var_binding_list_node;
        while (current && strcmp(current->symbol, "VarBindingList") == 0 && current->child_count > 0) {
            CSTNode* var_binding = current->children[0];
            if (var_binding && strcmp(var_binding->symbol, "VarBinding") == 0) {
                extern ASTNode* assignment_to_ast(CSTNode*);
                ASTNode* var_decl = assignment_to_ast(var_binding);
                if (var_decl) {
                    bindings[idx++] = var_decl;
                }
            }
            if (current->child_count > 1) {
                current = current->children[1]; // VarBindingListTail
                if (current && strcmp(current->symbol, "VarBindingListTail") == 0 && current->child_count > 1) {
                    current = current->children[1]; // next VarBindingList
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        DEBUG("let_in_to_ast - convirtiendo LetBody\n");
        ASTNode* body = NULL;
        if (let_body_node) {
            if (strcmp(let_body_node->symbol, "Expr") == 0) {
                body = expr_to_ast(let_body_node);
            } else if (strcmp(let_body_node->symbol, "BlockStmt") == 0) {
                body = block_to_ast(let_body_node);
            } else if (strcmp(let_body_node->symbol, "WhileStmt") == 0) {
                extern ASTNode* while_to_ast(CSTNode*);
                body = while_to_ast(let_body_node);
            } else if (strcmp(let_body_node->symbol, "ForStmt") == 0) {
                extern ASTNode* for_to_ast(CSTNode*);
                body = for_to_ast(let_body_node);
            } else {
                DEBUG("let_in_to_ast - tipo de LetBody inesperado: %s\n", let_body_node->symbol);
                return NULL;
            }
        }
        if (!body) {
            DEBUG("let_in_to_ast - error al convertir LetBody\n");
            return NULL;
        }
        // Crear LetExpr anidados desde la última variable hacia la primera
        ASTNode* currentBody = body;
        for (int i = binding_count - 1; i >= 0; i--) {
            DEBUG("let_in_to_ast - creando LetExpr para binding %d\n", i);
            ASTNode* single_binding = bindings[i];
            // En este diseño, create_let_in_node espera un array de 1 binding
            ASTNode** single_binding_arr = &single_binding;
            currentBody = create_let_in_node(single_binding_arr, 1, currentBody);
        }
        free(bindings);
        return currentBody;
    }
    DEBUG("let_in_to_ast - estructura inválida\n");
    return NULL;
}


ASTNode* assignment_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("assignment tiene %d hijos\n", cst_node->child_count);

    if (!cst_node) {
        ERROR("assignment_to_ast: NULL node\n");
        return NULL;
    }

    // Verificar si es un VarBinding (usado en let-in)
    if (strcmp(cst_node->symbol, "VarBinding") == 0) {
        // VarBinding -> ID TypeAnnotation EQUALS Expr
        // children: 0=ID, 1=TypeAnnotation,2LS,3=Expr
        if (cst_node->child_count < 4) {
            ERROR("assignment_to_ast: Invalid VarBinding structure, expected 4 children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* id_node = cst_node->children[0];
        CSTNode* type_annotation_node = cst_node->children[1];
        CSTNode* expr_node = cst_node->children[3];
        
        char* var_name = id_node->symbol;
        char* type_name = "";
        
        // Extraer tipo si existe
        if (type_annotation_node && strcmp(type_annotation_node->symbol, "TypeAnnotation") == 0 && type_annotation_node->child_count > 1) {
            CSTNode* type_node = type_annotation_node->children[1]; // TypeAnnotation -> ID
            if (type_node) {
                type_name = type_node->symbol;
            }
        }
        
        ASTNode* value = expr_to_ast(expr_node);
        if (!value) {
            ERROR("assignment_to_ast: Failed to convert expression in VarBinding\n");
            return NULL;
        }
        
        ASTNode* assignment = create_assignment_node(var_name, value, type_name, NODE_ASSIGNMENT);
        FINISH("assignment_to_ast (VarBinding) completado exitosamente\n");
        return assignment;
    }
    
    // Verificar si es un simple_var_decl
    if (strcmp(cst_node->symbol, "simple_var_decl") == 0) {
        // simple_var_decl -> ID COLON ID EQUALS expression | ID EQUALS expression
        if (cst_node->child_count < 3) {
            ERROR("assignment_to_ast: Invalid simple_var_decl structure, expected at least 3 children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* id_node = cst_node->children[0];
        char* var_name = id_node->symbol;
        char* type_name = "";
        ASTNode* value = NULL;
        
        if (cst_node->child_count == 3) {
            // ID EQUALS expression
            CSTNode* expr_node = cst_node->children[2];
            value = expr_to_ast(expr_node);
        } else if (cst_node->child_count == 5) {
            // ID COLON ID EQUALS expression
            CSTNode* type_node = cst_node->children[2];
            CSTNode* expr_node = cst_node->children[4];
            type_name = type_node->symbol;
            value = expr_to_ast(expr_node);
        }
        
        if (!value) {
            ERROR("assignment_to_ast: Failed to convert expression in simple_var_decl\n");
            return NULL;
        }
        
        ASTNode* assignment = create_assignment_node(var_name, value, type_name, NODE_ASSIGNMENT);
        FINISH("assignment_to_ast (simple_var_decl) completado exitosamente\n");
        return assignment;
    }
    
    // Verificar si es un destructive_var_decl
    if (strcmp(cst_node->symbol, "destructive_var_decl") == 0) {
        // destructive_var_decl -> ID DEQUALS expression
        if (cst_node->child_count < 3) {
            ERROR("assignment_to_ast: Invalid destructive_var_decl structure, expected 3 children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* id_node = cst_node->children[0];
        CSTNode* expr_node = cst_node->children[2];
        
        char* var_name = id_node->symbol;
        ASTNode* value = expr_to_ast(expr_node);
        if (!value) {
            ERROR("assignment_to_ast: Failed to convert expression in destructive_var_decl\n");
            return NULL;
        }
        
        ASTNode* assignment = create_assignment_node(var_name, value, NODE_D_ASSIGNMENT);
        FINISH("assignment_to_ast (destructive_var_decl) completado exitosamente\n");
        return assignment;
    }
    
    // Verificar si es un destructive_member_decl
    if (strcmp(cst_node->symbol, "destructive_member_decl") == 0) {
        // destructive_member_decl -> expression DOT ID DEQUALS expression
        if (cst_node->child_count < 5) {
            ERROR("assignment_to_ast: Invalid destructive_member_decl structure, expected 5 children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* instance_node = cst_node->children[0];
        CSTNode* member_id_node = cst_node->children[2];
        CSTNode* value_node = cst_node->children[4];
        
        ASTNode* instance = expr_to_ast(instance_node);
        ASTNode* member = create_variable_node(member_id_node->symbol, 0);
        ASTNode* value = expr_to_ast(value_node);
        
        if (!instance || !member || !value) {
            ERROR("assignment_to_ast: Failed to convert parts of destructive_member_decl\n");
            return NULL;
        }
        
        ASTNode* setter = create_attr_setter_node(instance, member, value);
        FINISH("assignment_to_ast (destructive_member_decl) completado exitosamente\n");
        return setter;
    }
    
    // Si no es ninguno de los tipos conocidos, intentar procesar como expresión general
    ERROR("assignment_to_ast: Unknown assignment node type %s\n", cst_node->symbol);
    return NULL;
}


ASTNode* funCall_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("funCall tiene %d hijos\n", cst_node->child_count);

    if (!cst_node) {
        ERROR("funCall_to_ast: NULL node\n");
        return NULL;
    }

    // Verificar si es un function_call
    if (strcmp(cst_node->symbol, "function_call") == 0) {
        // function_call -> ID LPAREN list_args RPAREN
        // children:0, 1=LPAREN,2=list_args, 3=RPAREN
        if (cst_node->child_count < 4) {
            ERROR("funCall_to_ast: Invalid function_call structure, expected 4children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* id_node = cst_node->children[0];
        CSTNode* list_args_node = cst_node->children[2];
        
        char* func_name = id_node->symbol;
        ASTNode** args = NULL;
        int arg_count = 0;
        
        // Procesar list_args
        if (list_args_node && strcmp(list_args_node->symbol, "list_args") == 0) {
            // Contar argumentos
            int count = 0;
            CSTNode* current = list_args_node;
            while (current && strcmp(current->symbol, "list_args") == 0 && current->child_count > 0) {
                if (current->child_count >= 1) {
                    count++;
                }
                if (current->child_count > 2) {
                    current = current->children[2]; // list_args_not_empty
                } else {
                    break;
                }
            }
            
            args = malloc(sizeof(ASTNode*) * count);
            arg_count = 0;
            current = list_args_node;
            
            while (current && strcmp(current->symbol, "list_args") == 0 && current->child_count > 0) {
                if (current->child_count >= 1) {
                    CSTNode* expr_node = current->children[0];
                    ASTNode* arg = expr_to_ast(expr_node);
                    if (arg) {
                        args[arg_count++] = arg;
                    }
                }
                if (current->child_count > 2) {
                    current = current->children[2]; // list_args_not_empty
                } else {
                    break;
                }
            }
        }
        
        ASTNode* func_call = create_func_call_node(func_name, args, arg_count);
        FINISH("funCall_to_ast (function_call) completado exitosamente\n");
        return func_call;
    }
    
    // Verificar si es un member_call
    if (strcmp(cst_node->symbol, "member_call") == 0) {
        // member_call -> expression DOT ID | expression DOT function_call
        if (cst_node->child_count < 3) {
            ERROR("funCall_to_ast: Invalid member_call structure, expected at least 3children, got %d\n", cst_node->child_count);
            return NULL;
        }
        
        CSTNode* instance_node = cst_node->children[0];
        CSTNode* member_node = cst_node->children[2];
        
        ASTNode* instance = expr_to_ast(instance_node);
        if (!instance) {
            ERROR("funCall_to_ast: Failed to convert instance expression in member_call\n");
            return NULL;
        }
        
        ASTNode* member = NULL;
        if (strcmp(member_node->symbol, "ID") == 0) {
            // expression DOT ID
            member = create_variable_node(member_node->symbol, 0);
        } else if (strcmp(member_node->symbol, "function_call") == 0) {
            // expression DOT function_call
            member = funCall_to_ast(member_node);
        } else {
            ERROR("funCall_to_ast: Unexpected member type %s in member_call\n", member_node->symbol);
            return NULL;
        }
        
        if (!member) {
            ERROR("funCall_to_ast: Failed to convert member in member_call\n");
            return NULL;
        }
        
        ASTNode* getter = create_attr_getter_node(instance, member);
        FINISH("funCall_to_ast (member_call) completado exitosamente\n");
        return getter;
    }
    
    // Si no es ninguno de los tipos conocidos
    ERROR("funCall_to_ast: Unknown function call node type %s\n", cst_node->symbol);
    return NULL;
}


ASTNode* while_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    if (!cst_node || strcmp(cst_node->symbol, "WhileStmt") != 0) {
        DEBUG("while_to_ast - nodo inválido o no es WhileStmt\n");
        return NULL;
    }
    DEBUG("while_to_ast - procesando WhileStmt con %d hijos\n", cst_node->child_count);
    if (cst_node->child_count >= 3) {
        CSTNode* condition_node = cst_node->children[1];
        CSTNode* while_body_node = cst_node->children[2];
        if (!condition_node || !while_body_node) {
            DEBUG("while_to_ast - hijos nulos\n");
            return NULL;
        }
        DEBUG("while_to_ast - convirtiendo condición\n");
        ASTNode* condExpr = expr_to_ast(condition_node);
        if (!condExpr) {
            DEBUG("while_to_ast - error al convertir condición\n");
            return NULL;
        }
        DEBUG("while_to_ast - convirtiendo while body\n");
        ASTNode* bodyStmt = NULL;
        if (strcmp(while_body_node->symbol, "BlockStmt") == 0) {
            bodyStmt = block_to_ast(while_body_node);
        } else if (strcmp(while_body_node->symbol, "Expr") == 0) {
            bodyStmt = expr_to_ast(while_body_node);
        } else {
            DEBUG("while_to_ast - tipo de cuerpo inesperado: %s\n", while_body_node->symbol);
            return NULL;
        }
        if (!bodyStmt) {
            DEBUG("while_to_ast - error al convertir while body\n");
            return NULL;
        }
        DEBUG("while_to_ast - creando nodo AST While\n");
        ASTNode* while_ast = create_loop_node(condExpr, bodyStmt);
        return while_ast;
    }
    DEBUG("while_to_ast - estructura inválida\n");
    return NULL;
}

//AUXILIARES

// Helper function to process ElifList (recursively nests elifs into else branch)
ASTNode* process_elif_list(CSTNode* elif_list_node, ASTNode* else_body)
{
    ACCEPT("process_elif_list: procesando %s\n", elif_list_node->symbol);
    if (!elif_list_node || elif_list_node->child_count == 0) {
        return else_body;
    }
    // ElifList -> ElifBranch ElifList | ε
    // ElifBranch -> ELIF LPAREN Expr RPAREN IfBody
    CSTNode* elif_branch = elif_list_node->children[0];
    if (strcmp(elif_branch->symbol, "ElifBranch") == 0 && elif_branch->child_count >= 5) {
        // Extract elif condition (child[2] should be Expr)
        CSTNode* elif_condition_node = elif_branch->children[2];
        ASTNode* elif_condition = expr_to_ast(elif_condition_node);
        if (!elif_condition) {
            ERROR("process_elif_list: Failed to convert elif condition\n");
            return else_body;
        }
        // Extract elif body (child[4] should be IfBody)
        CSTNode* elif_body_node = elif_branch->children[4];
        ASTNode* elif_body = if_body_to_ast(elif_body_node);
        if (!elif_body) {
            ERROR("process_elif_list: Failed to convert elif body\n");
            return else_body;
        }
        // Recursively process remaining elifs
        ASTNode* nested_else = else_body;
        if (elif_list_node->child_count > 1) {
            CSTNode* remaining_elif_list = elif_list_node->children[1];
            nested_else = process_elif_list(remaining_elif_list, else_body);
        }
        // Create nested conditional: if (elif_condition) elif_body else nested_else
        return create_conditional_node(elif_condition, elif_body, nested_else);
    }
    return else_body;
}

// Helper function to process IfBody (can be BlockStmt or Expr)
ASTNode* if_body_to_ast(CSTNode* cst_node)
{
    ACCEPT("if_body_to_ast: procesando %s\n", cst_node->symbol);
    if (!cst_node) {
        ERROR("if_body_to_ast: NULL node\n");
        return NULL;
    }
    if (strcmp(cst_node->symbol, "BlockStmt") == 0) {
        DEBUG("if_body_to_ast: procesando BlockStmt\n");
        return block_to_ast(cst_node);
    } else if (strcmp(cst_node->symbol, "Expr") == 0) {
        DEBUG("if_body_to_ast: procesando Expr\n");
        return expr_to_ast(cst_node);
    } else {
        ERROR("if_body_to_ast: Unexpected node type %s\n", cst_node->symbol);
        return NULL;
    }
}