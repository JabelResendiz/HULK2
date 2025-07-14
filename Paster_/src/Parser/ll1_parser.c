#include "ll1_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Implementación simplificada del parser LL1

LL1Parser* ll1_parser_create(Token* tokens, int token_count) {
    LL1Parser* parser = (LL1Parser*)malloc(sizeof(LL1Parser));
    if (!parser) return NULL;
    
    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->current_token = 0;
    parser->max_tokens = token_count;
    
    return parser;
}

LL1Parser* ll1_parser_create_with_errors(Token* tokens, int token_count, ErrorContext* error_context) {
    LL1Parser* parser = ll1_parser_create(tokens, token_count);
    if (!parser) return NULL;
    
    parser->error_context = error_context;
    return parser;
}

void ll1_parser_destroy(LL1Parser* parser) {
    if (parser) {
        free(parser);
    }
}

Token* ll1_parser_peek(LL1Parser* parser) {
    if (!parser) return NULL;
    if (parser->current_token >= parser->token_count) return NULL;
    return &parser->tokens[parser->current_token];
}

Token* ll1_parser_advance(LL1Parser* parser) {
    if (!parser) return NULL;
    if (parser->current_token >= parser->token_count) return NULL;
    return &parser->tokens[parser->current_token++];
}

bool ll1_parser_match(LL1Parser* parser, TokenType token_type) {
    Token* current = ll1_parser_peek(parser);
    if (!current) return false;
    return current->type == token_type;
}

bool ll1_parser_is_at_end(LL1Parser* parser) {
    return parser->current_token >= parser->token_count;
}

// Helper: lookahead for next token type
bool ll1_parser_match_ahead(LL1Parser* parser, TokenType token_type) {
    if (!parser) return false;
    int next = parser->current_token + 1;
    if (next >= parser->token_count) return false;
    return parser->tokens[next].type == token_type;
}

// Función principal de parsing
Program* ll1_parser_parse(LL1Parser* parser) {
    if (!parser) return NULL;
    
    Program* program = create_program();
    if (!program) return NULL;
    
    // Parsear statements hasta el final
    int max_iterations = 1000; // Prevenir bucle infinito
    int iteration = 0;
    
    while (!ll1_parser_is_at_end(parser) && iteration < max_iterations) {
        Token* current = ll1_parser_peek(parser);
        if (!current) {
            fprintf(stderr, "DEBUG: No current token, advancing\n");
            ll1_parser_advance(parser);
            iteration++;
            continue;
        }
        
        fprintf(stderr, "DEBUG: Parsing statement %d, current token: %s\n", 
                iteration, current->lexeme ? current->lexeme : get_token_name(current->type));
        
        Stmt* stmt = ll1_parse_statement(parser);
        if (stmt) {
            // LOG: Tipo de nodo agregado al AST
            fprintf(stderr, "[AST-ADD] Statement agregado: tipo=%d\n", ((ASTNode*)stmt)->type);
            program_add_stmt(program, stmt);
            fprintf(stderr, "DEBUG: Statement parsed successfully\n");
        } else {
            // Error en parsing, continuar con el siguiente token
            fprintf(stderr, "DEBUG: Failed to parse statement, advancing\n");
            ll1_parser_advance(parser);
        }
        
        iteration++;
    }
    
    if (iteration >= max_iterations) {
        fprintf(stderr, "DEBUG: Maximum iterations reached, stopping parser\n");
    }
    
    return program;
}

// Parsing de bloques
Stmt* ll1_parse_block(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: [BLOCK] Iniciando ll1_parse_block\n");
    
    Token* current = ll1_parser_peek(parser);
    if (current) {
        fprintf(stderr, "DEBUG: [BLOCK] Token actual al inicio: tipo=%d (%s), lexeme='%s'\n", 
                current->type, get_token_name(current->type), 
                current->lexeme ? current->lexeme : "NULL");
    }
    
    // Determinar el token de cierre basado en el contexto
    TokenType close_token = RBRACE; // Por defecto
    if (ll1_parser_match(parser, LBRACE)) {
        close_token = RBRACE;
        fprintf(stderr, "DEBUG: [BLOCK] Parsing block with braces\n");
    } else if (ll1_parser_match(parser, LBRACKET)) {
        close_token = RBRACKET;
        fprintf(stderr, "DEBUG: [BLOCK] Parsing block with brackets\n");
    } else {
        fprintf(stderr, "DEBUG: [BLOCK] No opening brace/bracket found\n");
        return NULL;
    }
    
    fprintf(stderr, "DEBUG: [BLOCK] Token de cierre: %s\n", get_token_name(close_token));
    
    ll1_parser_advance(parser); // Consumir la llave de apertura
    
    Stmt** stmts = NULL;
    size_t stmt_count = 0;
    int loop_count = 0;
    const int MAX_LOOPS = 1000;
    
    while (!ll1_parser_is_at_end(parser) && !ll1_parser_match(parser, close_token) && loop_count < MAX_LOOPS) {
        Token* current = ll1_parser_peek(parser);
        if (!current) {
            fprintf(stderr, "DEBUG: No current token in block, advancing\n");
            ll1_parser_advance(parser);
            loop_count++;
            continue;
        }
        
        fprintf(stderr, "DEBUG: Block loop %d, current token: %s\n", 
                loop_count, current->lexeme ? current->lexeme : get_token_name(current->type));
        
        // Skip closing tokens that might be inside the block
        if (current->type == close_token) {
            fprintf(stderr, "DEBUG: Found closing token, ending block\n");
            break;
        }
        
        // Skip whitespace and other non-statement tokens
        if (current->type == SEMICOLON || current->type == NEWLINE || current->type == WHITESPACE) {
            fprintf(stderr, "DEBUG: Skipping non-statement token: %s\n", get_token_name(current->type));
            ll1_parser_advance(parser);
            loop_count++;
            continue;
        }
        
        // Intentar parsear un statement
        fprintf(stderr, "DEBUG: Attempting to parse statement in block\n");
        fprintf(stderr, "DEBUG: [BLOCK] Token actual: tipo=%d (%s), lexeme='%s'\n", 
                current->type, get_token_name(current->type), 
                current->lexeme ? current->lexeme : "NULL");
        
        // Verificar el token antes de parsear
        Token* before_parse = ll1_parser_peek(parser);
        fprintf(stderr, "DEBUG: [BLOCK] Token antes de parsear: tipo=%d (%s), lexeme='%s'\n", 
                before_parse->type, get_token_name(before_parse->type), 
                before_parse->lexeme ? before_parse->lexeme : "NULL");
        
        Stmt* stmt = ll1_parse_statement(parser);
        
        // Verificar el token después de parsear
        Token* after_parse = ll1_parser_peek(parser);
        fprintf(stderr, "DEBUG: [BLOCK] Token después de parsear: tipo=%d (%s), lexeme='%s'\n", 
                after_parse->type, get_token_name(after_parse->type), 
                after_parse->lexeme ? after_parse->lexeme : "NULL");
        
        if (stmt) {
            stmts = realloc(stmts, sizeof(Stmt*) * (stmt_count + 1));
            stmts[stmt_count++] = stmt;
            fprintf(stderr, "DEBUG: Added statement to block\n");
        } else {
            fprintf(stderr, "DEBUG: Failed to parse statement in block, current token: %s\n", 
                    current ? get_token_name(current->type) : "NULL");
            fprintf(stderr, "DEBUG: [BLOCK] Falló al parsear statement con token: tipo=%d (%s), lexeme='%s'\n", 
                    current->type, get_token_name(current->type), 
                    current->lexeme ? current->lexeme : "NULL");
            
            // Si no se puede parsear el statement, reportar error y continuar
            if (parser->error_context) {
                parser_error_invalid_expression(parser, "block statement");
            }
            
            ll1_parser_advance(parser); // Evita bucle infinito
        }
        loop_count++;
    }
    
    if (ll1_parser_match(parser, close_token)) {
        ll1_parser_advance(parser);
        fprintf(stderr, "DEBUG: Block parsing completed with %zu statements\n", stmt_count);
    } else {
        fprintf(stderr, "DEBUG: No closing token found for block\n");
        if (parser->error_context) {
            parser_error_missing_token(parser, close_token);
        }
    }
    
    return create_block_stmt(stmts, stmt_count);
}

