#include "cst_to_ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Función principal de conversión
Program* cst_to_ast_convert(DerivationNode* cst) {
    if (!cst || strcmp(derivation_node_get_symbol(cst), "Program") != 0) {
        fprintf(stderr, "Error: CST root must be a Program node\n");
        return NULL;
    }

    Program* program = create_program();
    if (!program) return NULL;

    // Program → StmtList
    if (cst->child_count > 0) {
        printf("DEBUG: Program tiene %zu hijos\n", cst->child_count);
        printf("DEBUG: Primer hijo es %s\n", derivation_node_get_symbol(cst->children[0]));
        
        Stmt** stmts = NULL;
        size_t stmt_count = 0;
        convert_stmt_list(cst->children[0], &stmts, &stmt_count);
        
        // Agregar statements al programa
        for (size_t i = 0; i < stmt_count; i++) {
            program_add_stmt(program, stmts[i]);
        }
        
        // Liberar el array de statements (los statements individuales son propiedad del programa)
        free(stmts);
    }

    printf("DEBUG: Program tiene %zu statements\n", program->stmt_count);
    return program;
}

// Conversión de lista de statements
void convert_stmt_list(DerivationNode* node, Stmt*** stmts, size_t* stmt_count) {
    if (!node || strcmp(derivation_node_get_symbol(node), "StmtList") != 0) {
        printf("DEBUG: StmtList - nodo inválido o no es StmtList\n");
        *stmts = NULL;
        *stmt_count = 0;
        return;
    }

    // StmtList → TerminatedStmt StmtList | ε
    if (node->child_count == 0) {
        printf("DEBUG: StmtList - ε production\n");
        *stmts = NULL;
        *stmt_count = 0;
        return; // ε production
    }

    printf("DEBUG: StmtList tiene %zu hijos\n", node->child_count);

    // Procesar el primer hijo (TerminatedStmt)
    Stmt** first_stmts = NULL;
    size_t first_count = 0;
    
    if (node->child_count >= 1) {
        DerivationNode* terminated_stmt = node->children[0];
        if (terminated_stmt && strcmp(derivation_node_get_symbol(terminated_stmt), "TerminatedStmt") == 0) {
            printf("DEBUG: Procesando TerminatedStmt\n");
            // TerminatedStmt → Stmt SEMICOLON
            if (terminated_stmt->child_count > 0) {
                DerivationNode* stmt = terminated_stmt->children[0];
                if (stmt) {
                    printf("DEBUG: Convirtiendo Stmt de tipo %s\n", derivation_node_get_symbol(stmt));
                    Stmt* ast_stmt = convert_stmt(stmt);
                    if (ast_stmt) {
                        printf("DEBUG: Statement convertido exitosamente\n");
                        first_stmts = (Stmt**)malloc(sizeof(Stmt*));
                        first_stmts[0] = ast_stmt;
                        first_count = 1;
                    } else {
                        printf("DEBUG: Error al convertir statement\n");
                    }
                }
            }
        }
    }

    // Procesar el segundo hijo (StmtList recursivo)
    Stmt** rest_stmts = NULL;
    size_t rest_count = 0;
    
    if (node->child_count >= 2) {
        printf("DEBUG: Procesando siguiente StmtList\n");
        convert_stmt_list(node->children[1], &rest_stmts, &rest_count);
    }

    // Combinar los resultados
    *stmt_count = first_count + rest_count;
    if (*stmt_count > 0) {
        *stmts = (Stmt**)malloc(sizeof(Stmt*) * (*stmt_count));
        
        // Copiar primeros statements
        for (size_t i = 0; i < first_count; i++) {
            (*stmts)[i] = first_stmts[i];
        }
        
        // Copiar statements restantes
        for (size_t i = 0; i < rest_count; i++) {
            (*stmts)[first_count + i] = rest_stmts[i];
        }
        
        free(first_stmts);
        free(rest_stmts);
    } else {
        *stmts = NULL;
    }
}

