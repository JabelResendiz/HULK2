#include <stdio.h>
#include <stdlib.h>
#include "./ast/ast.h"
#include "./parser/cst_to_ast.h"

int main() {
    printf("Testing AST creation...\n");
    
    // Test creating a simple number node
    ASTNode* num_node = create_number_node(42.0);
    printf("Number node created successfully\n");
    
    // Test creating a variable node
    ASTNode* var_node = create_variable_node("x", NULL, 0);
    printf("Variable node created successfully\n");
    
    // Test creating a function call node
    ASTNode** args = malloc(sizeof(ASTNode*) * 1);
    args[0] = var_node;
    ASTNode* func_call = create_func_call_node("print", args, 1);
    printf("Function call node created successfully\n");
    
    // Test creating a let-in node
    ASTNode** declarations = malloc(sizeof(ASTNode*) * 1);
    declarations[0] = var_node;
    ASTNode* let_in = create_let_in_node(declarations, 1, func_call);
    printf("Let-in node created successfully\n");
    
    printf("All AST nodes created successfully!\n");
    
    // Clean up
    free(args);
    free(declarations);
    
    return 0;
} 