// Parsing de return
Stmt* ll1_parse_return_statement(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting return statement parsing\n");
    
    if (!ll1_parser_match(parser, RETURN)) {
        if (parser->error_context) {
            parser_error_missing_token(parser, RETURN);
        }
        fprintf(stderr, "DEBUG: No RETURN token found\n");
        return NULL;
    }
    ll1_parser_advance(parser);
    fprintf(stderr, "DEBUG: Found RETURN token, parsing expression\n");
    
    // Verificar qué token viene después del return
    Token* next_token = ll1_parser_peek(parser);
    if (next_token) {
        fprintf(stderr, "DEBUG: Next token after RETURN: type=%d (%s), lexeme='%s'\n", 
                next_token->type, get_token_name(next_token->type), 
                next_token->lexeme ? next_token->lexeme : "NULL");
    }
    
    Expr* value = ll1_parse_expression(parser);
    if (!value) {
        if (parser->error_context) {
            parser_error_invalid_expression(parser, "return value");
        }
        fprintf(stderr, "DEBUG: Failed to parse return expression\n");
        return NULL;
    }
    // Consumir punto y coma opcional
    if (ll1_parser_match(parser, SEMICOLON)) {
        ll1_parser_advance(parser);
    }
    fprintf(stderr, "DEBUG: Return statement parsed successfully\n");
    return create_return_stmt_wrapper(value);
}

// Parsing de asignaciones
Stmt* ll1_parse_assignment(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting assignment parsing\n");
    
    if (!ll1_parser_match(parser, IDENT)) {
        if (parser->error_context) {
            parser_error_missing_token(parser, IDENT);
        }
        fprintf(stderr, "DEBUG: No IDENT token found\n");
        return NULL;
    }
    Token* id_token = ll1_parser_advance(parser);
    fprintf(stderr, "DEBUG: Assignment target: %s\n", id_token->lexeme);
    
    if (!ll1_parser_match(parser, ASSIGN)) {
        if (parser->error_context) {
            parser_error_missing_token(parser, ASSIGN);
        }
        fprintf(stderr, "DEBUG: No ASSIGN token found\n");
        return NULL;
    }
    ll1_parser_advance(parser);
    
    Expr* value = ll1_parse_expression(parser);
    if (!value) {
        if (parser->error_context) {
            parser_error_invalid_expression(parser, "assignment value");
        }
        fprintf(stderr, "DEBUG: Failed to parse assignment value\n");
        return NULL;
    }
    // Consumir punto y coma opcional
    if (ll1_parser_match(parser, SEMICOLON)) {
        ll1_parser_advance(parser);
    }
    fprintf(stderr, "DEBUG: Assignment parsed successfully\n");
    return create_assign_expr(id_token->lexeme, value);
}

// Parsing de let-in
Stmt* ll1_parse_let_in(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting let-in parsing\n");
    if (!ll1_parser_match(parser, LET)) return NULL;
    ll1_parser_advance(parser);
    
    // Parsear definiciones (pueden ser varias)
    Stmt** bindings = NULL;
    size_t binding_count = 0;
    int loop_count = 0;
    const int MAX_LOOPS = 100; // Prevenir bucle infinito
    
    do {
        fprintf(stderr, "DEBUG: Let-in loop iteration %d\n", loop_count);
        Token* current = ll1_parser_peek(parser);
        if (current) {
            fprintf(stderr, "DEBUG: Current token in let-in: %s\n", 
                    current->lexeme ? current->lexeme : get_token_name(current->type));
        }
        
        Stmt* binding = ll1_parse_assignment(parser);
        if (binding) {
            fprintf(stderr, "DEBUG: Binding parsed successfully\n");
            bindings = realloc(bindings, sizeof(Stmt*) * (binding_count + 1));
            bindings[binding_count++] = binding;
        } else {
            fprintf(stderr, "DEBUG: Failed to parse binding\n");
        }
        
        if (ll1_parser_match(parser, COMMA)) {
            fprintf(stderr, "DEBUG: Found comma, advancing\n");
            ll1_parser_advance(parser);
        }
        
        loop_count++;
        if (loop_count > MAX_LOOPS) {
            fprintf(stderr, "DEBUG: Maximum loop count reached, breaking\n");
            break;
        }
    } while (!ll1_parser_match(parser, IN) && !ll1_parser_is_at_end(parser));
    
    fprintf(stderr, "DEBUG: Let-in loop finished, looking for IN\n");
    if (!ll1_parser_match(parser, IN)) {
        fprintf(stderr, "DEBUG: IN token not found\n");
        return NULL;
    }
    
    fprintf(stderr, "DEBUG: Found IN token, advancing\n");
    ll1_parser_advance(parser);
    
    fprintf(stderr, "DEBUG: Parsing let-in body\n");
    Expr* body = ll1_parse_expression(parser);
    if (!body) {
        fprintf(stderr, "DEBUG: Failed to parse let-in body\n");
        return NULL;
    }
    // Consumir punto y coma opcional
    if (ll1_parser_match(parser, SEMICOLON)) {
        ll1_parser_advance(parser);
    }
    fprintf(stderr, "DEBUG: Creating let-in node\n");
    return (Stmt*)create_let_in_node((ASTNode**)bindings, binding_count, (ASTNode*)body);
}