// Conversión de un statement individual
Stmt* convert_stmt(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Stmt") != 0) {
        printf("DEBUG: convert_stmt - nodo inválido o no es Stmt\n");
        return NULL;
    }

    printf("DEBUG: convert_stmt - procesando Stmt con %zu hijos\n", node->child_count);

    // Stmt → Expr | WhileStmt | ForStmt | BlockStmt | FunctionDef | TypeDef
    if (node->child_count > 0) {
        DerivationNode* child = node->children[0];
        if (!child) {
            printf("DEBUG: convert_stmt - primer hijo es null\n");
            return NULL;
        }

        const char* child_symbol = derivation_node_get_symbol(child);
        printf("DEBUG: convert_stmt - primer hijo es %s\n", child_symbol);

        Stmt* result = NULL;

        if (strcmp(child_symbol, "Expr") == 0) {
            printf("DEBUG: convert_stmt - convirtiendo Expr\n");
            Expr* expr = convert_expr(child);
            if (expr) {
                result = create_expr_stmt(expr);
            }
        } else if (strcmp(child_symbol, "FunctionDef") == 0) {
            result = convert_function_def(child);
        } else if (strcmp(child_symbol, "TypeDef") == 0) {
            result = convert_type_def(child);
        } else if (strcmp(child_symbol, "WhileStmt") == 0) {
            Expr* while_expr = convert_while_expr(child);
            if (while_expr) {
                result = create_expr_stmt(while_expr);
            }
        } else if (strcmp(child_symbol, "ForStmt") == 0) {
            Expr* for_expr = convert_for_expr(child);
            if (for_expr) {
                result = create_expr_stmt(for_expr);
            }
        } else if (strcmp(child_symbol, "BlockStmt") == 0) {
            Expr* block_expr = convert_block_stmt(child);
            if (block_expr) {
                result = create_expr_stmt(block_expr);
            }
        }

        // Establecer información de línea y columna
        if (result) {
            set_line_column_stmt(result, node);
        }

        return result;
    }

    printf("DEBUG: convert_stmt - no se pudo convertir el statement\n");
    return NULL;
}

// Conversión de una expresión
Expr* convert_expr(DerivationNode* node) {
    if (!node) {
        printf("DEBUG: convert_expr - nodo nulo\n");
        return NULL;
    }

    const char* symbol = derivation_node_get_symbol(node);
    printf("DEBUG: convert_expr - procesando nodo de tipo %s\n", symbol);

    if (strcmp(symbol, "Expr") == 0) {
        // Expr → OrExpr | IfExpr | LetExpr
        if (node->child_count > 0) {
            DerivationNode* child = node->children[0];
            if (!child) {
                printf("DEBUG: convert_expr - hijo nulo\n");
                return NULL;
            }

            const char* child_symbol = derivation_node_get_symbol(child);
            printf("DEBUG: convert_expr - procesando hijo de tipo %s\n", child_symbol);

            Expr* result = NULL;

            if (strcmp(child_symbol, "OrExpr") == 0) {
                result = convert_or_expr(child);
                if (result) {
                    printf("DEBUG: convert_expr - OrExpr convertido exitosamente\n");
                } else {
                    printf("DEBUG: convert_expr - error al convertir OrExpr\n");
                }
            } else if (strcmp(child_symbol, "IfExpr") == 0) {
                result = convert_if_expr(child);
            } else if (strcmp(child_symbol, "LetExpr") == 0) {
                printf("DEBUG: convert_expr - convirtiendo LetExpr\n");
                result = convert_let_expr(child);
            }

            // Establecer información de línea y columna
            if (result) {
                set_line_column_expr(result, node);
            }

            return result;
        }
    }

    printf("DEBUG: convert_expr - no se pudo convertir la expresión\n");
    return NULL;
}

// Conversión de expresiones OR
Expr* convert_or_expr(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "OrExpr") != 0) {
        return NULL;
    }

    // OrExpr → AndExpr OrExprPrime
    if (node->child_count >= 2) {
        Expr* left = convert_and_expr(node->children[0]);
        if (left) {
            return convert_or_expr_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_or_expr_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "OrExprPrime") != 0) {
        return inherited;
    }

    // OrExprPrime → OR AndExpr OrExprPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        if (strcmp(derivation_node_get_symbol(op_node), "OR") == 0) {
            Expr* right = convert_and_expr(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, BINARY_OP_OR);
                return convert_or_expr_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de expresiones AND
Expr* convert_and_expr(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "AndExpr") != 0) {
        return NULL;
    }

    // AndExpr → CmpExpr AndExprPrime
    if (node->child_count >= 2) {
        Expr* left = convert_cmp_expr(node->children[0]);
        if (left) {
            return convert_and_expr_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_and_expr_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "AndExprPrime") != 0) {
        return inherited;
    }

    // AndExprPrime → AND CmpExpr AndExprPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        if (strcmp(derivation_node_get_symbol(op_node), "AND") == 0) {
            Expr* right = convert_cmp_expr(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, BINARY_OP_AND);
                return convert_and_expr_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de expresiones de comparación
