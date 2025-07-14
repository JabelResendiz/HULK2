

#include "operators.h"
#include "../ast/ast.h"
#include <stdio.h>



OperatorRule operatorRules[] =
    {
        {OP_ADD, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},
        {OP_SUB, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},
        {OP_MULT, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},
        {OP_DIV, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},
        {OP_POW, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},
        {OP_MOD, &TYPE_NUM, &TYPE_NUM, &TYPE_NUM},

        {OP_CONCAT, &TYPE_STRING, &TYPE_STRING, &TYPE_STRING},
        {OP_CONCAT, &TYPE_NUM, &TYPE_STRING, &TYPE_STRING},
        {OP_CONCAT, &TYPE_STRING, &TYPE_NUM, &TYPE_STRING},
        {OP_DCONCAT, &TYPE_NUM, &TYPE_STRING, &TYPE_STRING},
        {OP_DCONCAT, &TYPE_STRING, &TYPE_STRING, &TYPE_STRING},
        {OP_DCONCAT, &TYPE_STRING, &TYPE_NUM, &TYPE_STRING},

        {OP_AND, &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN},
        {OP_OR, &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN},
        {OP_NEQ, &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN},
        {OP_NEQ, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_NEQ, &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN},

        {OP_GT, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_GE, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_LT, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_LE, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_EQ, &TYPE_NUM, &TYPE_NUM, &TYPE_BOOLEAN},
        {OP_EQ, &TYPE_STRING, &TYPE_STRING, &TYPE_BOOLEAN},
        {OP_LT, &TYPE_BOOLEAN, &TYPE_BOOLEAN, &TYPE_BOOLEAN},
};

int rule_count = sizeof(operatorRules) / sizeof(OperatorRule);


// determina al cantidad de reglas que matchean con el operador
int find_rule(Operator op, OperatorRule **rule, int flag, ASTNode *right)
{
    int count = 0;
    for (int i = 0; i < rule_count; i++)
    {
        if ((!flag && operatorRules[i].op == op) ||
            (flag && operatorRules[i].op == op &&
             (!right || compare_types(right->computed_type, &TYPE_ERROR) ||
              compare_types(right->computed_type, (flag == 1) ? operatorRules[i].right_type : operatorRules[i].left_type))))

        {
            count++;
            if (count > 1)
            {
                return count; // hay una ambiguedad de tipos
            }

            *rule = &operatorRules[i];
        }
    }

    return count;
}


