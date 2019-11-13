#include "ciLisp.h"
#include <math.h>

void yyerror(char *s) {
    fprintf(stderr, "\nERROR: %s\n", s);
    // note stderr that normally defaults to stdout, but can be redirected: ./src 2> src.log
    // CLion will display stderr in a different color from stdin and stdout
}

// Array of string values for operations.
// Must be in sync with funcs in the OPER_TYPE enum in order for resolveFunc to work.
char *funcNames[] = {
        "neg",
        "abs",
        "exp",
        "sqrt",
        "add",
        "sub",
        "mult",
        "div",
        "remainder",
        "log",
        "pow",
        "max",
        "min",
        "exp2",
        "cbrt",
        "hypot",
        "read",
        "rand",
        "print",
        "equal",
        "less",
        "greater",
        ""
};

OPER_TYPE resolveFunc(char *funcName)
{
    int i = 0;
    while (funcNames[i][0] != '\0')
    {
        if (strcmp(funcNames[i], funcName) == 0)
            return i;
        i++;
    }
    return CUSTOM_OPER;
}

// Called when an INT or DOUBLE token is encountered (see ciLisp.l and ciLisp.y).
// Creates an AST_NODE for the number.
// Sets the AST_NODE's type to number.
// Populates the value of the contained NUMBER_AST_NODE with the argument value.
// SEE: AST_NODE, NUM_AST_NODE, AST_NODE_TYPE.
AST_NODE *createNumberNode(double value, NUM_TYPE type)
{
    AST_NODE *node;
    size_t nodeSize;

    // allocate space for the fixed sie and the variable part (union)
    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    // TODO set the AST_NODE's type, assign values to contained NUM_AST_NODE done
    node->type = NUM_NODE_TYPE;
    node->data.number.type = type;
    node->data.number.value.dval = value;

    return node;
}

// Called when an f_expr is created (see ciLisp.y).
// Creates an AST_NODE for a function call.
// Sets the created AST_NODE's type to function.
// Populates the contained FUNC_AST_NODE with:
//      - An OPER_TYPE (the enum identifying the specific function being called)
//      - 2 AST_NODEs, the operands
// SEE: AST_NODE, FUNC_AST_NODE, AST_NODE_TYPE.
AST_NODE *createFunctionNode(char *funcName, AST_NODE *op1, AST_NODE *op2)
{
    AST_NODE *node;
    size_t nodeSize;

    // allocate space (or error)
    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    // TODO set the AST_NODE's type, populate contained FUNC_AST_NODE done
    // NOTE: you do not need to populate the "ident" field unless the function is type CUSTOM_OPER.
    // When you do have a CUSTOM_OPER, you do NOT need to allocate and strcpy here.
    // The funcName will be a string identifier for which space should be allocated in the tokenizer.
    // For CUSTOM_OPER functions, you should simply assign the "ident" pointer to the passed in funcName.
    // For functions other than CUSTOM_OPER, you should free the funcName after you're assigned the OPER_TYPE.
    node->type = FUNC_NODE_TYPE;
    node->data.function.oper = resolveFunc(funcName);
    node->data.function.op1 = op1;
    op1->parent = node;
    node->data.function.op2 = op2;
    if (op2 != NULL) op2->parent = node;

    return node;
}

AST_NODE *createSymbolNode(char* symbol){
    AST_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = SYM_NODE_TYPE;
    node->data.symbol.identifier = symbol;

    return node;
}