Expr* convert_cmp_expr(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "CmpExpr") != 0) {
        return NULL;
    }

    // CmpExpr → ConcatExpr CmpExprPrime
    if (node->child_count >= 2) {
        Expr* left = convert_concat_expr(node->children[0]);
        if (left) {
            return convert_cmp_expr_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_cmp_expr_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "CmpExprPrime") != 0) {
        return inherited;
    }

    // CmpExprPrime → (EQ|NEQ|LESS_THAN|GREATER_THAN|LE|GE) ConcatExpr CmpExprPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        const char* op_symbol = derivation_node_get_symbol(op_node);
        BinaryOp op = convert_binary_op(op_symbol);
        
        if (op != BINARY_OP_UNKNOWN) {
            Expr* right = convert_concat_expr(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, op);
                return convert_cmp_expr_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de expresiones de concatenación
Expr* convert_concat_expr(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "ConcatExpr") != 0) {
        return NULL;
    }

    // ConcatExpr → AddExpr ConcatExprPrime
    if (node->child_count >= 2) {
        Expr* left = convert_add_expr(node->children[0]);
        if (left) {
            return convert_concat_expr_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_concat_expr_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "ConcatExprPrime") != 0) {
        return inherited;
    }

    // ConcatExprPrime → (CONCAT|CONCAT_WS) AddExpr ConcatExprPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        const char* op_symbol = derivation_node_get_symbol(op_node);
        BinaryOp op = convert_binary_op(op_symbol);
        
        if (op != BINARY_OP_UNKNOWN) {
            Expr* right = convert_add_expr(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, op);
                return convert_concat_expr_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de expresiones de suma
Expr* convert_add_expr(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "AddExpr") != 0) {
        return NULL;
    }

    // AddExpr → Term AddExprPrime
    if (node->child_count >= 2) {
        Expr* left = convert_term(node->children[0]);
        if (left) {
            return convert_add_expr_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_add_expr_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "AddExprPrime") != 0) {
        return inherited;
    }

    // AddExprPrime → (PLUS|MINUS) Term AddExprPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        const char* op_symbol = derivation_node_get_symbol(op_node);
        BinaryOp op = convert_binary_op(op_symbol);
        
        if (op != BINARY_OP_UNKNOWN) {
            Expr* right = convert_term(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, op);
                return convert_add_expr_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de términos
Expr* convert_term(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Term") != 0) {
        return NULL;
    }

    // Term → Factor TermPrime
    if (node->child_count >= 2) {
        Expr* left = convert_factor(node->children[0]);
        if (left) {
            return convert_term_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_term_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "TermPrime") != 0) {
        return inherited;
    }

    // TermPrime → (MULT|DIV|MOD) Factor TermPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        const char* op_symbol = derivation_node_get_symbol(op_node);
        BinaryOp op = convert_binary_op(op_symbol);
        
        if (op != BINARY_OP_UNKNOWN) {
            Expr* right = convert_factor(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, op);
                return convert_term_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de factores
Expr* convert_factor(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Factor") != 0) {
        return NULL;
    }

    // Factor → Power FactorPrime
    if (node->child_count >= 2) {
        Expr* left = convert_power(node->children[0]);
        if (left) {
            return convert_factor_prime(left, node->children[1]);
        }
    }

    return NULL;
}

Expr* convert_factor_prime(Expr* inherited, DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "FactorPrime") != 0) {
        return inherited;
    }

    // FactorPrime → POW Power FactorPrime | ε
    if (node->child_count >= 3) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* right_node = node->children[1];
        DerivationNode* prime_node = node->children[2];

        if (strcmp(derivation_node_get_symbol(op_node), "POW") == 0) {
            Expr* right = convert_power(right_node);
            if (right) {
                BinaryExpr* binary = create_binary_expr(inherited, right, BINARY_OP_POW);
                return convert_factor_prime((Expr*)binary, prime_node);
            }
        }
    }

    return inherited;
}

// Conversión de potencias
Expr* convert_power(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Power") != 0) {
        return NULL;
    }

    // Power → Unary
    if (node->child_count >= 1) {
        return convert_unary(node->children[0]);
    }

    return NULL;
}

// Conversión de expresiones unarias
Expr* convert_unary(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Unary") != 0) {
        return NULL;
    }

    // Unary → (PLUS|MINUS|NOT) Unary | Primary
    if (node->child_count >= 2) {
        DerivationNode* op_node = node->children[0];
        DerivationNode* operand_node = node->children[1];

        const char* op_symbol = derivation_node_get_symbol(op_node);
        UnaryOp op = convert_unary_op(op_symbol);
        
        if (op != UNARY_OP_UNKNOWN) {
            Expr* operand = convert_unary(operand_node);
            if (operand) {
                return (Expr*)create_unary_expr(operand, op);
            }
        }
    } else if (node->child_count >= 1) {
        // Unary → Primary
        return convert_primary(node->children[0]);
    }

    return NULL;
}

