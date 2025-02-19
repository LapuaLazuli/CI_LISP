%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct sym_table_node *symTbNode;
    struct arg_table_node *argTbNode;
};

%token <sval> FUNC SYMBOL TYPE
%token <dval> INT DOUBLE
%token LPAREN RPAREN EOL QUIT LET COND LAMBDA

%type <astNode> s_expr f_expr number s_expr_list
%type <symTbNode> let_elem let_section let_list
%type <argTbNode> arg_list

%%

program:
    s_expr EOL {
        fprintf(stderr, "yacc: program ::= s_expr EOL\n");
        if ($1) {
            printRetVal(eval($1));
            freeNode($1);
        }
    };

s_expr:
    number {
        fprintf(stderr, "yacc: s_expr ::= number\n");
        $$ = $1;
    }
    | SYMBOL {
        fprintf(stderr, "yacc: s_expr ::= symbol\n");
        $$ = createSymbolNode($1);
    }
    | f_expr {
        $$ = $1;
    }
    | QUIT {
        fprintf(stderr, "yacc: s_expr ::= QUIT\n");
        exit(EXIT_SUCCESS);
    }
    | LPAREN let_section s_expr RPAREN {
        fprintf(stderr, "yacc: s_expr ::= let\n");
        $$ = linkSymbolTable($2, $3);
    }
    | LPAREN COND s_expr s_expr s_expr RPAREN{
        fprintf(stderr, "yacc: s_expr ::= LPAREN COND s_expr s_expr s_expr RPAREN\n");
        $$ = createCondNode($3, $4, $5);
    }
    | error {
        fprintf(stderr, "yacc: s_expr ::= error\n");
        yyerror("unexpected token");
        $$ = NULL;
    };

s_expr_list:
    s_expr s_expr_list{
        $$ = addToS_exprList($1, $2);
    }
    | s_expr{
        $$ = $1;
    };

let_elem:
    LPAREN TYPE SYMBOL s_expr RPAREN {
        $$ = createSymbolTableNode($4, $3, $2);
    }
    | LPAREN SYMBOL s_expr RPAREN {
        $$ = createSymbolTableNode($3, $2, NULL);
    }
    | LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN{
        fprintf(stderr, "yacc: let_elem ::= LPAREN TYPE SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
        $$ = createLambdaSymbolTableNode($8, $3, $2, $6);
    }
    | LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN{
        fprintf(stderr, "yacc: let_elem ::= LPAREN SYMBOL LAMBDA LPAREN arg_list RPAREN s_expr RPAREN\n");
        $$ = createLambdaSymbolTableNode($7, $2, NULL, $5);
    };

arg_list:
    SYMBOL arg_list{
        $$ = addToArgTable($2, $1);
    }
    | SYMBOL{
        $$ = createArgTableNode($1);
    };

let_list:
    LET let_elem {
        $$ = $2;
    }
    | let_list let_elem {
        $$ = addToSymbolTable($1, $2);
    };

let_section:
    LPAREN let_list RPAREN{
        $$ = $2;
    };


number:
    INT {
        fprintf(stderr, "yacc: number ::= INT\n");
        $$ = createNumberNode($1, INT_TYPE);
    }
    | DOUBLE {
        fprintf(stderr, "yacc: number ::= DOUBLE\n");
        $$ = createNumberNode($1, DOUBLE_TYPE);
    };

f_expr:
    LPAREN FUNC RPAREN{
        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC RPAREN\n");
        $$ = createFunctionNode($2, NULL);
    }
    | LPAREN FUNC s_expr_list RPAREN {
        fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC s_expr_list RPAREN\n");
        $$ = createFunctionNode($2, $3);
    }
    | LPAREN SYMBOL s_expr_list RPAREN{
        fprintf(stderr, "yacc: s_expr ::= LPAREN SYMBOL s_expr_list RPAREN\n");
        $$ = createFunctionNode($2, $3);
    };

%%