SYM_TABLE_NODE *createSymbolTableNode(AST_NODE *value, char *identifier, char *type){
    SYM_TABLE_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(SYM_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->id = identifier;
    node->value = value;
    node->next = NULL;
    if (type == NULL){
        node->val_type = NO_TYPE;
    } else if(strcmp("double", type) == 0) node->val_type = DOUBLE_TYPE;
    else node->val_type = INT_TYPE;

    return node;

}

SYM_TABLE_NODE *addToSymbolTable(SYM_TABLE_NODE *root, SYM_TABLE_NODE *new){
    new->next = root;
    return new;
}

AST_NODE *linkSymbolTable(SYM_TABLE_NODE *table, AST_NODE *node){
    node->table = table;
    return node;
}

// Called after execution is done on the base of the tree.
// (see the program production in ciLisp.y)
// Recursively frees the whole abstract syntax tree.
// You'll need to update and expand freeNode as the project develops.
void freeNode(AST_NODE *node)
{
    if (!node)
        return;

    if (node->type == FUNC_NODE_TYPE)
    {
        // Recursive calls to free child nodes
        freeNode(node->data.function.op1);
        freeNode(node->data.function.op2);

        // Free up identifier string if necessary
        if (node->data.function.oper == CUSTOM_OPER)
        {
            free(node->data.function.ident);
        }
    }

    free(node);
}

// Evaluates an AST_NODE.
// returns a RET_VAL storing the the resulting value and type.
// You'll need to update and expand eval (and the more specific eval functions below)
// as the project develops.
RET_VAL eval(AST_NODE *node)
{
    if (!node)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN}; // see NUM_AST_NODE, because RET_VAL is just an alternative name for it.

    // TODO complete the switch. done
    // Make calls to other eval functions based on node type.
    // Use the results of those calls to populate result.
    switch (node->type)
    {
        case NUM_NODE_TYPE:
            result = evalNumNode(&node->data.number);
            break;
        case FUNC_NODE_TYPE:
            result = evalFuncNode(&node->data.function);
            break;
        case SYM_NODE_TYPE:
            result = evalSymNode(&node->data.symbol, node);
        default:
            yyerror("Invalid AST_NODE_TYPE, probably invalid writes somewhere!");
    }

    return result;
}  

// returns a pointer to the NUM_AST_NODE (aka RET_VAL) referenced by node.
// DOES NOT allocate space for a new RET_VAL.
RET_VAL evalNumNode(NUM_AST_NODE *numNode)
{
    if (!numNode)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    // TODO populate result with the values stored in the node. done
    // SEE: AST_NODE, AST_NODE_TYPE, NUM_AST_NODE
    result.type = numNode->type;
    result.value.dval = numNode->value.dval;

    return result;
}

RET_VAL evalSymNode(SYM_AST_NODE *symNode, AST_NODE *node){
    AST_NODE *done = lookup(symNode, node);
    return eval(done);
}

NUM_TYPE resultTypeSetter(RET_VAL o1, RET_VAL o2, FUNC_AST_NODE func){
    switch (func.oper){
        case NEG_OPER:
        case ABS_OPER:
        case EXP_OPER:
        case PRINT_OPER:
            return o1.type;
        case ADD_OPER:
        case SUB_OPER:
        case MULT_OPER:
        case REMAINDER_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE) return INT_TYPE;
            else return DOUBLE_TYPE;
        case LOG_OPER:
        case CBRT_OPER:
        case HYPOT_OPER:
        case DIV_OPER:
        case MAX_OPER:
        case MIN_OPER:
        case POW_OPER:
        case EXP2_OPER:
        case SQRT_OPER:
            return DOUBLE_TYPE;
    }

}

