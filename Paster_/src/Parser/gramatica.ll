# Gramática LL(1) para el lenguaje Hulk
# Basada en el parser actual implementado en ll1_parser.c

# Símbolos terminales (tokens)
%token FUNCTION LET IN IF ELSE WHILE FOR RETURN TYPE
%token IDENT NUMBER STRING TRUE FALSE
%token PLUS MINUS MULT DIV MOD POW
%token LE GE EQ NEQ LESS_THAN GREATER_THAN
%token OR AND NOT
%token ASSIGN DEQUALS
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET
%token COMMA SEMICOLON DOT COLON
%token EOF

# Símbolos no terminales
%start program

# Reglas gramaticales

# Programa principal
program ::= statement_list
statement_list ::= statement statement_list
                | ε

# Statements
statement ::= function_decl
            | let_decl
            | if_statement
            | while_statement
            | for_statement
            | return_statement
            | assignment_statement
            | expression_statement
            | block_statement

# Declaración de funciones
function_decl ::= FUNCTION IDENT LPAREN param_list RPAREN function_body
param_list ::= IDENT param_list_tail
             | ε
param_list_tail ::= COMMA IDENT param_list_tail
                  | ε
function_body ::= LBRACE statement_list RBRACE
                | expression_statement

# Declaraciones let
let_decl ::= LET IDENT ASSIGN expression SEMICOLON
           | let_in_statement

let_in_statement ::= LET binding_list IN expression
binding_list ::= binding binding_list_tail
binding ::= IDENT ASSIGN expression
binding_list_tail ::= COMMA binding binding_list_tail
                    | ε

# Estructuras de control
if_statement ::= IF LPAREN expression RPAREN statement else_clause
else_clause ::= ELSE statement
              | ε

while_statement ::= WHILE LPAREN expression RPAREN statement

for_statement ::= FOR LPAREN IDENT IN expression RPAREN statement

# Return statement
return_statement ::= RETURN expression SEMICOLON

# Asignaciones
assignment_statement ::= IDENT ASSIGN expression SEMICOLON

# Expresiones
expression_statement ::= expression SEMICOLON

# Bloques
block_statement ::= LBRACE statement_list RBRACE
                  | LBRACKET statement_list RBRACKET

# Jerarquía de expresiones (precedencia)
expression ::= logical_or_expression

logical_or_expression ::= logical_and_expression logical_or_tail
logical_or_tail ::= OR logical_and_expression logical_or_tail
                  | ε

logical_and_expression ::= equality_expression logical_and_tail
logical_and_tail ::= AND equality_expression logical_and_tail
                   | ε

equality_expression ::= comparison_expression equality_tail
equality_tail ::= EQ comparison_expression equality_tail
                | NEQ comparison_expression equality_tail
                | ε

comparison_expression ::= addition_expression comparison_tail
comparison_tail ::= LESS_THAN addition_expression comparison_tail
                  | GREATER_THAN addition_expression comparison_tail
                  | LE addition_expression comparison_tail
                  | GE addition_expression comparison_tail
                  | ε

addition_expression ::= multiplication_expression addition_tail
addition_tail ::= PLUS multiplication_expression addition_tail
                | MINUS multiplication_expression addition_tail
                | ε

multiplication_expression ::= unary_expression multiplication_tail
multiplication_tail ::= MULT unary_expression multiplication_tail
                      | DIV unary_expression multiplication_tail
                      | MOD unary_expression multiplication_tail
                      | POW unary_expression multiplication_tail
                      | ε

unary_expression ::= MINUS unary_expression
                   | NOT unary_expression
                   | primary_expression

primary_expression ::= NUMBER
                     | STRING
                     | TRUE
                     | FALSE
                     | IDENT
                     | LPAREN expression RPAREN
                     | function_call
                     | method_call
                     | property_access

# Llamadas de función
function_call ::= IDENT LPAREN argument_list RPAREN
argument_list ::= expression argument_list_tail
                | ε
argument_list_tail ::= COMMA expression argument_list_tail
                     | ε

# Llamadas de método
method_call ::= IDENT DOT IDENT LPAREN argument_list RPAREN

# Acceso a propiedades
property_access ::= IDENT DOT IDENT

# Comentarios (ignorados por el lexer)
# Los comentarios se manejan en el lexer, no en la gramática

# Reglas de precedencia de operadores
%left OR
%left AND
%left EQ NEQ
%left LESS_THAN GREATER_THAN LE GE
%left PLUS MINUS
%left MULT DIV MOD
%right POW
%right UMINUS NOT 