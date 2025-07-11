#include <stdio.h>
#include "./ast/ast.h"
#include "./codegen/codegen.h"
#include "./semantic_check/semantic.h"

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
    yyin = fopen("script.hulk", "r");
    if (!yyin)
    {
        perror("Error opening script.hulk");
        return 1;
    }

    if (!yyparse() && !analyze_semantics(root))
    {
        fclose(yyin);

        printf(BLUE "\n🌳 Abstract Syntax Tree:\n" RESET);
        print_ast(root, 0);

        printf(CYAN "\nGenerating LLVM code...\n" RESET);
        compile_to_llvm(root, "./build/output.ll");
        printf(GREEN "✅ LLVM code generated succesfully in output.ll\n" RESET);

        free_ast(root);
        root = NULL;
    }

    return 0;
}