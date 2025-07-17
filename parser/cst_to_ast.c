#include "cst_to_ast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG(...) \
    fprintf(stderr, YELLOW "DEBUG:" RESET __VA_ARGS__)

#define ERROR(...) \
    fprintf(stderr, RED "ERROR:" RESET __VA_ARGS__)

#define ACCEPT(...) \
    fprintf(stderr, GREEN "ACCEPT:" RESET __VA_ARGS__)

#define FINISH(...) \
    fprintf(stderr, BLUE "FINISH:" RESET __VA_ARGS__)

// ============================================================================
// PROTOTIPOS DE FUNCIONES
// ============================================================================

ASTNode* for_to_ast(CSTNode* cst_node);
ASTNode* process_for_body(CSTNode* cst_node);
ASTNode* process_if_body(CSTNode* cst_node);
ASTNode* process_while_body(CSTNode* cst_node);

// ============================================================================
// FUNCIONES PRINCIPALES
// ============================================================================

ASTNode* cst_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del tipo es %s\n", cst_node->symbol);

    if (!cst_node || strcmp(cst_node->symbol, "Program")) {
        ERROR("CST root must be a program node\n");
        return NULL;
    }

    ASTNode* root = NULL;
    DEBUG("El programa tiene %d hijos\n", cst_node->child_count);

    if (cst_node->child_count > 0) {
        DEBUG("Primer hijo es %s\n", cst_node->children[0]->symbol);
        root = stmtList_to_ast(cst_node->children[0]);
        if (root) {
            DEBUG("AST creado exitosamente\n");
        } else {
            ERROR("Error al crear AST - stmtList_to_ast retornó NULL\n");
            DEBUG("Hijos del Program:");
            for (int i = 0; i < cst_node->child_count; i++) {
                if (cst_node->children[i]) {
                    DEBUG(" %s", cst_node->children[i]->symbol);
                } else {
                    DEBUG(" NULL");
                }
            }
            DEBUG("\n");
        }
    } else {
        DEBUG("Programa sin hijos\n");
    }

    FINISH("EL cst_to_ast termina exitosamente\n");
    return root;
}

ASTNode* stmtList_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("el StmtList tiene %d hijos\n", cst_node->child_count);

    // Crear lista de statements
    ASTNode** statements = NULL;
    int stmt_count = 0;

    for (int i = 0; i < cst_node->child_count; i++) {
        if (!cst_node->children || !cst_node->children[i]) {
            DEBUG("Hijo %d es NULL, saltando...\n", i);
            continue;
        }
        
        DEBUG("el nombre de mi hijo %d es %s\n", i, cst_node->children[i]->symbol);
        CSTNode* child = cst_node->children[i];

        if (child && !strcmp(child->symbol, "TerminatedStmt")) {
            ASTNode* stmt = terminated_stmt_to_ast(child);
            if (stmt) {
                statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                statements[stmt_count++] = stmt;
                DEBUG("TerminatedStmt %d procesado\n", stmt_count);
            } else {
                DEBUG("terminated_stmt_to_ast retornó NULL para hijo %d\n", i);
            }
        } else if (child && !strcmp(child->symbol, "StmtList")) {
            // Recursión para manejar listas anidadas
            ASTNode* nested_list = stmtList_to_ast(child);
            if (nested_list && nested_list->type == NODE_PROGRAM) {
                // Agregar todos los statements de la lista anidada
                for (int j = 0; j < nested_list->data.program_node.count; j++) {
                    statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                    statements[stmt_count++] = nested_list->data.program_node.statements[j];
                }
                DEBUG("StmtList anidado procesado con %d statements\n", nested_list->data.program_node.count);
            } else if (nested_list) {
                // Si el nested_list no es un programa, agregarlo como un statement
                statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                statements[stmt_count++] = nested_list;
                DEBUG("StmtList anidado agregado como statement individual\n");
            } else {
                DEBUG("StmtList anidado retornó NULL\n");
            }
        } else if (child && !strcmp(child->symbol, "ε")) {
            // Ignorar epsilon
            DEBUG("Ignorando nodo epsilon\n");
            continue;
        } else if (child) {
            // Intentar procesar otros tipos de statements directamente
            DEBUG("Intentando procesar statement directo: %s\n", child->symbol);
            ASTNode* stmt = stmt_to_ast(child);
            if (stmt) {
                statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                statements[stmt_count++] = stmt;
                DEBUG("Statement directo procesado: %s\n", child->symbol);
            } else {
                DEBUG("No se pudo procesar statement directo: %s\n", child->symbol);
            }
        }
    }

    if (stmt_count == 0) {
        DEBUG("No se encontraron statements válidos\n");
        DEBUG("Hijos del StmtList:");
        for (int i = 0; i < cst_node->child_count; i++) {
            if (cst_node->children[i]) {
                DEBUG(" %s", cst_node->children[i]->symbol);
            } else {
                DEBUG(" NULL");
            }
        }
        DEBUG("\n");
        free(statements);
        return NULL;
    }

    ASTNode* program = create_program_node(statements, stmt_count, NODE_PROGRAM);
    free(statements); // create_program_node hace su propia copia
    
    DEBUG("Programa creado con %d statements\n", stmt_count);
    FINISH("EL stmtlist_to_ast termina exitosamente\n");
    return program;
}

ASTNode* terminated_stmt_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("el TerminatedStmt tiene %d hijos\n", cst_node->child_count);

    for (int i = 0; i < cst_node->child_count; i++) {
        if (!cst_node->children || !cst_node->children[i]) {
            DEBUG("Hijo %d es NULL, saltando...\n", i);
            continue;
        }
        
        DEBUG("el nombre de mi hijo %d es %s\n", i, cst_node->children[i]->symbol);
        CSTNode* child = cst_node->children[i];

        // Buscar el statement real (no el terminador ";" u otros tokens)
        if (child && strcmp(child->symbol, ";") != 0 && 
            strcmp(child->symbol, "SEMICOLON") != 0 &&
            strcmp(child->symbol, "LPAREN") != 0 &&
            strcmp(child->symbol, "RPAREN") != 0 &&
            strcmp(child->symbol, "LBRACKET") != 0 &&
            strcmp(child->symbol, "RBRACKET") != 0 &&
            strcmp(child->symbol, "ARROW") != 0 &&
            strcmp(child->symbol, "IN") != 0 &&
            strcmp(child->symbol, "LET") != 0 &&
            strcmp(child->symbol, "WHILE") != 0 &&
            strcmp(child->symbol, "FOR") != 0 &&
            strcmp(child->symbol, "IF") != 0 &&
            strcmp(child->symbol, "ELSE") != 0 &&
            strcmp(child->symbol, "FUNCTION") != 0 &&
            strcmp(child->symbol, "TYPE") != 0 &&
            strcmp(child->symbol, "INHERITS") != 0 &&
            strcmp(child->symbol, "NEW") != 0 &&
            strcmp(child->symbol, "BASE") != 0 &&
            strcmp(child->symbol, "AS") != 0 &&
            strcmp(child->symbol, "IS") != 0 &&
            strcmp(child->symbol, "AND") != 0 &&
            strcmp(child->symbol, "OR") != 0 &&
            strcmp(child->symbol, "RANGE") != 0 &&
            strcmp(child->symbol, "ε") != 0) {
            ASTNode* result = stmt_to_ast(child);
            if (result) {
                DEBUG("Statement procesado exitosamente\n");
                FINISH("EL terminated_stmt_to_ast termina exitosamente\n");
                return result;
            } else {
                DEBUG("stmt_to_ast retornó NULL para hijo %d\n", i);
            }
        } else if (child) {
            DEBUG("Ignorando token/símbolo en TerminatedStmt: %s\n", child->symbol);
        }
    }

    ERROR("No se encontró statement válido en TerminatedStmt\n");
    return NULL;
}