RET_VAL evalFuncNode(FUNC_AST_NODE *funcNode)
{
    if (!funcNode)
        return (RET_VAL){INT_TYPE, NAN};

    RET_VAL result = {INT_TYPE, NAN};

    // TODO populate result with the result of running the function on its operands.
    // SEE: AST_NODE, AST_NODE_TYPE, FUNC_AST_NODE
    RET_VAL o1 = eval(funcNode->op1);
    RET_VAL o2 = eval(funcNode->op2);
    result.type = resultTypeSetter(o1, o2, *funcNode);
    switch (funcNode->oper){
        case NEG_OPER:
            result = o1;
            result.value.dval *= -1;
            break;

        case ABS_OPER:
            result = o1;
            result.value.dval = fabs(result.value.dval);
            break;

        case EXP_OPER:
            result = o1;
            result.value.dval = exp(result.value.dval);
            break;

        case SQRT_OPER:
            result = o1;
            result.value.dval = sqrt(result.value.dval);
            break;

        case ADD_OPER:
            result.value.dval = o1.value.dval + o2.value.dval;
            break;

        case SUB_OPER:
            result.value.dval = o1.value.dval - o2.value.dval;
            break;

        case MULT_OPER:
            result.value.dval = o1.value.dval * o2.value.dval;
            break;

        case DIV_OPER:
            result.value.dval = o1.value.dval / o2.value.dval;
            break;

        case REMAINDER_OPER:
            result.value.dval = remainder(o1.value.dval, o2.value.dval);
            break;

        case LOG_OPER:
            result.value.dval = log(o1.value.dval);
            break;

        case POW_OPER:
            result.value.dval = pow(o1.value.dval, o2.value.dval);
            break;

        case MAX_OPER:
            result.value.dval = fmax(o1.value.dval, o2.value.dval);
            break;

        case MIN_OPER:
            result.value.dval = fmin(o1.value.dval, o2.value.dval);
            break;

        case EXP2_OPER:
            result.value.dval = exp2(o1.value.dval);
            break;

        case CBRT_OPER:
            result.value.dval = cbrt(o1.value.dval);
            break;

        case HYPOT_OPER:
            result.value.dval = hypot(o1.value.dval, o2.value.dval);
            break;

        case PRINT_OPER:
            printFunc(funcNode->op1);
            printf("\n");
            result.value.dval = (eval(funcNode->op1)).value.dval;
            break;
    }
    return result;
}

char *operNames[] = {"negate", "absolute value of", "base e exponent of",
                  "square root of", "add", "subtract", "multiply", "divide", "remainder of", "logarithm of",
                  "power of", "maximum of", "minimum of", "base 2 exponent of",
                  "cube root of", "hypotenuse of", "reading", "randing", "printing"};

void printFunc(AST_NODE *node){
    double num;
    switch (node->type){
        case NUM_NODE_TYPE:
            num = node->data.number.value.dval;
            switch (node->data.number.type){
                case INT_TYPE:
                    printf("%.0lf ", num);
                    break;
                case DOUBLE_TYPE:
                    printf("%.2lf ", num);
                    break;
            }
            break;
        case FUNC_NODE_TYPE:
            printf("PRINT: ( %s ", operNames[node->data.function.oper]);
            printFunc(node->data.function.op1);

            if (node->data.function.op2 != NULL) {
                printf("with ");
                printFunc(node->data.function.op2);
            }

            printf(")");
            break;
        case SYM_NODE_TYPE:
            num = eval(node).value.dval;
            printf("%.2lf ", num);
            break;
    }
}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val)
{
    // TODO print the type and value of the value passed in. done
    printf("Type: ");
    switch (val.type){
        case INT_TYPE:
            printf("Integer, Value %.0lf\n", val.value.dval);
            break;
        case DOUBLE_TYPE:
            printf("Double, Value %.2lf\n", val.value.dval);
            break;
    }
}

AST_NODE *lookup(SYM_AST_NODE *symbol, AST_NODE *origin){
    char *search = symbol->identifier;
    while (origin != NULL) {
        SYM_TABLE_NODE *currentTable = origin->table;
        while (currentTable != NULL) {
            if (strcmp(currentTable->id, search) == 0) {
                if (currentTable->val_type != NO_TYPE){
                    if (currentTable->val_type == INT_TYPE && currentTable->value->data.number.type == DOUBLE_TYPE){
                        printf("WARNING: Precision loss of %.5lf in variable %s\n", currentTable->value->data.number.value.dval, search);
                    }
                    currentTable->value->data.number.type = currentTable->val_type;
                }
                return currentTable->value;
            }
            currentTable = currentTable->next;

        }
        origin = origin->parent;
    }

    yyerror("Invalid symbol given!");
    exit(1);
}