// Parsing de declaraciones de función
Stmt* ll1_parse_function_decl(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting function declaration parsing\n");
    if (!ll1_parser_match(parser, FUNCTION)) return NULL;
    ll1_parser_advance(parser);
    
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* func_token = ll1_parser_advance(parser);
    fprintf(stderr, "DEBUG: Function name: %s\n", func_token->lexeme);
    
    // Parsear parámetros con anotaciones de tipo opcionales
    char** params = NULL;
    size_t param_count = 0;
    TypeValue** param_types = NULL;
    
    if (ll1_parser_match(parser, LPAREN)) {
        ll1_parser_advance(parser);
        if (!ll1_parser_match(parser, RPAREN)) {
            do {
                if (ll1_parser_match(parser, IDENT)) {
                    Token* param_token = ll1_parser_advance(parser);
                    params = realloc(params, sizeof(char*) * (param_count + 1));
                    params[param_count] = strdup(param_token->lexeme);
                    
                    // Parsear anotación de tipo opcional
                    TypeValue* param_type = ll1_parse_type_annotation(parser);
                    param_types = realloc(param_types, sizeof(TypeValue*) * (param_count + 1));
                    param_types[param_count] = param_type;
                    
                    fprintf(stderr, "DEBUG: Parameter: %s\n", param_token->lexeme);
                    param_count++;
                }
            } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
        }
        if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    }
    
    // Parsear anotación de tipo de retorno opcional
    TypeValue* return_type = NULL;
    if (ll1_parser_match(parser, COLON)) {
        return_type = ll1_parse_type_annotation(parser);
        fprintf(stderr, "DEBUG: Return type: %s\n", return_type ? return_type->name : "inferred");
    }
    
    // Parsear el cuerpo de la función
    Stmt* body = NULL;
    
    // Verificar si hay un bloque con llaves
    if (ll1_parser_match(parser, LBRACE)) {
        fprintf(stderr, "DEBUG: Found function body block with braces\n");
        
        // Verificar el token después de consumir la llave de apertura
        Token* after_lbrace = ll1_parser_peek(parser);
        fprintf(stderr, "DEBUG: [FUNCTION] Token después de LBRACE: tipo=%d (%s), lexeme='%s'\n", 
                after_lbrace->type, get_token_name(after_lbrace->type), 
                after_lbrace->lexeme ? after_lbrace->lexeme : "NULL");
        
        // Parsear el bloque (NO consumir la llave de apertura aquí)
        body = ll1_parse_block(parser);
        if (!body) {
            if (parser->error_context) {
                parser_error_invalid_expression(parser, "function body block");
            }
            fprintf(stderr, "DEBUG: Failed to parse function body block\n");
            return NULL;
        }
    } else if (ll1_parser_match(parser, ARROW)) {
        // Función de una línea con arrow syntax
        ll1_parser_advance(parser);
        Expr* body_expr = ll1_parse_expression(parser);
        if (body_expr) {
            body = create_expr_stmt(body_expr);
            fprintf(stderr, "DEBUG: Successfully parsed function body as expression\n");
        } else {
            if (parser->error_context) {
                parser_error_invalid_expression(parser, "function body expression");
            }
            fprintf(stderr, "DEBUG: Failed to parse function body expression\n");
            return NULL;
        }
    } else {
        // Intentar parsear como una expresión simple (para funciones de una línea)
        fprintf(stderr, "DEBUG: Parsing function body as simple expression\n");
        Expr* body_expr = ll1_parse_expression(parser);
        if (body_expr) {
            body = create_expr_stmt(body_expr);
            fprintf(stderr, "DEBUG: Successfully parsed function body as expression\n");
        } else {
            if (parser->error_context) {
                parser_error_invalid_expression(parser, "function body expression");
            }
            fprintf(stderr, "DEBUG: Failed to parse function body expression\n");
            return NULL;
        }
    }
    
    fprintf(stderr, "DEBUG: Function body parsed successfully\n");
    
    // Duplicar el nombre de la función para evitar problemas de memoria
    char* func_name = func_token->lexeme ? strdup(func_token->lexeme) : NULL;
    
    fprintf(stderr, "DEBUG: Creating function declaration with name: %s, params: %zu, body: %p\n", 
            func_name, param_count, body);
    
    FunctionDecl* func_decl = create_function_decl(func_name, params, param_count, body, param_types, return_type);
    
    // Liberar memoria de parámetros
    for (size_t i = 0; i < param_count; i++) {
        free(params[i]);
    }
    free(params);
    
    // Convertir FunctionDecl* a Stmt* - ambos son ASTNode* internamente
    return (Stmt*)func_decl;
}

// Parsing de if
Stmt* ll1_parse_if_statement(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting if statement parsing\n");
    if (!ll1_parser_match(parser, IF)) return NULL;
    ll1_parser_advance(parser);
    
    fprintf(stderr, "DEBUG: Looking for opening parenthesis after IF\n");
    Token* current = ll1_parser_peek(parser);
    if (current) {
        fprintf(stderr, "DEBUG: Current token after IF: type=%d (%s), lexeme='%s'\n", 
                current->type, get_token_name(current->type), current->lexeme ? current->lexeme : "NULL");
    } else {
        fprintf(stderr, "DEBUG: No current token after IF\n");
    }
    
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    Expr* cond = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, RPAREN)) return NULL;
    ll1_parser_advance(parser);
    
    fprintf(stderr, "DEBUG: Parsing if then body\n");
    Stmt* then_body = ll1_parse_statement(parser);
    Stmt* else_body = NULL;
    
    if (ll1_parser_match(parser, ELSE)) {
        fprintf(stderr, "DEBUG: Found else, parsing else body\n");
        ll1_parser_advance(parser);
        else_body = ll1_parse_statement(parser);
    }
    
    fprintf(stderr, "DEBUG: Creating if expression\n");
    return (Stmt*)create_if_expr(cond, then_body, else_body);
}

