

#include "cst_to_ast.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define DEBUG(...) \
    fprintf(stderr,YELLOW "DEBUG:" RESET  __VA_ARGS__ )

#define ERROR(...) \
    fprintf(stderr,RED "ERROR:" RESET  __VA_ARGS__ )

#define ACCEPT(...) \
    fprintf(stderr,GREEN "ACCEPT:" RESET  __VA_ARGS__ )

#define FINISH(...) \
    fprintf(stderr,BLUE "FINISH:" RESET  __VA_ARGS__ )


ASTNode* cst_to_ast(CSTNode* cst_node)
{

    //fprintf(stderr,"El nombre del tipo es %s\n", cst_node->symbol);
    
    ACCEPT("el nombre del tipo es %s\n", cst_node->symbol);

    if(!cst_node || strcmp(cst_node->symbol,"Program"))
    {
        fprintf(stderr,"CST root must be a program node");
        return NULL;
    }

    ASTNode* root = NULL;
    DEBUG("El programa tiene %d \n", cst_node->child_count);

    if(cst_node->child_count >0)
    {
        DEBUG("Primer hijo es %s\n", cst_node->children[0]->symbol);
        
        root = stmtList_to_ast(cst_node->children[0]);

    }

    //fprintf(stderr,"DEBUG: El Programa tiene %d statements\n", )
    
    FINISH("EL cst_to_ast termina existosamente\n");

    return root;

}

ASTNode*  stmtList_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);

    DEBUG(" el StmtList tiene %d hijos\n",cst_node->child_count);

    for(int i=0;i<cst_node->child_count;i++)
    {

        DEBUG("el nombre de mi hijo %d es %s\n",i, cst_node->children[i]->symbol);

        CSTNode* child = cst_node->children[i];

        if(child && !strcmp(child->symbol,"TerminatedStmt"))
        {

            ASTNode* terminatedStmt = terminated_stmt_to_ast(child);

           

            // fprintf(stderr,"DEBUG: Procesando TerminatedStmt\n");

            // if(!child->child_count==0)
            // {
            //     CSTNode* stmt = child->children[0];

            //     if(stmt)
            //     {
            //         fprintf(stderr,"DEBUG: Convirtiendo Stmt de tipo %s\n",stmt->symbol);

            //         ASTNode* astStmt = convertStmt(stmt);

            //         if(astStmt)
            //         {
            //             fprintf(stderr,"DEBUG: Statement convertido existosamente\n");
            //             stmt
            //         }
            //     }
            // }
        }
    }

    FINISH("EL stmtlist_to_ast termina exitosamente\n");

}



ASTNode* terminated_stmt_to_ast(CSTNode* cst_node)
{
    ACCEPT("el nombre del cst_node es %s\n", cst_node->symbol);

    DEBUG("el TerminatedStmt tiene %d hijos\n",cst_node->child_count);

    for(int i=0;i<cst_node->child_count;i++)
    {

        DEBUG("el nombre de mi hijo %d es %s\n",i, cst_node->children[i]->symbol);

        CSTNode* child = cst_node->children[i];

        stmt_to_ast(child);
    }

    FINISH("EL terminated_to_stmt to ast exito\n");
    
}


ASTNode* stmt_tp_ast(CSTNode* cst_node)
{
    ACCEPT("El nombre del cst_node es %s\n", cst_node->symbol);

    DEBUG("el stmt tiene %d hijos \n", cst_node->child_count);

    if(cst_node->child_count > 0)
    {

    }

    FINISH("El stmt to ast ha sido exito\n");
}


ASTNode* expr_to_ast(CSTNode* cst_node)
{

}


ASTNode* functionDef_to_ast(CSTNode* cst_node)
{

}

ASTNode* typeDef_to_ast(CSTNode* cst_node)
{

}

ASTNode* while_to_ast(CSTNode* cst_node)
{

}

ASTNode* block_to_ast(CSTNode* cst_node)
{
    
}