ASTNode* stmt_to_ast(CSTNode* cst_node)
{
    ACCEPT("El nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("el stmt tiene %d hijos\n", cst_node->child_count);

    // Determinar el tipo de statement basado en el símbolo
    if (!strcmp(cst_node->symbol, "FunctionDef")) {
        ASTNode* result = functionDef_to_ast(cst_node);
        DEBUG("FunctionDef procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "TypeDef")) {
        ASTNode* result = typeDef_to_ast(cst_node);
        DEBUG("TypeDef procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "Assignment")) {
        ASTNode* result = assignment_to_ast(cst_node);
        DEBUG("Assignment procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "WhileStmt")) {
        ASTNode* result = while_to_ast(cst_node);
        DEBUG("WhileStmt procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "ForStmt")) {
        ASTNode* result = for_to_ast(cst_node);
        DEBUG("ForStmt procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "Block")) {
        ASTNode* result = block_to_ast(cst_node);
        DEBUG("Block procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "Expr")) {
        ASTNode* result = expr_to_ast(cst_node);
        DEBUG("Expr procesado\n");
        return result;
    } else if (!strcmp(cst_node->symbol, "SEMICOLON") || 
               !strcmp(cst_node->symbol, ";") ||
               !strcmp(cst_node->symbol, "LPAREN") ||
               !strcmp(cst_node->symbol, "RPAREN") ||
               !strcmp(cst_node->symbol, "LBRACKET") ||
               !strcmp(cst_node->symbol, "RBRACKET") ||
               !strcmp(cst_node->symbol, "ARROW") ||
               !strcmp(cst_node->symbol, "IN") ||
               !strcmp(cst_node->symbol, "LET") ||
               !strcmp(cst_node->symbol, "WHILE") ||
               !strcmp(cst_node->symbol, "FOR") ||
               !strcmp(cst_node->symbol, "IF") ||
               !strcmp(cst_node->symbol, "ELSE") ||
               !strcmp(cst_node->symbol, "FUNCTION") ||
               !strcmp(cst_node->symbol, "TYPE") ||
               !strcmp(cst_node->symbol, "INHERITS") ||
               !strcmp(cst_node->symbol, "NEW") ||
               !strcmp(cst_node->symbol, "BASE") ||
               !strcmp(cst_node->symbol, "AS") ||
               !strcmp(cst_node->symbol, "IS") ||
               !strcmp(cst_node->symbol, "AND") ||
               !strcmp(cst_node->symbol, "OR") ||
               !strcmp(cst_node->symbol, "RANGE") ||
               !strcmp(cst_node->symbol, "ε")) {
        // Ignorar tokens y símbolos que no son statements
        DEBUG("Ignorando token/símbolo: %s\n", cst_node->symbol);
        return NULL;
    } else if (cst_node->child_count == 1) {
        // Statement con un solo hijo - procesarlo directamente
        DEBUG("Procesando hijo único\n");
        return stmt_to_ast(cst_node->children[0]);
    }

    ERROR("Statement no reconocido: %s\n", cst_node->symbol);
    FINISH("El stmt_to_ast termina\n");
    return NULL;
}

// ============================================================================
// EXPRESSIONS
// ============================================================================



ASTNode* conditional_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando condicional\n");
    DEBUG("Conditional tiene %d hijos\n", cst_node->child_count);
    
    ASTNode* condition = NULL;
    ASTNode* then_body = NULL;
    ASTNode* else_body = NULL;

    // Estructura esperada: IfExpr -> IF LPAREN Expr RPAREN IfBody ELSE IfBody
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de Conditional: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "Expr")) {
            // La expresión entre paréntesis es la condición
            condition = expr_to_ast(child);
            DEBUG("Condición del if procesada\n");
        } else if (!strcmp(child->symbol, "IfBody")) {
            // El primer IfBody es el then, el segundo es el else
            if (!then_body) {
                then_body = process_if_body(child);
                DEBUG("Cuerpo then del if procesado\n");
            } else {
                else_body = process_if_body(child);
                DEBUG("Cuerpo else del if procesado\n");
            }
        }
    }

    if (!condition || !then_body) {
        ERROR("Condicional incompleta\n");
        DEBUG("condition: %p, then_body: %p, else_body: %p\n", condition, then_body, else_body);
        return NULL;
    }

    return create_conditional_node(condition, then_body, else_body);
}

ASTNode* process_if_body(CSTNode* cst_node)
{
    ACCEPT("Procesando cuerpo de if: %s\n", cst_node->symbol);
    DEBUG("IfBody tiene %d hijos\n", cst_node->child_count);
    
    // IfBody -> BlockStmt | Expr
    if (!strcmp(cst_node->symbol, "BlockStmt")) {
        return block_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "Expr")) {
        return expr_to_ast(cst_node);
    } else if (cst_node->child_count == 1) {
        // Si tiene un solo hijo, procesarlo directamente
        CSTNode* child = cst_node->children[0];
        DEBUG("Procesando hijo único: %s\n", child->symbol);
        if (!strcmp(child->symbol, "BlockStmt")) {
            return block_to_ast(child);
        } else if (!strcmp(child->symbol, "Expr")) {
            return expr_to_ast(child);
        } else if (!strcmp(child->symbol, "LetExpr")) {
            return let_in_to_ast(child);
        }
    }
    
    ERROR("Cuerpo de if no reconocido: %s\n", cst_node->symbol);
    DEBUG("Hijos del IfBody:");
    for (int i = 0; i < cst_node->child_count; i++) {
        if (cst_node->children[i]) {
            DEBUG(" %s", cst_node->children[i]->symbol);
        } else {
            DEBUG(" NULL");
        }
    }
    DEBUG("\n");
    return NULL;
}

ASTNode* let_in_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando let-in: %s\n", cst_node->symbol);
    DEBUG("LetExpr tiene %d hijos\n", cst_node->child_count);
    
    ASTNode** declarations = NULL;
    int dec_count = 0;
    ASTNode* body = NULL;

    // Estructura esperada: LetExpr -> LET VarBindingList IN LetBody
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de LetExpr: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "VarBindingList")) {
            // Procesar la lista de declaraciones
            declarations = process_var_binding_list(child, &dec_count);
            DEBUG("VarBindingList procesada con %d declaraciones\n", dec_count);
        } else if (!strcmp(child->symbol, "LetBody")) {
            // Procesar el cuerpo del let-in
            body = process_let_body(child);
            DEBUG("LetBody procesado\n");
        }
    }

    if (!body) {
        ERROR("Let-in sin cuerpo\n");
        DEBUG("body: %p\n", body);
        if (declarations) {
            for (int i = 0; i < dec_count; i++) {
                if (declarations[i]) free_ast(declarations[i]);
            }
            free(declarations);
        }
        return NULL;
    }

    ASTNode* result = create_let_in_node(declarations, dec_count, body);
    DEBUG("Creado let-in con %d declaraciones\n", dec_count);
    if (declarations) free(declarations);
    return result;
}