// Parsing de while
Stmt* ll1_parse_while_statement(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting while statement parsing\n");
    if (!ll1_parser_match(parser, WHILE)) return NULL;
    ll1_parser_advance(parser);
    
    fprintf(stderr, "DEBUG: Looking for opening parenthesis after WHILE\n");
    Token* current = ll1_parser_peek(parser);
    if (current) {
        fprintf(stderr, "DEBUG: Current token after WHILE: type=%d (%s), lexeme='%s'\n", 
                current->type, get_token_name(current->type), current->lexeme ? current->lexeme : "NULL");
    } else {
        fprintf(stderr, "DEBUG: No current token after WHILE\n");
    }
    
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    Expr* cond = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, RPAREN)) return NULL;
    ll1_parser_advance(parser);
    
    fprintf(stderr, "DEBUG: Parsing while body\n");
    Stmt* body = ll1_parse_statement(parser);
    
    fprintf(stderr, "DEBUG: Creating while expression\n");
    return (Stmt*)create_while_expr(cond, body);
}

// Parsing de for-loops
Stmt* ll1_parse_for_statement(LL1Parser* parser) {
    if (!ll1_parser_match(parser, FOR)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* var_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IN)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, RANGE)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    // Parsear lista de argumentos para el rango
    Expr** range_args = NULL;
    size_t range_arg_count = 0;
    if (!ll1_parser_match(parser, RPAREN)) {
        do {
            Expr* arg = ll1_parse_expression(parser);
            if (arg) {
                range_args = realloc(range_args, sizeof(Expr*) * (range_arg_count + 1));
                range_args[range_arg_count++] = arg;
            }
        } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
    }
    if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    Stmt* body = ll1_parse_statement(parser);
    return (Stmt*)create_for_loop_node(var_token->lexeme, range_args, (ASTNode*)body, range_arg_count);
}

// Parsing de declaraciones destructivas (:=)
Stmt* ll1_parse_destructive_decl(LL1Parser* parser) {
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* id_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, DEQUALS)) return NULL; // :=
    ll1_parser_advance(parser);
    Expr* value = ll1_parse_expression(parser);
    return (Stmt*)create_assign_expr(id_token->lexeme, value);
}

// Parsing de setters (obj.attr := value)
Stmt* ll1_parse_setter(LL1Parser* parser) {
    // Parse object expression but don't use it for now
    ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, DOT)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* attr_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, DEQUALS)) return NULL;
    ll1_parser_advance(parser);
    Expr* value = ll1_parse_expression(parser);
    // For now, just create a simple assignment
    return (Stmt*)create_assign_expr(attr_token->lexeme, value);
}

// Parsing de operadores compuestos (+=, -=, etc.)
Stmt* ll1_parse_compound_operator(LL1Parser* parser) {
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* id_token = ll1_parser_advance(parser);
    Token* op_token = ll1_parser_peek(parser);
    if (!op_token) return NULL;
    
    // Determinar el operador compuesto
    BinaryOp op_type = BINARY_OP_UNKNOWN;
    switch (op_token->type) {
        case PLUSEQUAL: op_type = BINARY_OP_ADD; break;
        case MINUSEQUAL: op_type = BINARY_OP_SUB; break;
        case TIMESEQUAL: op_type = BINARY_OP_MUL; break;
        case DIVEQUAL: op_type = BINARY_OP_DIV_OP; break;
        case MODEQUAL: op_type = BINARY_OP_MOD; break;
        case POWEQUAL: op_type = BINARY_OP_POW; break;
        case CONCATEQUAL: op_type = BINARY_OP_CONCAT; break;
        case ANDEQUAL: op_type = BINARY_OP_AND; break;
        case OREQUAL: op_type = BINARY_OP_OR; break;
        default: return NULL;
    }
    ll1_parser_advance(parser); // Consumir el operador compuesto
    Expr* value = ll1_parse_expression(parser);
    
    // Crear la expresión compuesta: id = id op value
    Expr* var_expr = create_variable_expr(id_token->lexeme);
    Expr* binary_expr = create_binary_expr(var_expr, value, op_type);
    return (Stmt*)create_assign_expr(id_token->lexeme, binary_expr);
}

// Parsing de acceso a atributos (obj.attr)
Expr* ll1_parse_get_attr(LL1Parser* parser) {
    Expr* obj = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, DOT)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* attr_token = ll1_parser_advance(parser);
    return create_get_attr_expr(obj, attr_token->lexeme);
}

// Parsing de llamadas a métodos (obj.method(...))
Expr* ll1_parse_method_call(LL1Parser* parser) {
    Expr* obj = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, DOT)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* method_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    
    // Parsear argumentos
    Expr** args = NULL;
    size_t arg_count = 0;
    if (!ll1_parser_match(parser, RPAREN)) {
        do {
            Expr* arg = ll1_parse_expression(parser);
            if (arg) {
                args = realloc(args, sizeof(Expr*) * (arg_count + 1));
                args[arg_count++] = arg;
            }
        } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
    }
    if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    
    return create_method_call_expr(obj, method_token->lexeme, args, arg_count);
}

// Parsing de expresiones ternarias (cond ? then : else)
Expr* ll1_parse_ternary(LL1Parser* parser) {
    Expr* condition = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, QUESTION)) return condition;
    ll1_parser_advance(parser);
    Expr* then_expr = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, COLON)) return condition;
    ll1_parser_advance(parser);
    Expr* else_expr = ll1_parse_expression(parser);
    // Crear una expresión condicional como ternaria
    return create_if_expr(condition, (Stmt*)then_expr, (Stmt*)else_expr);
}

