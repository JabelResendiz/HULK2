#include <stdio.h>
#include "./ast/ast.h"
#include "./codegen/codegen.h"
#include "./semantic_check/semantic.h"
#include "./lexer/lexer_generator.h"
#include "./parser/grammar.h"

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define RESET "\033[0m"
#define RED "\033[31m"
#define CYAN "\033[36m"

extern int yyparse(void);
extern FILE *yyin;
extern ASTNode *root;

int main()
{
    // --- Análisis léxico ---
    // FILE *lex_file = fopen("script.hulk", "r");
    // if (!lex_file)
    // {
    //     perror("Error opening script.hulk");
    //     return 1;
    // }
    // fseek(lex_file, 0, SEEK_END);
    // long fsize = ftell(lex_file);
    // fseek(lex_file, 0, SEEK_SET);
    // char *source = malloc(fsize + 1);
    // fread(source, 1, fsize, lex_file);
    // source[fsize] = '\0';
    // fclose(lex_file);

    // Lexer *lexer = lexer_create();
    // lexer_set_input(lexer, source);
    // printf(CYAN "\n[LEXER] Tokens reconocidos en script.hulk:\n" RESET);
    // Token *tok;
    // while ((tok = lexer_next_token(lexer)) && tok->type != TOKEN_EOF)
    // {
    //     printf("%s: '%s' (línea %d, columna %d)\n", get_token_name(tok->type), tok->lexeme, tok->line, tok->column);
    //     destroy_token(tok);
    // }
    // lexer_destroy(lexer);
    // free(source);

    // --- Análisis sintáctico ---
    LL1_Grammar g;
    ll1_load_grammar_from_file(&g, "parser/grammar.txt");
    ll1_print_grammar(&g);

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