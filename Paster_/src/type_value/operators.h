
#ifndef OPERATORS_H
#define OPERATORS_H

typedef struct TypeValue TypeValue;
typedef struct ASTNode ASTNode;

typedef enum
{
    OP_ADD,OP_SUB,OP_MULT,OP_DIV,OP_CONCAT,OP_DCONCAT,OP_POW,OP_MOD,
    OP_AND,OP_OR,OP_NEQ,
    OP_GT,OP_LT,OP_EQ,OP_GE,OP_LE
    
}Operator;


typedef struct 
{
    Operator op;
    TypeValue* left_type;
    TypeValue* right_type;
    TypeValue* result_type; 
}OperatorRule;

extern int rule_count;
extern OperatorRule operatorRules[];


int match_op(TypeValue* first, TypeValue* second,Operator op);

int find_rule(Operator op, OperatorRule **rule, int flag, ASTNode *node);

#endif