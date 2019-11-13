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
    switch (type){
        case INT_TYPE:
            node->data.number.value.ival = (long) value;
            break;
        case DOUBLE_TYPE:
            node->data.number.value.dval = value;
            break;
    }

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
    op2->parent = node;

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

SYM_TABLE_NODE *createSymbolTableNode(AST_NODE *value, char *identifier){
    SYM_TABLE_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(SYM_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->id = identifier;
    node->value = value;
    node->next = NULL;

    return node;

}

SYM_TABLE_NODE *addToSymbolTable(SYM_TABLE_NODE *root, SYM_TABLE_NODE *new){
    SYM_TABLE_NODE *current = root->next;
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
            result = evalSymNode(&node->data.symbol);
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
    switch (numNode->type){
        case INT_TYPE:
            result.value.ival = numNode->value.ival;
            break;
        case DOUBLE_TYPE:
            result.value.dval = numNode->value.dval;
            break;
    }


    return result;
}

RET_VAL evalSymNode(SYM_AST_NODE *symNode){
    AST_NODE *done = lookup(symNode);
    return eval(done);
}

NUM_TYPE resultTypeSetter(RET_VAL o1, RET_VAL o2, FUNC_AST_NODE func){
    switch (func.oper){
        case NEG_OPER:
        case ABS_OPER:
        case EXP_OPER:
        case SQRT_OPER:
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
            if (result.type == INT_TYPE){
                result.value.ival *= -1;
            } else result.value.dval *= -1;
            break;

        case ABS_OPER:
            result = o1;
            switch (result.type){
                case INT_TYPE:
                    result.value.ival = (long) fabs((double) result.value.ival);
                    break;
                case DOUBLE_TYPE:
                    result.value.dval = fabs(result.value.dval);
                    break;
            }
            break;

        case EXP_OPER:
            result = o1;
            switch (result.type){
                case INT_TYPE:
                    result.type = DOUBLE_TYPE;
                    result.value.dval = exp((double) result.value.ival);
                    break;
                case DOUBLE_TYPE:
                    result.value.dval = exp(result.value.dval);
                    break;
            }
            break;

        case SQRT_OPER:
            result = o1;
            switch (result.type){
                case INT_TYPE:
                    result.type = DOUBLE_TYPE;
                    result.value.dval = sqrt((double) result.value.ival);
                    break;
                case DOUBLE_TYPE:
                    result.value.dval = sqrt(result.value.dval);
                    break;
            }
            break;
        case ADD_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.ival = o1.value.ival + o2.value.ival;
            }
            else{
                if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = o1.value.dval + o2.value.dval;
                }
                else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = (double) o1.value.ival + o2.value.dval;
                } else{
                    result.value.dval = o1.value.dval + (double) o2.value.ival;
                }
            }
            break;
        case SUB_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.ival = o1.value.ival - o2.value.ival;
            }
            else{
                if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = o1.value.dval - o2.value.dval;
                }
                else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = (double) o1.value.ival - o2.value.dval;
                } else{
                    result.value.dval = o1.value.dval - (double) o2.value.ival;
                }
            }
            break;
        case MULT_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.ival = o1.value.ival * o2.value.ival;
            }
            else{
                if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = o1.value.dval * o2.value.dval;
                }
                else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = (double) o1.value.ival * o2.value.dval;
                } else{
                    result.value.dval = o1.value.dval * (double) o2.value.ival;
                }
            }
            break;
        case DIV_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.dval = (double) o1.value.ival / (double) o2.value.ival;
            }
            else if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = o1.value.dval / o2.value.dval;
            }
            else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = (double) o1.value.ival / o2.value.dval;
            } else{
                result.value.dval = o1.value.dval / (double) o2.value.ival;
            }

            break;
        case REMAINDER_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.ival = o1.value.ival % o2.value.ival;
            }
            else{
                if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = remainder(o1.value.dval, o2.value.dval);
                }
                else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                    result.value.dval = remainder((double) o1.value.ival, o2.value.dval);
                } else{
                    result.value.dval = remainder(o1.value.dval, (double) o2.value.ival);
                }
            }
            break;
        case LOG_OPER:
            switch (o1.type){
                case INT_TYPE:
                    result.value.dval = log((double) o1.value.ival);
                    break;
                case DOUBLE_TYPE:
                    result.value.dval = log(o1.value.dval);
                    break;
            }
            break;
        case POW_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.dval = pow((double) o1.value.ival, (double) o2.value.ival);
            }else if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = pow(o1.value.dval, o2.value.dval);
            }
            else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = pow((double) o1.value.ival, o2.value.dval);
            } else{
                result.value.dval = pow(o1.value.dval, (double) o2.value.ival);
            }
            break;
        case MAX_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.dval = fmax((double) o1.value.ival, (double) o2.value.ival);
            }
            else if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = fmax(o1.value.dval, o2.value.dval);
            }
            else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = fmax((double) o1.value.ival, o2.value.dval);
            } else{
                result.value.dval = fmax(o1.value.dval, (double) o2.value.ival);
            }

            break;
        case MIN_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.dval = fmin((double) o1.value.ival, (double) o2.value.ival);
            }
            else if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = fmin(o1.value.dval, o2.value.dval);
            }
            else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = fmin((double) o1.value.ival, o2.value.dval);
            } else{
                result.value.dval = fmin(o1.value.dval, (double) o2.value.ival);
            }
            break;
        case EXP2_OPER:
            if(o1.type == INT_TYPE) result.value.dval = exp2(o1.value.ival);
            else result.value.dval = exp2(o1.value.dval);
            break;
        case CBRT_OPER:
            if(o1.type == INT_TYPE) result.value.dval = cbrt((double) o1.value.ival);
            else result.value.dval = cbrt(o1.value.dval);
            break;
        case HYPOT_OPER:
            if (o1.type == INT_TYPE && o2.type == INT_TYPE){
                result.value.dval = hypot((double) o1.value.ival, (double) o2.value.ival);
            }
            else if (o1.type == DOUBLE_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = hypot(o1.value.dval, o2.value.dval);
            }
            else if (o1.type == INT_TYPE && o2.type == DOUBLE_TYPE){
                result.value.dval = hypot((double) o1.value.ival, o2.value.dval);
            } else{
                result.value.dval = hypot(o1.value.dval, (double) o2.value.ival);
            }
            break;
    }
    return result;
}

// prints the type and value of a RET_VAL
void printRetVal(RET_VAL val)
{
    // TODO print the type and value of the value passed in. done
    printf("Type: ");
    switch (val.type){
        case INT_TYPE:
            printf("Integer, Value %ld\n", val.value.ival);
            break;
        case DOUBLE_TYPE:
            printf("Double, Value %.2lf\n", val.value.dval);
            break;
    }
}

AST_NODE *lookup(AST_NODE *node){
    char *search = node->data.symbol.identifier;
    while (node != NULL) {
        SYM_TABLE_NODE *currentTable = node->table;
        while (currentTable->next != NULL) {
            if (strcmp(currentTable->id, search) == 0) {
                return currentTable->value;
            }
        }
        node = node->parent;
    }

    yyerror("Invalid symbol given!");

    return NULL;
}