ASTNode* assignment_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando asignación\n");
    DEBUG("Assignment tiene %d hijos\n", cst_node->child_count);
    
    char* var_name = NULL;
    ASTNode* value = NULL;
    char* type_name = NULL;
    
    // Buscar: let ID = Expr o let ID : Type = Expr
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de Assignment: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            var_name = strdup(child->token->value);
            DEBUG("Nombre de variable: %s\n", var_name);
        } else if (!strcmp(child->symbol, "TYPE") && child->token) {
            type_name = strdup(child->token->value);
            DEBUG("Tipo de variable: %s\n", type_name);
        } else if (!strcmp(child->symbol, "=")) {
            // Siguiente hijo es el valor
            if (i + 1 < cst_node->child_count) {
                value = expr_to_ast(cst_node->children[i + 1]);
                DEBUG("Valor de asignación procesado\n");
            }
        }
    }

    if (!var_name || !value) {
        ERROR("Asignación incompleta\n");
        DEBUG("var_name: %s, value: %p\n", var_name ? var_name : "NULL", value);
        return NULL;
    }

    return create_assignment_node(var_name, value, type_name, NODE_ASSIGNMENT);
}

ASTNode* funCall_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando llamada a función: %s\n", cst_node->symbol);
    DEBUG("FunCall tiene %d hijos\n", cst_node->child_count);
    
    // Esta función podría no ser llamada directamente si las llamadas
    // a función se manejan en primary_to_ast()
    char* func_name = NULL;
    ASTNode** args = NULL;
    int arg_count = 0;

    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de FunCall: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            func_name = strdup(child->token->value);
            DEBUG("Nombre de función: %s\n", func_name);
        } else if (!strcmp(child->symbol, "ArgList")) {
            args = process_arg_list(child, &arg_count);
            DEBUG("ArgList procesada con %d argumentos\n", arg_count);
        }
    }

    if (!func_name) {
        ERROR("Llamada a función sin nombre\n");
        return NULL;
    }

    ASTNode* result = create_func_call_node(func_name, args, arg_count);
    if (args) free(args);
    return result;
}

// ============================================================================
// STATEMENTS
// ============================================================================