// Parsing de new (new Type(...))
Expr* ll1_parse_new(LL1Parser* parser) {
    if (!ll1_parser_match(parser, NEW)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* type_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    
    // Parsear argumentos
    Expr** args = NULL;
    size_t arg_count = 0;
    if (!ll1_parser_match(parser, RPAREN)) {
        do {
            Expr* arg = ll1_parse_expression(parser);
            if (arg) {
                args = realloc(args, sizeof(Expr*) * (arg_count + 1));
                args[arg_count++] = arg;
            }
        } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
    }
    if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    
    return create_new_expr(type_token->lexeme, args, arg_count);
}

// Parsing de type declarations
Stmt* ll1_parse_type_decl(LL1Parser* parser) {
    if (!ll1_parser_match(parser, TYPE)) return NULL;
    ll1_parser_advance(parser);
    
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* type_token = ll1_parser_advance(parser);
    
    // Parse inheritance
    char* parent_type = NULL;
    if (ll1_parser_match(parser, INHERITS)) {
        ll1_parser_advance(parser);
        if (ll1_parser_match(parser, IDENT)) {
            Token* parent_token = ll1_parser_advance(parser);
            parent_type = strdup(parent_token->lexeme);
        }
    }
    
    // Parse constructor parameters
    ASTNode** args = NULL;
    int args_count = 0;
    if (ll1_parser_match(parser, LPAREN)) {
        ll1_parser_advance(parser);
        if (!ll1_parser_match(parser, RPAREN)) {
            do {
                if (ll1_parser_match(parser, IDENT)) {
                    Token* param_token = ll1_parser_advance(parser);
                    args = realloc(args, sizeof(ASTNode*) * (args_count + 1));
                    args[args_count++] = create_var_node(strdup(param_token->lexeme), NULL, 0);
                }
            } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
        }
        if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    }
    
    // Parse body
    ASTNode** body_elements = NULL;
    int body_count = 0;
    if (ll1_parser_match(parser, LBRACE)) {
        ll1_parser_advance(parser);
        while (!ll1_parser_is_at_end(parser) && !ll1_parser_match(parser, RBRACE)) {
            Stmt* stmt = ll1_parse_statement(parser);
            if (stmt) {
                body_elements = realloc(body_elements, sizeof(ASTNode*) * (body_count + 1));
                body_elements[body_count++] = (ASTNode*)stmt;
            } else {
                ll1_parser_advance(parser);
            }
        }
        if (ll1_parser_match(parser, RBRACE)) ll1_parser_advance(parser);
    }
    
    return (Stmt*)create_type_node(
        strdup(type_token->lexeme),
        parent_type,
        args,
        args_count,
        NULL, // p_args
        0,    // p_args_count
        body_elements,
        body_count,
        0     // p_constructor
    );
}

// Parsing de casting (expr as Type)
Expr* ll1_parse_casting(LL1Parser* parser) {
    Expr* expr = ll1_parse_expression(parser);
    if (!ll1_parser_match(parser, AS)) return expr;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return expr;
    Token* type_token = ll1_parser_advance(parser);
    return create_as_expr(expr, type_token->lexeme);
}

// Parsing de declaraciones de atributos
Stmt* ll1_parse_attribute_decl(LL1Parser* parser) {
    if (!ll1_parser_match(parser, ATTRIBUTE)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* attr_token = ll1_parser_advance(parser);
    Expr* initializer = NULL;
    if (ll1_parser_match(parser, ASSIGN)) {
        ll1_parser_advance(parser);
        initializer = ll1_parse_expression(parser);
    }
    return (Stmt*)create_attribute_decl(attr_token->lexeme, initializer, NULL);
}

// Parsing de declaraciones de métodos
Stmt* ll1_parse_method_decl(LL1Parser* parser) {
    if (!ll1_parser_match(parser, METHOD)) return NULL;
    ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* method_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, LPAREN)) return NULL;
    ll1_parser_advance(parser);
    
    // Parsear parámetros
    char** params = NULL;
    size_t param_count = 0;
    if (!ll1_parser_match(parser, RPAREN)) {
        do {
            if (ll1_parser_match(parser, IDENT)) {
                Token* param_token = ll1_parser_advance(parser);
                params = realloc(params, sizeof(char*) * (param_count + 1));
                params[param_count++] = strdup(param_token->lexeme);
            }
        } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
    }
    if (ll1_parser_match(parser, RPAREN)) ll1_parser_advance(parser);
    
    Stmt* body = ll1_parse_statement(parser);
    return (Stmt*)create_method_decl(method_token->lexeme, params, param_count, body, NULL, NULL);
}

// Parsing de bloques de expresiones
Expr* ll1_parse_block_expr(LL1Parser* parser) {
    if (!ll1_parser_match(parser, LBRACKET)) return NULL;
    ll1_parser_advance(parser);
    Stmt** stmts = NULL;
    size_t stmt_count = 0;
    while (!ll1_parser_is_at_end(parser) && !ll1_parser_match(parser, RBRACKET)) {
        Stmt* stmt = ll1_parse_statement(parser);
        if (stmt) {
            stmts = realloc(stmts, sizeof(Stmt*) * (stmt_count + 1));
            stmts[stmt_count++] = stmt;
        } else {
            ll1_parser_advance(parser);
        }
    }
    if (ll1_parser_match(parser, RBRACKET)) ll1_parser_advance(parser);
    return (Expr*)create_expr_block(stmts, stmt_count);
}

// Parsing de destructores
Stmt* ll1_parse_destructor(LL1Parser* parser) {
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* id_token = ll1_parser_advance(parser);
    if (!ll1_parser_match(parser, DEQUALS)) return NULL;
    ll1_parser_advance(parser);
    Expr* value = ll1_parse_expression(parser);
    return (Stmt*)create_assign_expr(id_token->lexeme, value);
}

// Parsing de llamadas a función
Expr* ll1_parse_function_call(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting function call parsing\n");
    if (!ll1_parser_match(parser, IDENT)) {
        fprintf(stderr, "DEBUG: No IDENT token for function call\n");
        return NULL;
    }
    Token* func_token = ll1_parser_advance(parser);
    fprintf(stderr, "DEBUG: Function call name: %s\n", func_token->lexeme);
    
    if (!ll1_parser_match(parser, LPAREN)) {
        fprintf(stderr, "DEBUG: No opening parenthesis for function call\n");
        return NULL;
    }
    ll1_parser_advance(parser);
    fprintf(stderr, "DEBUG: Found opening parenthesis for function call\n");
    
    // Parsear argumentos
    Expr** args = NULL;
    size_t arg_count = 0;
    if (!ll1_parser_match(parser, RPAREN)) {
        do {
            fprintf(stderr, "DEBUG: Parsing function call argument\n");
            Expr* arg = ll1_parse_expression(parser);
            if (arg) {
                args = realloc(args, sizeof(Expr*) * (arg_count + 1));
                args[arg_count++] = arg;
                fprintf(stderr, "DEBUG: Added argument to function call\n");
            } else {
                fprintf(stderr, "DEBUG: Failed to parse function call argument\n");
            }
        } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
    }
    if (ll1_parser_match(parser, RPAREN)) {
        ll1_parser_advance(parser);
        fprintf(stderr, "DEBUG: Found closing parenthesis for function call\n");
    } else {
        fprintf(stderr, "DEBUG: No closing parenthesis found for function call\n");
    }
    
    fprintf(stderr, "DEBUG: Creating function call expression with %zu arguments\n", arg_count);
    // Usar strdup para duplicar el nombre de la función
    char* func_name = func_token->lexeme ? strdup(func_token->lexeme) : NULL;
    return create_call_expr(func_name, args, arg_count);
}

