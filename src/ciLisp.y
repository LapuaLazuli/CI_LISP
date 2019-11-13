%{
    #include "ciLisp.h"
%}

%union {
    double dval;
    char *sval;
    struct ast_node *astNode;
    struct sym_table_node *symTbNode;
};

%token <sval> FUNC SYMBOL
%token <dval> INT DOUBLE
%token LPAREN RPAREN EOL QUIT LET

%type <astNode> s_expr f_expr number
%type <symTbNode> let_elem let_section let_list

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
        fprint(stderr, "yacc: s_expr ::= symbol\n");
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
    | error {
        fprintf(stderr, "yacc: s_expr ::= error\n");
        yyerror("unexpected token");
        $$ = NULL;
    };

let_elem:
    LPAREN SYMBOL s_expr RPAREN {
        $$ = createSymbolTableNode($2, $3);
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
    LPAREN FUNC s_expr RPAREN {
        //fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr RPAREN\n");
        $$ = createFunctionNode($2, $3, NULL);
    }
    | LPAREN FUNC s_expr s_expr RPAREN {
        //fprintf(stderr, "yacc: s_expr ::= LPAREN FUNC expr expr RPAREN\n");
        $$ = createFunctionNode($2, $3, $4);
    };
%%