ASTNode* functionDef_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando definición de función\n");
    DEBUG("FunctionDef tiene %d hijos\n", cst_node->child_count);
    
    char* func_name = NULL;
    ASTNode** params = NULL;
    int param_count = 0;
    ASTNode* body = NULL;
    char* return_type = NULL;

    // Debug: mostrar todos los hijos
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d: %s\n", i, child->symbol);
        if (child->token) {
            DEBUG("  Token: %s\n", child->token->value);
        }
    }

    // Procesar los hijos en orden específico basado en la estructura del CST
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            func_name = strdup(child->token->value);
            DEBUG("Encontrado nombre de función: %s\n", func_name);
        } else if (!strcmp(child->symbol, "ArgIdList")) {
            DEBUG("Procesando ArgIdList\n");
            // Procesar ArgIdList recursivamente
            for (int j = 0; j < child->child_count; j++) {
                CSTNode* arg_child = child->children[j];
                if (!strcmp(arg_child->symbol, "ArgId")) {
                    // Procesar un ArgId individual
                    for (int k = 0; k < arg_child->child_count; k++) {
                        if (!strcmp(arg_child->children[k]->symbol, "ID") && arg_child->children[k]->token) {
                            // Crear variable para el parámetro
                            ASTNode* param = create_variable_node(strdup(arg_child->children[k]->token->value), NULL, 0);
                            param->is_param = 1;
                            params = realloc(params, (param_count + 1) * sizeof(ASTNode*));
                            params[param_count++] = param;
                            DEBUG("Agregado parámetro: %s\n", arg_child->children[k]->token->value);
                        }
                    }
                }
            }
        } else if (!strcmp(child->symbol, "TypeAnnotation")) {
            DEBUG("Procesando TypeAnnotation\n");
            // Buscar el ID dentro de TypeAnnotation
            for (int j = 0; j < child->child_count; j++) {
                if (!strcmp(child->children[j]->symbol, "ID") && child->children[j]->token) {
                    return_type = strdup(child->children[j]->token->value);
                    DEBUG("Tipo de retorno: %s\n", return_type);
                    break;
                }
            }
        } else if (!strcmp(child->symbol, "FunctionBody")) {
            DEBUG("Procesando FunctionBody\n");
            // FunctionBody tiene ARROW y FunctionBodyExpr como hijos
            for (int j = 0; j < child->child_count; j++) {
                if (!strcmp(child->children[j]->symbol, "FunctionBodyExpr")) {
                    // FunctionBodyExpr puede ser Expr, WhileStmt, o ForStmt
                    CSTNode* body_expr = child->children[j];
                    DEBUG("FunctionBodyExpr tiene %d hijos\n", body_expr->child_count);
                    for (int k = 0; k < body_expr->child_count; k++) {
                        CSTNode* body_child = body_expr->children[k];
                        DEBUG("Hijo %d de FunctionBodyExpr: %s\n", k, body_child->symbol);
                        if (!strcmp(body_child->symbol, "Expr")) {
                            body = expr_to_ast(body_child);
                            DEBUG("Cuerpo de función (Expr) procesado\n");
                            break;
                        } else if (!strcmp(body_child->symbol, "WhileStmt")) {
                            body = while_to_ast(body_child);
                            DEBUG("Cuerpo de función (WhileStmt) procesado\n");
                            break;
                        } else if (!strcmp(body_child->symbol, "ForStmt")) {
                            body = for_to_ast(body_child);
                            DEBUG("Cuerpo de función (ForStmt) procesado\n");
                            break;
                        } else if (body_child->child_count == 1) {
                            // Si tiene un solo hijo, procesarlo directamente
                            CSTNode* grandchild = body_child->children[0];
                            if (!strcmp(grandchild->symbol, "Expr")) {
                                body = expr_to_ast(grandchild);
                                DEBUG("Cuerpo de función (Expr anidado) procesado\n");
                                break;
                            } else if (!strcmp(grandchild->symbol, "WhileStmt")) {
                                body = while_to_ast(grandchild);
                                DEBUG("Cuerpo de función (WhileStmt anidado) procesado\n");
                                break;
                            } else if (!strcmp(grandchild->symbol, "ForStmt")) {
                                body = for_to_ast(grandchild);
                                DEBUG("Cuerpo de función (ForStmt anidado) procesado\n");
                                break;
                            }
                        }
                    }
                    break;
                } else if (!strcmp(child->children[j]->symbol, "BlockStmt")) {
                    body = block_to_ast(child->children[j]);
                    DEBUG("Cuerpo de función (BlockStmt) procesado\n");
                    break;
                } else if (!strcmp(child->children[j]->symbol, "ARROW")) {
                    // Ignorar el token ARROW
                    DEBUG("Ignorando token ARROW\n");
                } else if (child->children[j]->child_count == 1) {
                    // Si tiene un solo hijo, procesarlo directamente
                    CSTNode* grandchild = child->children[j]->children[0];
                    if (!strcmp(grandchild->symbol, "FunctionBodyExpr")) {
                        // Procesar FunctionBodyExpr recursivamente
                        for (int k = 0; k < grandchild->child_count; k++) {
                            CSTNode* body_child = grandchild->children[k];
                            if (!strcmp(body_child->symbol, "Expr")) {
                                body = expr_to_ast(body_child);
                                DEBUG("Cuerpo de función (Expr) procesado (recursivo)\n");
                                break;
                            } else if (!strcmp(body_child->symbol, "WhileStmt")) {
                                body = while_to_ast(body_child);
                                DEBUG("Cuerpo de función (WhileStmt) procesado (recursivo)\n");
                                break;
                            } else if (!strcmp(body_child->symbol, "ForStmt")) {
                                body = for_to_ast(body_child);
                                DEBUG("Cuerpo de función (ForStmt) procesado (recursivo)\n");
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }

    if (!func_name || !body) {
        ERROR("Definición de función incompleta\n");
        DEBUG("func_name: %s, body: %p\n", func_name ? func_name : "NULL", body);
        if (func_name) free(func_name);
        if (return_type) free(return_type);
        if (params) free(params);
        return NULL;
    }

    ASTNode* result = create_func_dec_node(func_name, params, param_count, body, return_type);
    DEBUG("Función creada exitosamente: %s\n", func_name);
    free(params);
    return result;
}

ASTNode* typeDef_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando definición de tipo\n");
    DEBUG("TypeDef tiene %d hijos\n", cst_node->child_count);
    
    char* type_name = NULL;
    char* parent_name = NULL;
    ASTNode** params = NULL;
    int param_count = 0;
    ASTNode** p_params = NULL;
    int p_param_count = 0;
    ASTNode* body = NULL;
    int p_constructor = 0;

    // Buscar: type ID ( ParamList ) inherits ParentType ( ParentParams ) { Body }
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de TypeDef: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            if (!type_name) {
                type_name = strdup(child->token->value);
                DEBUG("Nombre de tipo: %s\n", type_name);
            } else if (!parent_name) {
                parent_name = strdup(child->token->value);
                DEBUG("Nombre de padre: %s\n", parent_name);
            }
        } else if (!strcmp(child->symbol, "ParamList")) {
            // Procesar parámetros del tipo
            for (int j = 0; j < child->child_count; j++) {
                if (strcmp(child->children[j]->symbol, ",") != 0) {
                    ASTNode* param = variable_to_ast(child->children[j]);
                    if (param) {
                        params = realloc(params, (param_count + 1) * sizeof(ASTNode*));
                        params[param_count++] = param;
                    }
                }
            }
            DEBUG("Parámetros del tipo procesados: %d\n", param_count);
        } else if (!strcmp(child->symbol, "ParentParams")) {
            // Procesar parámetros del padre
            for (int j = 0; j < child->child_count; j++) {
                if (strcmp(child->children[j]->symbol, ",") != 0) {
                    ASTNode* param = expr_to_ast(child->children[j]);
                    if (param) {
                        p_params = realloc(p_params, (p_param_count + 1) * sizeof(ASTNode*));
                        p_params[p_param_count++] = param;
                    }
                }
            }
            p_constructor = 1;
            DEBUG("Parámetros del padre procesados: %d\n", p_param_count);
        } else if (!strcmp(child->symbol, "Block") || !strcmp(child->symbol, "Body")) {
            body = block_to_ast(child);
            DEBUG("Cuerpo del tipo procesado\n");
        }
    }

    if (!type_name || !body) {
        ERROR("Definición de tipo incompleta\n");
        DEBUG("type_name: %s, body: %p\n", type_name ? type_name : "NULL", body);
        return NULL;
    }

    ASTNode* result = create_type_dec_node(type_name, params, param_count, 
                                          parent_name, p_params, p_param_count, 
                                          body, p_constructor);
    free(params);
    free(p_params);
    return result;
}

ASTNode* while_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando while\n");
    DEBUG("WhileStmt tiene %d hijos\n", cst_node->child_count);
    
    ASTNode* condition = NULL;
    ASTNode* body = NULL;

    // Estructura esperada: WhileStmt -> WHILE Expr WhileBody
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de WhileStmt: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "Expr")) {
            condition = expr_to_ast(child);
            DEBUG("Condición del while procesada: %p\n", condition);
        } else if (!strcmp(child->symbol, "WhileBody")) {
            body = process_while_body(child);
            DEBUG("Cuerpo del while procesado: %p\n", body);
        } else if (!strcmp(child->symbol, "WHILE")) {
            // Ignorar el token WHILE
            DEBUG("Ignorando token WHILE\n");
        } else if (child->child_count == 1) {
            // Si tiene un solo hijo, procesarlo recursivamente
            DEBUG("Procesando hijo único de WhileStmt: %s\n", child->children[0]->symbol);
            if (!strcmp(child->children[0]->symbol, "Expr")) {
                condition = expr_to_ast(child->children[0]);
                DEBUG("Condición del while procesada (hijo único): %p\n", condition);
            } else if (!strcmp(child->children[0]->symbol, "WhileBody")) {
                body = process_while_body(child->children[0]);
                DEBUG("Cuerpo del while procesado (hijo único): %p\n", body);
            }
        }
    }

    // Si no encontramos la condición o el cuerpo directamente, buscar en hijos anidados
    if (!condition || !body) {
        DEBUG("Buscando condición y cuerpo en hijos anidados...\n");
        for (int i = 0; i < cst_node->child_count; i++) {
            CSTNode* child = cst_node->children[i];
            if (!child) continue;
            
            // Buscar recursivamente en todos los hijos
            for (int j = 0; j < child->child_count; j++) {
                CSTNode* grandchild = child->children[j];
                if (!grandchild) continue;
                
                if (!condition && !strcmp(grandchild->symbol, "Expr")) {
                    condition = expr_to_ast(grandchild);
                    DEBUG("Condición encontrada en hijo anidado: %p\n", condition);
                } else if (!body && !strcmp(grandchild->symbol, "WhileBody")) {
                    body = process_while_body(grandchild);
                    DEBUG("Cuerpo encontrado en hijo anidado: %p\n", body);
                }
            }
        }
    }

    if (!condition || !body) {
        ERROR("While incompleto\n");
        DEBUG("condition: %p, body: %p\n", condition, body);
        DEBUG("Hijos del WhileStmt:");
        for (int i = 0; i < cst_node->child_count; i++) {
            if (cst_node->children[i]) {
                DEBUG(" %s", cst_node->children[i]->symbol);
            } else {
                DEBUG(" NULL");
            }
        }
        DEBUG("\n");
        if (condition) free_ast(condition);
        if (body) free_ast(body);
        return NULL;
    }

    ASTNode* result = create_loop_node(condition, body);
    DEBUG("While creado exitosamente\n");
    return result;
}

ASTNode* process_while_body(CSTNode* cst_node)
{
    ACCEPT("Procesando cuerpo de while: %s\n", cst_node->symbol);
    DEBUG("WhileBody tiene %d hijos\n", cst_node->child_count);
    
    // WhileBody -> BlockStmt | Expr
    if (!strcmp(cst_node->symbol, "BlockStmt")) {
        return block_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "Expr")) {
        return expr_to_ast(cst_node);
    } else if (cst_node->child_count == 1) {
        // Si tiene un solo hijo, procesarlo directamente
        CSTNode* child = cst_node->children[0];
        DEBUG("Procesando hijo único: %s\n", child->symbol);
        if (!strcmp(child->symbol, "BlockStmt")) {
            return block_to_ast(child);
        } else if (!strcmp(child->symbol, "Expr")) {
            return expr_to_ast(child);
        } else if (!strcmp(child->symbol, "LetExpr")) {
            return let_in_to_ast(child);
        }
    }
    
    ERROR("Cuerpo de while no reconocido: %s\n", cst_node->symbol);
    DEBUG("Hijos del WhileBody:");
    for (int i = 0; i < cst_node->child_count; i++) {
        if (cst_node->children[i]) {
            DEBUG(" %s", cst_node->children[i]->symbol);
        } else {
            DEBUG(" NULL");
        }
    }
    DEBUG("\n");
    return NULL;
}