// Parsing de expresiones primarias
Expr* ll1_parse_primary(LL1Parser* parser) {
    Token* token = ll1_parser_peek(parser);
    if (!token) return NULL;
    
    fprintf(stderr, "DEBUG: Parsing primary expression, token type: %d (%s)\n", 
            token->type, get_token_name(token->type));
    
    switch (token->type) {
        case NUMBER:
            ll1_parser_advance(parser);
            fprintf(stderr, "DEBUG: Parsed number: %s\n", token->lexeme);
            return create_number_expr(atof(token->lexeme));
            
        case STRING:
            ll1_parser_advance(parser);
            fprintf(stderr, "DEBUG: Parsed string: %s\n", token->lexeme);
            return create_string_expr(token->lexeme);
            
        case TRUE:
        case FALSE:
            ll1_parser_advance(parser);
            fprintf(stderr, "DEBUG: Parsed boolean: %s\n", token->lexeme);
            return create_bool_expr(token->type == TRUE);
            
        case IDENT:
            ll1_parser_advance(parser);
            fprintf(stderr, "DEBUG: Parsed identifier: %s\n", token->lexeme);
            // Check if this is a function call
            if (ll1_parser_match(parser, LPAREN)) {
                fprintf(stderr, "DEBUG: Found function call in primary expression\n");
                // Go back to the identifier and parse as function call
                parser->current_token--;
                return ll1_parse_function_call(parser);
            }
            return create_variable_expr(token->lexeme);
            
        case LPAREN:
            ll1_parser_advance(parser);
            fprintf(stderr, "DEBUG: Parsing parenthesized expression\n");
            Expr* expr = ll1_parse_expression(parser);
            if (ll1_parser_match(parser, RPAREN)) {
                ll1_parser_advance(parser);
                return expr;
            }
            fprintf(stderr, "DEBUG: Missing closing parenthesis\n");
            return NULL;
            
        case LBRACKET:
            return ll1_parse_block_expr(parser);
            
        case NEW:
            return ll1_parse_new(parser);
            
        default:
            fprintf(stderr, "DEBUG: Unknown primary expression token: %d (%s)\n", 
                    token->type, get_token_name(token->type));
            return NULL;
    }
}

// Parsing de expresiones unarias
Expr* ll1_parse_unary(LL1Parser* parser) {
    Token* token = ll1_parser_peek(parser);
    if (!token) return ll1_parse_primary(parser);
    
    switch (token->type) {
        case MINUS:
        case NOT:
            ll1_parser_advance(parser);
            Expr* operand = ll1_parse_unary(parser);
            if (!operand) return NULL;
            UnaryOp op = (token->type == MINUS) ? UNARY_OP_MINUS : UNARY_OP_NOT;
            return create_unary_expr(operand, op);
            
        default:
            return ll1_parse_primary(parser);
    }
}

// Parsing de expresiones de multiplicación
Expr* ll1_parse_multiplication(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting multiplication expression parsing\n");
    Expr* left = ll1_parse_unary(parser);
    if (!left) {
        fprintf(stderr, "DEBUG: Failed to parse left operand for multiplication\n");
        return NULL;
    }
    
    while (ll1_parser_match(parser, MULT) || ll1_parser_match(parser, DIV) || ll1_parser_match(parser, MOD)) {
        Token* op = ll1_parser_advance(parser);
        fprintf(stderr, "DEBUG: Found multiplication operator: %s\n", op->lexeme);
        
        Expr* right = ll1_parse_unary(parser);
        if (!right) {
            fprintf(stderr, "DEBUG: Failed to parse right operand for multiplication\n");
            return NULL;
        }
        
        BinaryOp binary_op;
        switch (op->type) {
            case MULT: binary_op = BINARY_OP_MULT; break;
            case DIV: binary_op = BINARY_OP_DIV_OP; break;
            case MOD: binary_op = BINARY_OP_MOD; break;
            default: return NULL;
        }
        
        fprintf(stderr, "DEBUG: Creating multiplication binary expression\n");
        left = create_binary_expr(left, right, binary_op);
    }
    
    fprintf(stderr, "DEBUG: Multiplication expression parsing completed\n");
    return left;
}

// Parsing de expresiones de adición
Expr* ll1_parse_addition(LL1Parser* parser) {
    fprintf(stderr, "DEBUG: Starting addition expression parsing\n");
    Expr* left = ll1_parse_multiplication(parser);
    if (!left) {
        fprintf(stderr, "DEBUG: Failed to parse left operand for addition\n");
        return NULL;
    }
    
    while (ll1_parser_match(parser, PLUS) || ll1_parser_match(parser, MINUS)) {
        Token* op = ll1_parser_advance(parser);
        fprintf(stderr, "DEBUG: Found operator: %s\n", op->lexeme);
        
        Expr* right = ll1_parse_multiplication(parser);
        if (!right) {
            fprintf(stderr, "DEBUG: Failed to parse right operand for addition\n");
            return NULL;
        }
        
        BinaryOp binary_op = (op->type == MINUS) ? BINARY_OP_MINUS : BINARY_OP_PLUS;
        fprintf(stderr, "DEBUG: Creating binary expression with operator: %s\n", 
                (binary_op == BINARY_OP_PLUS) ? "+" : "-");
        left = create_binary_expr(left, right, binary_op);
    }
    
    fprintf(stderr, "DEBUG: Addition expression parsing completed\n");
    return left;
}

// Parsing de expresiones de comparación
Expr* ll1_parse_comparison(LL1Parser* parser) {
    Expr* left = ll1_parse_addition(parser);
    if (!left) return NULL;
    
    while (ll1_parser_match(parser, LESS_THAN) || ll1_parser_match(parser, GREATER_THAN) ||
           ll1_parser_match(parser, LE) || ll1_parser_match(parser, GE)) {
        Token* op = ll1_parser_advance(parser);
        Expr* right = ll1_parse_addition(parser);
        if (!right) return NULL;
        
        BinaryOp binary_op;
        switch (op->type) {
            case GREATER_THAN: binary_op = BINARY_OP_GREATER_THAN; break;
            case LESS_THAN: binary_op = BINARY_OP_LESS_THAN; break;
            case GE: binary_op = BINARY_OP_GE; break;
            case LE: binary_op = BINARY_OP_LE; break;
            default: return NULL;
        }
        
        left = create_binary_expr(left, right, binary_op);
    }
    
    return left;
}

