#include <stdio.h>
#include "./ast/ast.h"
#include "./codegen/codegen.h"
#include "./semantic_check/semantic.h"
#include "./lexer/lexer_generator.h"
// Parser LL(1) includes
#include "./parser/ll1_parser.h"
#include "./parser/grammar_parser.h"
#include "./parser/first_calculator.h"
#include "./parser/follow_calculator.h"
#include "./parser/ll1_table.h"

// Redefinir colores para evitar conflictos
#undef GREEN
#undef BLUE
#undef RESET
#undef RED
#undef CYAN
#undef YELLOW

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
#define RED "\033[31m"
#define CYAN "\033[36m"
#define YELLOW "\033[33m"

extern int yyparse(void);
extern FILE *yyin;
extern ASTNode *root;

// Función para convertir tokens del lexer a tokens del parser LL(1)
ParserToken *convert_lexer_token_to_parser_token(Token *lexer_token)
{
    ParserToken *parser_token = malloc(sizeof(ParserToken));
    parser_token->type = strdup(get_token_name(lexer_token->type));
    parser_token->value = strdup(lexer_token->lexeme);
    parser_token->line = lexer_token->line;
    parser_token->column = lexer_token->column;
    return parser_token;
}

int main()
{
    // --- Análisis léxico ---
    FILE *lex_file = fopen("script.hulk", "r");
    if (!lex_file)
    {
        perror("Error opening script.hulk");
        return 1;
    }
    fseek(lex_file, 0, SEEK_END);
    long fsize = ftell(lex_file);
    fseek(lex_file, 0, SEEK_SET);
    char *source = malloc(fsize + 1);
    fread(source, 1, fsize, lex_file);
    source[fsize] = '\0';
    fclose(lex_file);

    Lexer *lexer = lexer_create();
    lexer_set_input(lexer, source);
    printf(CYAN "\n[LEXER] Tokens reconocidos en script.hulk:\n" RESET);

    // Recolectar todos los tokens para el parser LL(1)
    ParserToken **parser_tokens = malloc(1000 * sizeof(ParserToken *)); // Máximo 1000 tokens
    int token_count = 0;

    Token *tok;
    while ((tok = lexer_next_token(lexer)) && tok->type != TOKEN_EOF)
    {
        printf("%s: '%s' (línea %d, columna %d)\n", get_token_name(tok->type), tok->lexeme, tok->line, tok->column);

        // Convertir y almacenar para el parser LL(1)
        parser_tokens[token_count] = convert_lexer_token_to_parser_token(tok);
        token_count++;

        destroy_token(tok);
    }

    // Agregar token EOF al final
    parser_tokens[token_count] = malloc(sizeof(ParserToken));
    parser_tokens[token_count]->type = strdup("$");
    parser_tokens[token_count]->value = strdup("$");
    parser_tokens[token_count]->line = 0;
    parser_tokens[token_count]->column = 0;
    token_count++;

    lexer_destroy(lexer);
    free(source);

    // --- Análisis sintáctico con Parser LL(1) ---
    printf(YELLOW "\n[PARSER LL(1)] Iniciando análisis sintáctico...\n" RESET);

    // Construir la gramática
    Grammar *grammar = parse_grammar_file("parser/grammar_hulk.ll1");
    if (!grammar)
    {
        printf(RED "❌ Error: No se pudo cargar la gramática\n" RESET);
        return 1;
    }

    // Calcular conjuntos FIRST y FOLLOW
    FirstResult first_result = compute_first_sets(grammar);
    FollowResult *follow_result = compute_follow_sets(grammar, &first_result);

    // Construir tabla LL(1)
    LL1Table *table = build_ll1_table(grammar, &first_result, follow_result);
    if (!table)
    {
        printf(RED "❌ Error: No se pudo construir la tabla LL(1)\n" RESET);
        return 1;
    }

    // Crear parser LL(1)
    LL1Parser *parser = create_ll1_parser(grammar, table);
    if (!parser)
    {
        printf(RED "❌ Error: No se pudo crear el parser LL(1)\n" RESET);
        return 1;
    }

    // Agregar tokens al parser
    for (int i = 0; i < token_count; i++)
    {
        add_token(parser, parser_tokens[i]);
    }

    // Ejecutar parsing
    CSTNode *cst_root = parse_ll1(parser);

    if (cst_root)
    {
        printf(GREEN "✅ Análisis sintáctico exitoso!\n" RESET);
        printf(BLUE "\n🌳 Concrete Syntax Tree (CST):\n" RESET);
        print_cst_tree(cst_root, 0);

        // NO liberar el CST aquí - free_ll1_parser se encarga de liberarlo
    }
    else
    {
        printf(RED "❌ Error en el análisis sintáctico\n" RESET);
    }

    // Liberar memoria
    free(parser_tokens);
    free_ll1_parser(parser);
    free_ll1_table(table);
    free_follow_result(follow_result);
    free_first_result(first_result);
    free_grammar(grammar);

    // --- Análisis sintáctico original (comentado) ---
    // yyin = fopen("script.hulk", "r");
    // if (!yyin)
    // {
    //     perror("Error opening script.hulk");
    //     return 1;
    // }

    // if (!yyparse() && !analyze_semantics(root))
    // {
    //     fclose(yyin);

    //     printf(BLUE "\n🌳 Abstract Syntax Tree:\n" RESET);
    //     print_ast(root, 0);

    //     printf(CYAN "\nGenerating LLVM code...\n" RESET);
    //     compile_to_llvm(root, "./build/output.ll");
    //     printf(GREEN "✅ LLVM code generated succesfully in output.ll\n" RESET);

    //     free_ast(root);
    //     root = NULL;
    // }

    return 0;
}