ASTNode* for_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando ciclo for\n");
    DEBUG("ForStmt tiene %d hijos\n", cst_node->child_count);
    
    char* var_name = NULL;
    ASTNode* range_expr = NULL;
    ASTNode* body = NULL;

    // Estructura esperada: ForStmt -> FOR LPAREN ID IN Expr RPAREN ForBody
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de ForStmt: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            var_name = strdup(child->token->value);
            DEBUG("Variable del for: %s\n", var_name);
        } else if (!strcmp(child->symbol, "Expr")) {
            range_expr = expr_to_ast(child);
            DEBUG("Expresión de rango procesada\n");
        } else if (!strcmp(child->symbol, "ForBody")) {
            body = process_for_body(child);
            DEBUG("Cuerpo del for procesado\n");
        }
    }

    if (!var_name || !range_expr || !body) {
        ERROR("Ciclo for incompleto\n");
        DEBUG("var_name: %s, range_expr: %p, body: %p\n", 
              var_name ? var_name : "NULL", range_expr, body);
        if (var_name) free(var_name);
        if (range_expr) free_ast(range_expr);
        if (body) free_ast(body);
        return NULL;
    }

    ASTNode* result = create_for_loop_node(var_name, &range_expr, body, 1);
    DEBUG("Creado ciclo for con variable %s\n", var_name);
    return result;
}

ASTNode* process_for_body(CSTNode* cst_node)
{
    ACCEPT("Procesando cuerpo de for: %s\n", cst_node->symbol);
    DEBUG("ForBody tiene %d hijos\n", cst_node->child_count);
    
    // ForBody -> BlockStmt | Expr
    if (!strcmp(cst_node->symbol, "BlockStmt")) {
        return block_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "Expr")) {
        return expr_to_ast(cst_node);
    } else if (cst_node->child_count == 1) {
        // Si tiene un solo hijo, procesarlo directamente
        CSTNode* child = cst_node->children[0];
        DEBUG("Procesando hijo único: %s\n", child->symbol);
        if (!strcmp(child->symbol, "BlockStmt")) {
            return block_to_ast(child);
        } else if (!strcmp(child->symbol, "Expr")) {
            return expr_to_ast(child);
        } else if (!strcmp(child->symbol, "LetExpr")) {
            return let_in_to_ast(child);
        }
    }
    
    ERROR("Cuerpo de for no reconocido: %s\n", cst_node->symbol);
    DEBUG("Hijos del ForBody:");
    for (int i = 0; i < cst_node->child_count; i++) {
        if (cst_node->children[i]) {
            DEBUG(" %s", cst_node->children[i]->symbol);
        } else {
            DEBUG(" NULL");
        }
    }
    DEBUG("\n");
    return NULL;
}

ASTNode* block_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando bloque\n");
    DEBUG("Block tiene %d hijos\n", cst_node->child_count);
    
    ASTNode** statements = NULL;
    int stmt_count = 0;

    // Buscar: { StmtList }
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de Block: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "StmtList")) {
            ASTNode* stmt_list = stmtList_to_ast(child);
            if (stmt_list && stmt_list->type == NODE_PROGRAM) {
                statements = malloc(stmt_list->data.program_node.count * sizeof(ASTNode*));
                stmt_count = stmt_list->data.program_node.count;
                for (int j = 0; j < stmt_count; j++) {
                    statements[j] = stmt_list->data.program_node.statements[j];
                }
                DEBUG("Block procesado con %d statements\n", stmt_count);
            }
        }
    }

    if (stmt_count == 0) {
        DEBUG("Block vacío\n");
        return create_program_node(NULL, 0, NODE_BLOCK);
    }

    ASTNode* result = create_program_node(statements, stmt_count, NODE_BLOCK);
    free(statements);
    return result;
}

// ============================================================================
// PRIMARY EXPRESSIONS
// ============================================================================

ASTNode* number_to_ast(CSTNode* cst_node)
{
    if (!cst_node->token || !cst_node->token->value) {
        ERROR("Número sin valor\n");
        return NULL;
    }

    // Actualizar line_num antes de crear el nodo
    extern int line_num;
    line_num = cst_node->token->line;
    
    double value = atof(cst_node->token->value);
    DEBUG("Número procesado: %f\n", value);
    return create_number_node(value);
}

ASTNode* string_to_ast(CSTNode* cst_node)
{
    if (!cst_node->token || !cst_node->token->value) {
        ERROR("String sin valor\n");
        return NULL;
    }

    // Actualizar line_num antes de crear el nodo
    extern int line_num;
    line_num = cst_node->token->line;

    // Remover comillas y procesar escapes
    char* value = strdup(cst_node->token->value);
    DEBUG("String procesado: %s\n", value);
    return create_string_node(value);
}

ASTNode* boolean_to_ast(CSTNode* cst_node)
{
    if (!cst_node->token || !cst_node->token->value) {
        ERROR("Boolean sin valor\n");
        return NULL;
    }

    // Actualizar line_num antes de crear el nodo
    extern int line_num;
    line_num = cst_node->token->line;

    char* value = strdup(cst_node->token->value);
    DEBUG("Boolean procesado: %s\n", value);
    return create_boolean_node(value);
}

ASTNode* variable_to_ast(CSTNode* cst_node)
{
    if (!cst_node->token || !cst_node->token->value) {
        ERROR("Variable sin nombre\n");
        return NULL;
    }

    // Actualizar line_num antes de crear el nodo
    extern int line_num;
    line_num = cst_node->token->line;

    char* name = strdup(cst_node->token->value);
    DEBUG("Variable procesada: %s\n", name);
    return create_variable_node(name, NULL, 0);
}

// ============================================================================
// TYPE OPERATIONS
// ============================================================================

ASTNode* casting_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando casting\n");
    
    ASTNode* expr = NULL;
    char* type_name = NULL;
    int is_test = 0;

    // Buscar: expr as Type o expr is Type
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, "as")) {
            is_test = 0;
            // Anterior hijo es expresión, siguiente es tipo
            if (i > 0) expr = expr_to_ast(cst_node->children[i - 1]);
            if (i + 1 < cst_node->child_count && cst_node->children[i + 1]->token) {
                type_name = strdup(cst_node->children[i + 1]->token->value);
            }
        } else if (!strcmp(child->symbol, "is")) {
            is_test = 1;
            // Anterior hijo es expresión, siguiente es tipo
            if (i > 0) expr = expr_to_ast(cst_node->children[i - 1]);
            if (i + 1 < cst_node->child_count && cst_node->children[i + 1]->token) {
                type_name = strdup(cst_node->children[i + 1]->token->value);
            }
        }
    }

    if (!expr || !type_name) {
        ERROR("Casting incompleto\n");
        return NULL;
    }

    return create_test_casting_type_node(expr, type_name, is_test);
}

ASTNode* instance_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando instancia de tipo\n");
    
    char* type_name = NULL;
    ASTNode** args = NULL;
    int arg_count = 0;

    // Buscar: new Type ( ArgumentList )
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            type_name = strdup(child->token->value);
        } else if (!strcmp(child->symbol, "ArgumentList")) {
            // Procesar argumentos
            for (int j = 0; j < child->child_count; j++) {
                if (strcmp(child->children[j]->symbol, ",") != 0) {
                    ASTNode* arg = expr_to_ast(child->children[j]);
                    if (arg) {
                        args = realloc(args, (arg_count + 1) * sizeof(ASTNode*));
                        args[arg_count++] = arg;
                    }
                }
            }
        }
    }

    if (!type_name) {
        ERROR("Instancia sin tipo\n");
        return NULL;
    }

    ASTNode* result = create_type_instance_node(type_name, args, arg_count);
    free(args);
    return result;
}