// Parsing de expresiones de igualdad
Expr* ll1_parse_equality(LL1Parser* parser) {
    Expr* left = ll1_parse_comparison(parser);
    if (!left) return NULL;
    
    while (ll1_parser_match(parser, EQ) || ll1_parser_match(parser, NEQ)) {
        Token* op = ll1_parser_advance(parser);
        Expr* right = ll1_parse_comparison(parser);
        if (!right) return NULL;
        
        BinaryOp binary_op = (op->type == EQ) ? BINARY_OP_EQ : BINARY_OP_NEQ;
        left = create_binary_expr(left, right, binary_op);
    }
    
    return left;
}

// Parsing de expresiones lógicas AND
Expr* ll1_parse_logical_and(LL1Parser* parser) {
    Expr* left = ll1_parse_equality(parser);
    if (!left) return NULL;
    
    while (ll1_parser_match(parser, AND)) {
        ll1_parser_advance(parser);
        Expr* right = ll1_parse_equality(parser);
        if (!right) return NULL;
        left = create_binary_expr(left, right, BINARY_OP_AND);
    }
    
    return left;
}

// Parsing de expresiones lógicas OR
Expr* ll1_parse_logical_or(LL1Parser* parser) {
    Expr* left = ll1_parse_logical_and(parser);
    if (!left) return NULL;
    
    while (ll1_parser_match(parser, OR)) {
        ll1_parser_advance(parser);
        Expr* right = ll1_parse_logical_and(parser);
        if (!right) return NULL;
        left = create_binary_expr(left, right, BINARY_OP_OR);
    }
    
    return left;
}

// Parsing de expresiones de asignación
Expr* ll1_parse_assignment_expr(LL1Parser* parser) {
    Expr* left = ll1_parse_logical_or(parser);
    if (!left) return NULL;
    
    if (ll1_parser_match(parser, ASSIGN)) {
        ll1_parser_advance(parser);
        Expr* value = ll1_parse_assignment_expr(parser);
        if (!value) return NULL;
        
        // For now, just return the left expression
        // Assignment will be handled in statement parsing
    }
    
    return left;
}

// Parsing de expresiones
Expr* ll1_parse_expression(LL1Parser* parser) {
    return ll1_parse_assignment_expr(parser);
}

// Parsing de statements
Stmt* ll1_parse_statement(LL1Parser* parser) {
    if (!parser) return NULL;
    
    Token* token = ll1_parser_peek(parser);
    if (!token) return NULL;
    
    fprintf(stderr, "DEBUG: ll1_parse_statement called with token type: %d (%s), lexeme: '%s'\n", 
            token->type, get_token_name(token->type), token->lexeme ? token->lexeme : "NULL");
    
    switch (token->type) {
        case LBRACKET:
        case LBRACE:
            fprintf(stderr, "DEBUG: Parsing block\n");
            return ll1_parse_block(parser);
            
        case LET:
            fprintf(stderr, "DEBUG: Parsing let declaration\n");
            // Check if this is a let-in or simple let assignment
            if (ll1_parser_match_ahead(parser, IDENT)) {
                // This is a simple let assignment: let x = 42
                ll1_parser_advance(parser); // consume LET
                return ll1_parse_assignment(parser);
            } else {
                // This is a let-in expression
                return ll1_parse_let_in(parser);
            }
            
        case IF:
            fprintf(stderr, "DEBUG: Parsing if statement\n");
            return ll1_parse_if_statement(parser);
            
        case WHILE:
            fprintf(stderr, "DEBUG: Parsing while statement\n");
            return ll1_parse_while_statement(parser);
            
        case FOR:
            fprintf(stderr, "DEBUG: Parsing for statement\n");
            return ll1_parse_for_statement(parser);
            
        case RETURN:
            fprintf(stderr, "DEBUG: Parsing return statement\n");
            return ll1_parse_return_statement(parser);
            
        case FUNCTION:
            fprintf(stderr, "DEBUG: Parsing function declaration\n");
            return ll1_parse_function_decl(parser);
            
        case TYPE:
            fprintf(stderr, "DEBUG: Parsing type declaration\n");
            return ll1_parse_type_decl(parser);
            
        case IDENT:
            fprintf(stderr, "DEBUG: Parsing identifier statement\n");
            // Check for destructive assignment
            if (ll1_parser_match_ahead(parser, DEQUALS)) {
                return ll1_parse_destructive_decl(parser);
            }
            // Check for compound operators
            if (ll1_parser_match_ahead(parser, PLUSEQUAL) || ll1_parser_match_ahead(parser, MINUSEQUAL) ||
                ll1_parser_match_ahead(parser, TIMESEQUAL) || ll1_parser_match_ahead(parser, DIVEQUAL) ||
                ll1_parser_match_ahead(parser, MODEQUAL) || ll1_parser_match_ahead(parser, POWEQUAL) ||
                ll1_parser_match_ahead(parser, CONCATEQUAL) || ll1_parser_match_ahead(parser, ANDEQUAL) ||
                ll1_parser_match_ahead(parser, OREQUAL)) {
                return ll1_parse_compound_operator(parser);
            }
            // Check for assignment
            if (ll1_parser_match_ahead(parser, ASSIGN)) {
                return ll1_parse_assignment(parser);
            }
            // Check for function call
            if (ll1_parser_match_ahead(parser, LPAREN)) {
                fprintf(stderr, "DEBUG: Found function call\n");
                return create_expr_stmt(ll1_parse_function_call(parser));
            }
            // Check for method call or attribute access
            if (ll1_parser_match_ahead(parser, DOT)) {
                // This could be a setter or method call
                return create_expr_stmt(ll1_parse_get_attr(parser));
            }
            // Default to expression statement
            fprintf(stderr, "DEBUG: Parsing as expression statement\n");
            // Consumir punto y coma opcional después de expresión
            Expr* expr = ll1_parse_expression(parser);
            if (ll1_parser_match(parser, SEMICOLON)) {
                ll1_parser_advance(parser);
            }
            return create_expr_stmt(expr);
            
        case ATTRIBUTE:
            fprintf(stderr, "DEBUG: Parsing attribute declaration\n");
            return ll1_parse_attribute_decl(parser);
            
        case METHOD:
            fprintf(stderr, "DEBUG: Parsing method declaration\n");
            return ll1_parse_method_decl(parser);
            
        case NUMBER:
        case STRING:
        case TRUE:
        case FALSE:
        case LPAREN:
        case NEW:
            fprintf(stderr, "DEBUG: Parsing as expression statement\n");
            // Consumir punto y coma opcional después de expresión
            Expr* expr2 = ll1_parse_expression(parser);
            if (ll1_parser_match(parser, SEMICOLON)) {
                ll1_parser_advance(parser);
            }
            return create_expr_stmt(expr2);
            
        case RPAREN:
        case RBRACE:
            fprintf(stderr, "DEBUG: Skipping bracket/brace token\n");
            ll1_parser_advance(parser);
            return NULL;
            
        case ELSE:
            fprintf(stderr, "DEBUG: Found ELSE token, this should be handled by if statement\n");
            // ELSE should be handled by if statement parsing, not here
            return NULL;
            
        default:
            fprintf(stderr, "DEBUG: [DEFAULT] Token no reconocido en ll1_parse_statement: tipo=%d (%s), lexema='%s'\n", token->type, get_token_name(token->type), token->lexeme ? token->lexeme : "NULL");
            return NULL;
    }
}