// Conversión de expresiones primarias
Expr* convert_primary(DerivationNode* node) {
    if (!node || strcmp(derivation_node_get_symbol(node), "Primary") != 0) {
        return NULL;
    }

    // Primary → PrimaryTail
    if (node->child_count >= 1) {
        Expr* base = convert_primary_tail(NULL, node->children[0]);
        return base;
    }

    return NULL;
}

// Conversión de cola de expresiones primarias
Expr* convert_primary_tail(Expr* base, DerivationNode* node) {
    if (!node) return base;

    const char* symbol = derivation_node_get_symbol(node);
    
    if (strcmp(symbol, "PrimaryTail") == 0) {
        // PrimaryTail → DOT IDENT PrimaryTail | LPAREN ArgList RPAREN PrimaryTail | ε
        if (node->child_count >= 2) {
            DerivationNode* first_child = node->children[0];
            const char* first_symbol = derivation_node_get_symbol(first_child);
            
            if (strcmp(first_symbol, "DOT") == 0 && node->child_count >= 3) {
                // PrimaryTail → DOT IDENT PrimaryTail
                DerivationNode* ident_node = node->children[1];
                DerivationNode* tail_node = node->child_count >= 3 ? node->children[2] : NULL;
                
                const char* attr_name = get_token_value(ident_node);
                if (attr_name) {
                    GetAttrExpr* get_attr = create_get_attr_expr(base, attr_name);
                    return convert_primary_tail((Expr*)get_attr, tail_node);
                }
            } else if (strcmp(first_symbol, "LPAREN") == 0 && node->child_count >= 4) {
                // PrimaryTail → LPAREN ArgList RPAREN PrimaryTail
                DerivationNode* arg_list_node = node->children[1];
                DerivationNode* tail_node = node->child_count >= 4 ? node->children[3] : NULL;
                
                Expr** args = NULL;
                size_t arg_count = 0;
                convert_arg_list(arg_list_node, &args, &arg_count);
                
                if (base) {
                    // Es una llamada a método
                    const char* method_name = "unknown"; // Necesitaríamos obtener el nombre del método
                    MethodCallExpr* method_call = create_method_call_expr(base, method_name, args, arg_count);
                    return convert_primary_tail((Expr*)method_call, tail_node);
                } else {
                    // Es una llamada a función
                    const char* func_name = "unknown"; // Necesitaríamos obtener el nombre de la función
                    CallExpr* call_expr = create_call_expr(func_name, args, arg_count);
                    return convert_primary_tail((Expr*)call_expr, tail_node);
                }
            }
        }
    } else if (is_terminal_node(node)) {
        // Es un terminal, crear la expresión base
        const char* token_value = get_token_value(node);
        if (token_value) {
            if (strcmp(symbol, "IDENT") == 0) {
                return (Expr*)create_variable_expr(token_value);
            } else if (strcmp(symbol, "NUMBER") == 0) {
                double value = atof(token_value);
                return (Expr*)create_number_expr(value);
            } else if (strcmp(symbol, "STRING") == 0) {
                return (Expr*)create_string_expr(token_value);
            } else if (strcmp(symbol, "TRUE") == 0) {
                return (Expr*)create_bool_expr(true);
            } else if (strcmp(symbol, "FALSE") == 0) {
                return (Expr*)create_bool_expr(false);
            }
        }
    }

    return base;
}

// Funciones auxiliares
char* get_token_value(DerivationNode* node) {
    if (!node || !node->token) return NULL;
    return node->token->lexeme;
}

bool is_terminal_node(DerivationNode* node) {
    if (!node) return false;
    return node->token != NULL;
}