ASTNode* getter_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando getter\n");
    
    ASTNode* instance = NULL;
    ASTNode* member = NULL;

    // Buscar: instance . member
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, ".")) {
            // Anterior hijo es instancia, siguiente es miembro
            if (i > 0) instance = expr_to_ast(cst_node->children[i - 1]);
            if (i + 1 < cst_node->child_count) {
                member = expr_to_ast(cst_node->children[i + 1]);
            }
        }
    }

    if (!instance || !member) {
        ERROR("Getter incompleto\n");
        return NULL;
    }

    return create_attr_getter_node(instance, member);
}

ASTNode* setter_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando setter\n");
    
    ASTNode* instance = NULL;
    ASTNode* member = NULL;
    ASTNode* value = NULL;

    // Buscar: instance . member = value
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, ".")) {
            // Anterior hijo es instancia, siguiente es miembro
            if (i > 0) instance = expr_to_ast(cst_node->children[i - 1]);
            if (i + 1 < cst_node->child_count) {
                member = expr_to_ast(cst_node->children[i + 1]);
            }
        } else if (!strcmp(child->symbol, "=")) {
            // Siguiente hijo es valor
            if (i + 1 < cst_node->child_count) {
                value = expr_to_ast(cst_node->children[i + 1]);
            }
        }
    }

    if (!instance || !member || !value) {
        ERROR("Setter incompleto\n");
        return NULL;
    }

    return create_attr_setter_node(instance, member, value);
}

ASTNode* base_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando base\n");
    
    ASTNode** args = NULL;
    int arg_count = 0;

    // Buscar: base ( ArgumentList )
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!strcmp(child->symbol, "ArgumentList")) {
            // Procesar argumentos
            for (int j = 0; j < child->child_count; j++) {
                if (strcmp(child->children[j]->symbol, ",") != 0) {
                    ASTNode* arg = expr_to_ast(child->children[j]);
                    if (arg) {
                        args = realloc(args, (arg_count + 1) * sizeof(ASTNode*));
                        args[arg_count++] = arg;
                    }
                }
            }
        }
    }

    ASTNode* result = create_base_func_node(args, arg_count);
    free(args);
    return result;
}

// ============================================================================
// OPERATORS
// ============================================================================

ASTNode* binary_expr_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando expresión binaria\n");
    DEBUG("BinaryExpr tiene %d hijos\n", cst_node->child_count);
    
    ASTNode* left = NULL;
    ASTNode* right = NULL;
    char* op_str = NULL;
    Operator op = OP_ADD;

    // Buscar: left_expr OPERATOR right_expr
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de BinaryExpr: %s\n", i, child->symbol);
        
        if (i == 0) {
            // Primer hijo es operando izquierdo
            left = expr_to_ast(child);
            DEBUG("Operando izquierdo procesado\n");
        } else if (i == 1 && child->token) {
            // Segundo hijo es operador
            op_str = strdup(child->token->value);
            DEBUG("Operador binario: %s\n", op_str);
            
            // Mapear string a enum
            if (!strcmp(op_str, "+")) op = OP_ADD;
            else if (!strcmp(op_str, "-")) op = OP_SUB;
            else if (!strcmp(op_str, "*")) op = OP_MUL;
            else if (!strcmp(op_str, "/")) op = OP_DIV;
            else if (!strcmp(op_str, "%")) op = OP_MOD;
            else if (!strcmp(op_str, "^")) op = OP_POW;
            else if (!strcmp(op_str, "&")) op = OP_AND;
            else if (!strcmp(op_str, "|")) op = OP_OR;
            else if (!strcmp(op_str, "@")) op = OP_CONCAT;
            else if (!strcmp(op_str, "@@")) op = OP_DCONCAT;
            else if (!strcmp(op_str, "==")) op = OP_EQ;
            else if (!strcmp(op_str, "!=")) op = OP_NEQ;
            else if (!strcmp(op_str, ">")) op = OP_GR;
            else if (!strcmp(op_str, "<")) op = OP_LS;
            else if (!strcmp(op_str, ">=")) op = OP_GRE;
            else if (!strcmp(op_str, "<=")) op = OP_LSE;
            
        } else if (i == 2) {
            // Tercer hijo es operando derecho
            right = expr_to_ast(child);
            DEBUG("Operando derecho procesado\n");
        }
    }

    if (!left || !right || !op_str) {
        ERROR("Expresión binaria incompleta\n");
        DEBUG("left: %p, right: %p, op_str: %s\n", left, right, op_str ? op_str : "NULL");
        return NULL;
    }

    return create_binary_op_node(op, op_str, left, right, &TYPE_OBJECT);
}

ASTNode* unary_expr_to_ast(CSTNode* cst_node)
{
    ACCEPT("Procesando expresión unaria\n");
    DEBUG("UnaryExpr tiene %d hijos\n", cst_node->child_count);
    
    ASTNode* operand = NULL;
    char* op_str = NULL;
    Operator op = OP_ADD;

    // Buscar: OPERATOR operand
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        DEBUG("Hijo %d de UnaryExpr: %s\n", i, child->symbol);
        
        if (i == 0 && child->token) {
            // Primer hijo es operador
            op_str = strdup(child->token->value);
            DEBUG("Operador unario: %s\n", op_str);
            
            // Mapear string a enum
            if (!strcmp(op_str, "-")) op = OP_SUB;
            else if (!strcmp(op_str, "!")) op = OP_NOT;
            
        } else if (i == 1) {
            // Segundo hijo es operando
            operand = expr_to_ast(child);
            DEBUG("Operando unario procesado\n");
        }
    }

    if (!operand || !op_str) {
        ERROR("Expresión unaria incompleta\n");
        DEBUG("operand: %p, op_str: %s\n", operand, op_str ? op_str : "NULL");
        return NULL;
    }

    return create_unary_op_node(op, op_str, operand, &TYPE_OBJECT);
}

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================

void debug_cst_node(CSTNode* node, int depth)
{
    if (!node) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    printf("CST: %s", node->symbol);
    if (node->token && node->token->value) {
        printf(" [%s]", node->token->value);
    }
    printf("\n");
    
    for (int i = 0; i < node->child_count; i++) {
        debug_cst_node(node->children[i], depth + 1);
    }
}

// Función auxiliar para manejar mejor los fallos en stmtList_to_ast
ASTNode* stmtList_to_ast_safe(CSTNode* cst_node)
{
    if (!cst_node) {
        ERROR("Nodo CST es NULL en stmtList_to_ast\n");
        return NULL;
    }
    
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);
    DEBUG("el StmtList tiene %d hijos\n", cst_node->child_count);

    // Crear lista de statements
    ASTNode** statements = NULL;
    int stmt_count = 0;

    for (int i = 0; i < cst_node->child_count; i++) {
        if (!cst_node->children || !cst_node->children[i]) {
            DEBUG("Hijo %d es NULL, saltando...\n", i);
            continue;
        }
        
        DEBUG("el nombre de mi hijo %d es %s\n", i, cst_node->children[i]->symbol);
        CSTNode* child = cst_node->children[i];

        if (!strcmp(child->symbol, "TerminatedStmt")) {
            ASTNode* stmt = terminated_stmt_to_ast(child);
            if (stmt) {
                statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                statements[stmt_count++] = stmt;
            } else {
                DEBUG("terminated_stmt_to_ast retornó NULL para hijo %d\n", i);
            }
        } else if (!strcmp(child->symbol, "StmtList")) {
            // Recursión para manejar listas anidadas
            ASTNode* nested_list = stmtList_to_ast_safe(child);
            if (nested_list && nested_list->type == NODE_PROGRAM) {
                // Agregar todos los statements de la lista anidada
                for (int j = 0; j < nested_list->data.program_node.count; j++) {
                    statements = realloc(statements, (stmt_count + 1) * sizeof(ASTNode*));
                    statements[stmt_count++] = nested_list->data.program_node.statements[j];
                }
            }
        } else if (!strcmp(child->symbol, "ε")) {
            // Ignorar epsilon
            continue;
        }
    }

    if (stmt_count == 0) {
        if (statements) free(statements);
        return NULL;
    }

    ASTNode* program = create_program_node(statements, stmt_count, NODE_PROGRAM);
    if (statements) free(statements); // create_program_node hace su propia copia
    
    FINISH("EL stmtlist_to_ast termina exitosamente\n");
    return program;
}

