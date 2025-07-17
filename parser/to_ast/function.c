

#include "../cst_to_ast.h"



ASTNode *functionDef_to_ast(CSTNode *cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);

    DEBUG(" el functionDef tiene %d hijos\n", cst_node->child_count);

    // FunctionDef → FUNCTION IDENT LPAREN ArgIdList RPAREN TypeAnnotation FunctionBody
    if(cst_node->child_count >=7)
    {
        // children[0] = FUNCTION (terminal)
        // children[1] = IDENT (function name)
        // children[2] = LPAREN (terminal)
        // children[3] = ArgIdList
        // children[4] = RPAREN (terminal)
        // children[5] = TypeAnnotation
        // children[6] = FunctionBody

        CSTNode* function_name = cst_node->children[1];
        CSTNode* argIdList = cst_node->children[3];
        CSTNode* type_annotation = cst_node->children[5];
        CSTNode* functionBody = cst_node->children[6];

        if(!function_name || !functionBody)
        {
            ERROR("El nomobre o el cuerpo de la funcion es nulo\n");
            return NULL;

        }

        char* name_function = getTokenValue(function_name);
        DEBUG("El nombre de mi funciones %s", name_function);

        // convertir argumentos
        ASTNode** args = arg_id_list(argIdList);

        //ASTNode* create_func_dec_node(char* name, ASTNode** args, int arg_count, ASTNode* body, char* ret_type);

    }

    FINISH()

}


ASTNode** arg_id_list(CSTNode* cst_node)
{

}

char* getTokenValue(CSTNode* cst_node)
{
    if(!cst_node || !is_terminal(cst_node->symbol))
    {
        return "";
    }
    return cst_node->token->value;
}