// Parsing de anotaciones de tipo
TypeValue* ll1_parse_type_annotation(LL1Parser* parser) {
    if (!ll1_parser_match(parser, COLON)) return NULL;
    ll1_parser_advance(parser);
    
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* type_token = ll1_parser_advance(parser);
    
    // Crear un TypeValue para el tipo
    TypeValue* type_value = malloc(sizeof(TypeValue));
    type_value->name = strdup(type_token->lexeme);
    type_value->super_type = &TYPE_OBJ;
    type_value->element_type = NULL;
    type_value->argument_types = NULL;
    type_value->num_params = 0;
    type_value->def_node = NULL;
    type_value->type_parameters = NULL;
    type_value->concrete_types = NULL;
    type_value->num_concrete_types = 0;
    type_value->is_generic = false;
    type_value->is_instantiated = false;
    
    // Verificar si es un tipo genérico (con parámetros de tipo)
    if (ll1_parser_match(parser, LPAREN)) {
        ll1_parser_advance(parser);
        type_value->is_generic = true;
        
        // Parsear parámetros de tipo
        TypeParameter* params = NULL;
        int param_count = 0;
        
        if (!ll1_parser_match(parser, RPAREN)) {
            do {
                if (ll1_parser_match(parser, IDENT)) {
                    Token* param_token = ll1_parser_advance(parser);
                    TypeParameter* param = create_type_parameter(strdup(param_token->lexeme), NULL);
                    
                    if (!params) {
                        params = param;
                    } else {
                        TypeParameter* current = params;
                        while (current->next) {
                            current = current->next;
                        }
                        current->next = param;
                    }
                    param_count++;
                }
            } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
        }
        
        if (ll1_parser_match(parser, RPAREN)) {
            ll1_parser_advance(parser);
        }
        
        type_value->type_parameters = params;
    }
    
    return type_value;
}

// Parsing de tipos genéricos
TypeValue* ll1_parse_generic_type(LL1Parser* parser) {
    if (!ll1_parser_match(parser, IDENT)) return NULL;
    Token* type_token = ll1_parser_advance(parser);
    
    // Crear un TypeValue para el tipo
    TypeValue* type_value = malloc(sizeof(TypeValue));
    type_value->name = strdup(type_token->lexeme);
    type_value->super_type = &TYPE_OBJ;
    type_value->element_type = NULL;
    type_value->argument_types = NULL;
    type_value->num_params = 0;
    type_value->def_node = NULL;
    type_value->type_parameters = NULL;
    type_value->concrete_types = NULL;
    type_value->num_concrete_types = 0;
    type_value->is_generic = false;
    type_value->is_instantiated = false;
    
    // Verificar si hay parámetros de tipo genérico
    if (ll1_parser_match(parser, LPAREN)) {
        ll1_parser_advance(parser);
        type_value->is_generic = true;
        
        // Parsear parámetros de tipo
        TypeParameter* params = NULL;
        int param_count = 0;
        
        if (!ll1_parser_match(parser, RPAREN)) {
            do {
                if (ll1_parser_match(parser, IDENT)) {
                    Token* param_token = ll1_parser_advance(parser);
                    TypeParameter* param = create_type_parameter(strdup(param_token->lexeme), NULL);
                    
                    if (!params) {
                        params = param;
                    } else {
                        TypeParameter* current = params;
                        while (current->next) {
                            current = current->next;
                        }
                        current->next = param;
                    }
                    param_count++;
                }
            } while (ll1_parser_match(parser, COMMA) && ll1_parser_advance(parser));
        }
        
        if (ll1_parser_match(parser, RPAREN)) {
            ll1_parser_advance(parser);
        }
        
        type_value->type_parameters = params;
    }
    
    return type_value;
}

// Funciones para manejo de errores sintácticos
void parser_error_unexpected_token(LL1Parser* parser, TokenType expected, TokenType found) {
    if (!parser || !parser->error_context) return;
    
    Token* current = ll1_parser_peek(parser);
    if (!current) return;
    
    char message[512];
    snprintf(message, sizeof(message), 
             "Unexpected token '%s', expected '%s'", 
             get_token_name(found), get_token_name(expected));
    
    add_compiler_error_with_token(parser->error_context, ERROR_SYNTAX, message, current);
}

void parser_error_unexpected_character(LL1Parser* parser, const char* expected, const char* found) {
    if (!parser || !parser->error_context) return;
    
    Token* current = ll1_parser_peek(parser);
    if (!current) return;
    
    char message[512];
    snprintf(message, sizeof(message), 
             "Unexpected character '%s', expected '%s'", found, expected);
    
    add_compiler_error_with_token(parser->error_context, ERROR_SYNTAX, message, current);
}

void parser_error_missing_token(LL1Parser* parser, TokenType expected) {
    if (!parser || !parser->error_context) return;
    
    Token* current = ll1_parser_peek(parser);
    if (!current) return;
    
    char message[512];
    snprintf(message, sizeof(message), 
             "Missing token '%s'", get_token_name(expected));
    
    add_compiler_error_with_token(parser->error_context, ERROR_SYNTAX, message, current);
}

void parser_error_invalid_expression(LL1Parser* parser, const char* context) {
    if (!parser || !parser->error_context) return;
    
    Token* current = ll1_parser_peek(parser);
    if (!current) return;
    
    char message[512];
    snprintf(message, sizeof(message), 
             "Invalid expression in %s", context);
    
    add_compiler_error_with_token(parser->error_context, ERROR_SYNTAX, message, current);
}