// Función auxiliar para procesar VarBindingList
ASTNode** process_var_binding_list(CSTNode* var_binding_list, int* count)
{
    *count = 0;
    ASTNode** declarations = NULL;
    
    DEBUG("VarBindingList tiene %d hijos\n", var_binding_list->child_count);
    
    for (int i = 0; i < var_binding_list->child_count; i++) {
        CSTNode* child = var_binding_list->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de VarBindingList: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "VarBinding")) {
            ASTNode* decl = process_var_binding(child);
            if (decl) {
                declarations = realloc(declarations, (*count + 1) * sizeof(ASTNode*));
                declarations[*count] = decl;
                (*count)++;
                DEBUG("VarBinding %d procesado\n", *count);
            }
        }
        // Ignorar VarBindingListTail si es ε
    }
    
    return declarations;
}

// Función auxiliar para procesar un VarBinding individual
ASTNode* process_var_binding(CSTNode* var_binding)
{
    char* var_name = NULL;
    char* type_name = NULL;
    ASTNode* value = NULL;
    
    DEBUG("VarBinding tiene %d hijos\n", var_binding->child_count);
    
    // Estructura esperada: VarBinding -> ID TypeAnnotation EQUALS Expr
    for (int i = 0; i < var_binding->child_count; i++) {
        CSTNode* child = var_binding->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de VarBinding: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID") && child->token) {
            var_name = strdup(child->token->value);
            DEBUG("Nombre de variable: %s\n", var_name);
        } else if (!strcmp(child->symbol, "TypeAnnotation")) {
            // Puede contener un tipo o ser ε
            if (child->child_count > 0 && child->children[0] && 
                strcmp(child->children[0]->symbol, "ε") != 0) {
                if (child->children[0]->token) {
                    type_name = strdup(child->children[0]->token->value);
                    DEBUG("Tipo de variable: %s\n", type_name);
                }
            }
        } else if (!strcmp(child->symbol, "Expr")) {
            value = expr_to_ast(child);
            DEBUG("Valor de variable procesado\n");
        }
    }
    
    if (!var_name || !value) {
        ERROR("VarBinding incompleto\n");
        DEBUG("var_name: %s, value: %p\n", var_name ? var_name : "NULL", value);
        if (var_name) free(var_name);
        if (type_name) free(type_name);
        if (value) free_ast(value);
        return NULL;
    }
    
    ASTNode* result = create_assignment_node(var_name, value, type_name ? type_name : "", NODE_ASSIGNMENT);
    
    DEBUG("Creada asignación: %s = valor\n", var_name);
    
    // No liberamos var_name y type_name aquí porque create_assignment_node los usa
    return result;
}

// Función auxiliar para procesar LetBody
ASTNode* process_let_body(CSTNode* let_body)
{
    ACCEPT("Procesando LetBody: %s\n", let_body->symbol);
    DEBUG("LetBody tiene %d hijos\n", let_body->child_count);
    
    // LetBody -> Expr | WhileStmt | ForStmt | BlockStmt
    for (int i = 0; i < let_body->child_count; i++) {
        CSTNode* child = let_body->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de LetBody: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "Expr")) {
            ASTNode* result = expr_to_ast(child);
            DEBUG("Expr procesado en LetBody\n");
            return result;
        } else if (!strcmp(child->symbol, "WhileStmt")) {
            ASTNode* result = while_to_ast(child);
            DEBUG("WhileStmt procesado en LetBody\n");
            return result;
        } else if (!strcmp(child->symbol, "ForStmt")) {
            ASTNode* result = for_to_ast(child);
            DEBUG("ForStmt procesado en LetBody\n");
            return result;
        } else if (!strcmp(child->symbol, "BlockStmt")) {
            ASTNode* result = block_to_ast(child);
            DEBUG("BlockStmt procesado en LetBody\n");
            return result;
        } else if (child->child_count == 1) {
            // Si tiene un solo hijo, procesarlo directamente
            CSTNode* grandchild = child->children[0];
            DEBUG("Procesando nieto: %s\n", grandchild->symbol);
            if (!strcmp(grandchild->symbol, "Expr")) {
                return expr_to_ast(grandchild);
            } else if (!strcmp(grandchild->symbol, "WhileStmt")) {
                return while_to_ast(grandchild);
            } else if (!strcmp(grandchild->symbol, "ForStmt")) {
                return for_to_ast(grandchild);
            } else if (!strcmp(grandchild->symbol, "BlockStmt")) {
                return block_to_ast(grandchild);
            }
        }
    }
    
    ERROR("LetBody sin expresión válida\n");
    DEBUG("Hijos del LetBody:");
    for (int i = 0; i < let_body->child_count; i++) {
        if (let_body->children[i]) {
            DEBUG(" %s", let_body->children[i]->symbol);
        } else {
            DEBUG(" NULL");
        }
    }
    DEBUG("\n");
    return NULL;
}

// Función mejorada para manejar la jerarquía de expresiones
ASTNode* expr_to_ast(CSTNode* cst_node)
{
    if (!cst_node) {
        ERROR("Nodo CST es NULL en expr_to_ast\n");
        return NULL;
    }
    
    ACCEPT("Procesando expresión: %s\n", cst_node->symbol);
    
    // Manejar Primary especialmente para llamadas a función
    if (!strcmp(cst_node->symbol, "Primary")) {
        return primary_to_ast(cst_node);
    }
    
    // Manejar expresiones jerárquicas con operadores
    if (!strcmp(cst_node->symbol, "OrExpr") || 
        !strcmp(cst_node->symbol, "AndExpr") ||
        !strcmp(cst_node->symbol, "CmpExpr") ||
        !strcmp(cst_node->symbol, "ConcatExpr") ||
        !strcmp(cst_node->symbol, "AddExpr") ||
        !strcmp(cst_node->symbol, "Term") ||
        !strcmp(cst_node->symbol, "Factor") ||
        !strcmp(cst_node->symbol, "Power") ||
        !strcmp(cst_node->symbol, "Unary")) {
        
        // Procesar el primer hijo no primed como operando izquierdo
        ASTNode* left_operand = NULL;
        CSTNode* primed_node = NULL;
        
        for (int i = 0; i < cst_node->child_count; i++) {
            CSTNode* child = cst_node->children[i];
            if (!child || strcmp(child->symbol, "ε") == 0) continue;
            
            if (strstr(child->symbol, "'") == NULL) {
                left_operand = expr_to_ast(child);
            } else {
                primed_node = child;
            }
        }
        
        // Si hay una producción primed, procesarla
        if (primed_node && left_operand) {
            return process_primed_expression(primed_node, left_operand);
        }
        
        // Si no hay producción primed, devolver el operando izquierdo
        if (left_operand) {
            return left_operand;
        }
        
        // Fallback: procesar el primer hijo no vacío
        for (int i = 0; i < cst_node->child_count; i++) {
            CSTNode* child = cst_node->children[i];
            if (!child || strcmp(child->symbol, "ε") == 0) continue;
            return expr_to_ast(child);
        }
    }
    
    // Manejar expresiones específicas
    if (!strcmp(cst_node->symbol, "IfExpr")) {
        return conditional_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "LetExpr")) {
        return let_in_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "Conditional")) {
        return conditional_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "BinaryExpr")) {
        return binary_expr_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "UnaryExpr")) {
        return unary_expr_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "FunCall")) {
        return funCall_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "TypeInstance")) {
        return instance_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "TypeCasting")) {
        return casting_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "AttributeGetter")) {
        return getter_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "AttributeSetter")) {
        return setter_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "BaseCall")) {
        return base_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "NUMBER")) {
        return number_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "STRING")) {
        return string_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "BOOLEAN")) {
        return boolean_to_ast(cst_node);
    } else if (!strcmp(cst_node->symbol, "ID")) {
        return variable_to_ast(cst_node);
    } else if (cst_node->child_count == 1) {
        // Expresión con un solo hijo
        if (cst_node->children && cst_node->children[0]) {
            return expr_to_ast(cst_node->children[0]);
        }
    } else if (cst_node->child_count == 0 || !strcmp(cst_node->symbol, "ε")) {
        // Nodo epsilon o nodo vacío - ignorar
        DEBUG("Ignorando nodo epsilon o vacío: %s\n", cst_node->symbol);
        return NULL;
    }

    ERROR("Expresión no reconocida: %s\n", cst_node->symbol);
    return NULL;
}