BinaryOp convert_binary_op(const char* op) {
    if (!op) return BINARY_OP_UNKNOWN;
    
    if (strcmp(op, "PLUS") == 0) return BINARY_OP_PLUS;
    if (strcmp(op, "MINUS") == 0) return BINARY_OP_MINUS;
    if (strcmp(op, "MULT") == 0) return BINARY_OP_MULT;
    if (strcmp(op, "DIV") == 0) return BINARY_OP_DIV;
    if (strcmp(op, "MOD") == 0) return BINARY_OP_MOD;
    if (strcmp(op, "POW") == 0) return BINARY_OP_POW;
    if (strcmp(op, "CONCAT") == 0) return BINARY_OP_CONCAT;
    if (strcmp(op, "CONCAT_WS") == 0) return BINARY_OP_CONCAT_WS;
    if (strcmp(op, "EQ") == 0) return BINARY_OP_EQ;
    if (strcmp(op, "NEQ") == 0) return BINARY_OP_NEQ;
    if (strcmp(op, "LESS_THAN") == 0) return BINARY_OP_LESS_THAN;
    if (strcmp(op, "GREATER_THAN") == 0) return BINARY_OP_GREATER_THAN;
    if (strcmp(op, "LE") == 0) return BINARY_OP_LE;
    if (strcmp(op, "GE") == 0) return BINARY_OP_GE;
    if (strcmp(op, "AND") == 0) return BINARY_OP_AND;
    if (strcmp(op, "OR") == 0) return BINARY_OP_OR;
    
    return BINARY_OP_UNKNOWN;
}

UnaryOp convert_unary_op(const char* op) {
    if (!op) return UNARY_OP_UNKNOWN;
    
    if (strcmp(op, "PLUS") == 0) return UNARY_OP_PLUS;
    if (strcmp(op, "MINUS") == 0) return UNARY_OP_MINUS;
    if (strcmp(op, "NOT") == 0) return UNARY_OP_NOT;
    
    return UNARY_OP_UNKNOWN;
}

// Funciones stub para las conversiones que no están implementadas completamente
FunctionDecl* convert_function_def(DerivationNode* node) {
    // TODO: Implementar conversión de definición de función
    printf("DEBUG: convert_function_def - no implementado\n");
    return NULL;
}

TypeDecl* convert_type_def(DerivationNode* node) {
    // TODO: Implementar conversión de definición de tipo
    printf("DEBUG: convert_type_def - no implementado\n");
    return NULL;
}

ExprStmt* convert_expr_stmt(DerivationNode* node) {
    // TODO: Implementar conversión de statement de expresión
    printf("DEBUG: convert_expr_stmt - no implementado\n");
    return NULL;
}

LetExpr* convert_let_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión let
    printf("DEBUG: convert_let_expr - no implementado\n");
    return NULL;
}

IfExpr* convert_if_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión if
    printf("DEBUG: convert_if_expr - no implementado\n");
    return NULL;
}

WhileExpr* convert_while_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión while
    printf("DEBUG: convert_while_expr - no implementado\n");
    return NULL;
}

ForExpr* convert_for_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión for
    printf("DEBUG: convert_for_expr - no implementado\n");
    return NULL;
}

CallExpr* convert_call_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión de llamada
    printf("DEBUG: convert_call_expr - no implementado\n");
    return NULL;
}

NewExpr* convert_new_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión new
    printf("DEBUG: convert_new_expr - no implementado\n");
    return NULL;
}

IsExpr* convert_is_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión is
    printf("DEBUG: convert_is_expr - no implementado\n");
    return NULL;
}

AsExpr* convert_as_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión as
    printf("DEBUG: convert_as_expr - no implementado\n");
    return NULL;
}

BaseCallExpr* convert_base_call_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión base call
    printf("DEBUG: convert_base_call_expr - no implementado\n");
    return NULL;
}

AssignExpr* convert_assign_expr(DerivationNode* node) {
    // TODO: Implementar conversión de expresión de asignación
    printf("DEBUG: convert_assign_expr - no implementado\n");
    return NULL;
}

ExprBlock* convert_block_stmt(DerivationNode* node) {
    // TODO: Implementar conversión de bloque de expresión
    printf("DEBUG: convert_block_stmt - no implementado\n");
    return NULL;
}

// Funciones auxiliares para argumentos
void convert_arg_list(DerivationNode* node, Expr*** args, size_t* count) {
    // TODO: Implementar conversión de lista de argumentos
    printf("DEBUG: convert_arg_list - no implementado\n");
    *args = NULL;
    *count = 0;
}

// Funciones auxiliares para información de línea y columna
void get_line_column(DerivationNode* node, int* line, int* column) {
    if (!node) {
        *line = 0;
        *column = 0;
        return;
    }
    *line = node->line_number;
    *column = node->column_number;
}

void set_line_column_expr(Expr* expr, DerivationNode* node) {
    if (!expr || !node) return;
    int line, column;
    get_line_column(node, &line, &column);
    expr->line_number = line;
    expr->column_number = column;
}

void set_line_column_stmt(Stmt* stmt, DerivationNode* node) {
    if (!stmt || !node) return;
    int line, column;
    get_line_column(node, &line, &column);
    stmt->line_number = line;
    stmt->column_number = column;
} 