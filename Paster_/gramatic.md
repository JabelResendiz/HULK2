program         → statement_list

statement_list  → statement
                | statement_list statement

statement       → print_stmt ';'
                | if_stmt    ';'
                | expr ';'

statement_2     → print_stmt 
                | if_stmt
                | expr

print_stmt      → 'print' '(' expr ')' 

if_stmt         → 'if' '(' expr ')' statement_2 'else' statement_2

expr            → expr '+' expr
                | expr '-' expr
                | expr '*' expr
                | expr '/' expr
                | '(' expr ')'
                | NUMBER
                | letExpr

letExpr         → 'let' binding_list 'in' let_body 

binding_list    → binding
                | binding_list ',' binding 

binding         → IDENTIFIER '=' expr

let_body        → statement_2
                | '{' statement_list '}'