// Función mejorada para manejar Primary con PrimaryTail (para llamadas a función)
ASTNode* primary_to_ast(CSTNode* cst_node)
{
    ASTNode* base = NULL;
    DEBUG("Primary tiene %d hijos\n", cst_node->child_count);
    
    // Buscar el elemento base (ID, NUMBER, STRING, etc.)
    for (int i = 0; i < cst_node->child_count; i++) {
        CSTNode* child = cst_node->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de Primary: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ID")) {
            base = variable_to_ast(child);
            DEBUG("Base ID procesada\n");
        } else if (!strcmp(child->symbol, "NUMBER")) {
            base = number_to_ast(child);
            DEBUG("Base NUMBER procesada\n");
        } else if (!strcmp(child->symbol, "STRING")) {
            base = string_to_ast(child);
            DEBUG("Base STRING procesada\n");
        } else if (!strcmp(child->symbol, "BOOLEAN")) {
            base = boolean_to_ast(child);
            DEBUG("Base BOOLEAN procesada\n");
        } else if (!strcmp(child->symbol, "PrimaryTail")) {
            // Manejar PrimaryTail para llamadas a función
            if (base && base->type == NODE_VARIABLE) {
                // Verificar si PrimaryTail tiene contenido (no es ε)
                int has_content = 0;
                for (int j = 0; j < child->child_count; j++) {
                    if (child->children[j] && strcmp(child->children[j]->symbol, "ε") != 0) {
                        has_content = 1;
                        break;
                    }
                }
                
                if (has_content) {
                    ASTNode* func_call = process_primary_tail(base, child);
                    if (func_call) {
                        DEBUG("Llamada a función procesada\n");
                        return func_call;
                    }
                }
            }
        }
    }
    
    return base;
}

// Función auxiliar para procesar PrimaryTail (llamadas a función)
ASTNode* process_primary_tail(ASTNode* base, CSTNode* primary_tail)
{
    char* func_name = NULL;
    ASTNode** args = NULL;
    int arg_count = 0;
    
    DEBUG("Procesando PrimaryTail con %d hijos\n", primary_tail->child_count);
    
    if (base && base->type == NODE_VARIABLE) {
        func_name = strdup(base->data.variable_name);
        DEBUG("Nombre de función: %s\n", func_name);
        // NO liberar el nodo base aquí, se liberará en free_ast
    }
    
    // Buscar LPAREN ArgList RPAREN
    for (int i = 0; i < primary_tail->child_count; i++) {
        CSTNode* child = primary_tail->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de PrimaryTail: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "ArgList")) {
            args = process_arg_list(child, &arg_count);
            DEBUG("ArgList procesada con %d argumentos\n", arg_count);
        }
    }
    
    if (!func_name) {
        ERROR("PrimaryTail sin nombre de función\n");
        return NULL;
    }
    
    ASTNode* result = create_func_call_node(func_name, args, arg_count);
    if (args) free(args);
    return result;
}

// Función auxiliar para procesar producciones primed con operadores
ASTNode* process_primed_expression(CSTNode* primed_node, ASTNode* left_operand)
{
    if (!primed_node || !left_operand) return left_operand;
    
    DEBUG("Procesando expresión primed con %d hijos\n", primed_node->child_count);
    
    // Buscar operadores en la producción primed
    for (int i = 0; i < primed_node->child_count; i++) {
        CSTNode* child = primed_node->children[i];
        if (!child || strcmp(child->symbol, "ε") == 0) continue;
        
        DEBUG("Hijo %d de expresión primed: %s\n", i, child->symbol);
        
        // Buscar operadores
        if (child->token && (strcmp(child->token->value, "*") == 0 || 
                           strcmp(child->token->value, "/") == 0 || 
                           strcmp(child->token->value, "%") == 0 ||
                           strcmp(child->token->value, "+") == 0 || 
                           strcmp(child->token->value, "-") == 0)) {
            
            char* op_str = strdup(child->token->value);
            Operator op = OP_ADD;
            
            if (!strcmp(op_str, "*")) op = OP_MUL;
            else if (!strcmp(op_str, "/")) op = OP_DIV;
            else if (!strcmp(op_str, "%")) op = OP_MOD;
            else if (!strcmp(op_str, "+")) op = OP_ADD;
            else if (!strcmp(op_str, "-")) op = OP_SUB;
            
            DEBUG("Operador encontrado: %s\n", op_str);
            
            // Buscar el operando derecho
            ASTNode* right_operand = NULL;
            for (int j = i + 1; j < primed_node->child_count; j++) {
                if (primed_node->children[j] && strcmp(primed_node->children[j]->symbol, "ε") != 0) {
                    right_operand = expr_to_ast(primed_node->children[j]);
                    break;
                }
            }
            
            if (right_operand) {
                DEBUG("Operando derecho encontrado\n");
                return create_binary_op_node(op, op_str, left_operand, right_operand, &TYPE_OBJECT);
            }
        }
    }
    
    return left_operand;
}

// Función auxiliar para procesar ArgList
ASTNode** process_arg_list(CSTNode* arg_list, int* count)
{
    *count = 0;
    ASTNode** args = NULL;
    
    DEBUG("Procesando ArgList con %d hijos\n", arg_list->child_count);
    
    for (int i = 0; i < arg_list->child_count; i++) {
        CSTNode* child = arg_list->children[i];
        
        if (!child) continue;
        
        DEBUG("Hijo %d de ArgList: %s\n", i, child->symbol);
        
        if (!strcmp(child->symbol, "Expr")) {
            ASTNode* arg = expr_to_ast(child);
            if (arg) {
                args = realloc(args, (*count + 1) * sizeof(ASTNode*));
                args[*count] = arg;
                (*count)++;
                DEBUG("Argumento %d procesado\n", *count);
            }
        }
        // Ignorar comas y ArgListTail
    }
    
    return args;
}

