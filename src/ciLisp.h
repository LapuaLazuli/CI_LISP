#ifndef __cilisp_h_
#define __cilisp_h_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "ciLispParser.h"

int yyparse(void);

int yylex(void);

void yyerror(char *);

// Enum of all operators.
// must be in sync with funcs in resolveFunc()
typedef enum oper {
    NEG_OPER, // 0
    ABS_OPER,
    EXP_OPER,
    SQRT_OPER,
    ADD_OPER,
    SUB_OPER,
    MULT_OPER,
    DIV_OPER,
    REMAINDER_OPER,
    LOG_OPER,
    POW_OPER,
    MAX_OPER,
    MIN_OPER,
    EXP2_OPER,
    CBRT_OPER,
    HYPOT_OPER,
    READ_OPER,
    RAND_OPER,
    PRINT_OPER,
    EQUAL_OPER,
    LESS_OPER,
    GREATER_OPER,
    CUSTOM_OPER =255
} OPER_TYPE;

OPER_TYPE resolveFunc(char *);

// Types of Abstract Syntax Tree nodes.
// Initially, there are only numbers and functions.
// You will expand this enum as you build the project.
typedef enum {
    NUM_NODE_TYPE,
    FUNC_NODE_TYPE,
    SYM_NODE_TYPE,
    COND_NODE_TYPE
} AST_NODE_TYPE;

// Types of numeric values
typedef enum {
    INT_TYPE,
    DOUBLE_TYPE,
    NO_TYPE
} NUM_TYPE;

typedef enum {
    VARIABLE_TYPE,
    LAMBDA_TYPE
} SYMBOL_TYPE;

//Node to store a condition
typedef struct{
    struct ast_node *cond;
    struct ast_node *nodeTrue;
    struct ast_node *nodeFalse;
} COND_AST_NODE;

//Node to store a symbol
typedef struct{
    char *identifier;
} SYM_AST_NODE;

// Node to store a number.
typedef struct {
    NUM_TYPE type;
    union{
        double dval;
    } value;
} NUM_AST_NODE;

typedef struct arg_table_node {
    char *ident;
    NUM_AST_NODE *val;
    struct arg_table_node *next;
} ARG_TABLE_NODE;


// Values returned by eval function will be numbers with a type.
// They have the same structure as a NUM_AST_NODE.
// The line below allows us to give this struct another name for readability.
typedef NUM_AST_NODE RET_VAL;

// Node to store a function call with its inputs
typedef struct {
    OPER_TYPE oper;
    char* ident; // only needed for custom functions
    struct ast_node *opList;
} FUNC_AST_NODE;

// Generic Abstract Syntax Tree node. Stores the type of node,
// and reference to the corresponding specific node (initially a number or function call).
typedef struct ast_node {
    AST_NODE_TYPE type;
    struct ast_node *parent;
    struct sym_table_node *table;
    struct arg_table_node *argTable;
    union {
        NUM_AST_NODE number;
        FUNC_AST_NODE function;
        SYM_AST_NODE symbol;
        COND_AST_NODE condition;
    } data;
    struct ast_node *next;
} AST_NODE;

//Stores a symbol table
typedef struct sym_table_node{
    SYMBOL_TYPE type;
    NUM_TYPE val_type;
    char *id;
    AST_NODE *value;
    struct sym_table_node *next;
} SYM_TABLE_NODE;

typedef struct ret_val_list{
    RET_VAL *val;
    struct ret_val_list *next;
} RET_VAL_LIST;

AST_NODE *createNumberNode(double value, NUM_TYPE type);

AST_NODE *createFunctionNode(char *funcName, AST_NODE *opList);

void freeNode(AST_NODE *node);

RET_VAL eval(AST_NODE *node);
RET_VAL evalNumNode(NUM_AST_NODE *numNode);
RET_VAL evalFuncNode(AST_NODE *node);
RET_VAL evalSymNode(SYM_AST_NODE *symNode, AST_NODE *node);
RET_VAL evalCondNode(COND_AST_NODE *condNode);

AST_NODE *lookup(char *search, AST_NODE *origin);
AST_NODE *createSymbolNode(char *symbol);
SYM_TABLE_NODE *createSymbolTableNode(AST_NODE *value, char *identifier, char *type);
SYM_TABLE_NODE *addToSymbolTable(SYM_TABLE_NODE *root, SYM_TABLE_NODE *new);
AST_NODE *linkSymbolTable(SYM_TABLE_NODE *table, AST_NODE *node);
AST_NODE *addToS_exprList(AST_NODE *new, AST_NODE *base);
AST_NODE *createCondNode(AST_NODE *cond, AST_NODE *trueSec, AST_NODE *falseSec);
ARG_TABLE_NODE *createArgTableNode(char *id);
ARG_TABLE_NODE *addToArgTable(ARG_TABLE_NODE *root, char *new);
SYM_TABLE_NODE *createLambdaSymbolTableNode(AST_NODE *value, char *id, char *type, ARG_TABLE_NODE *arg);
RET_VAL_LIST *evalForArg(AST_NODE *current);


void printFunc(AST_NODE *node);
void printRetVal(RET_VAL val);
void freeRetValList(RET_VAL_LIST *root);


#endif
