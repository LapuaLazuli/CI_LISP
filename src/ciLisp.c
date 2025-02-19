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
AST_NODE *createFunctionNode(char *funcName, AST_NODE *opList)
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
    node->data.function.opList = opList;
    AST_NODE *curNode = node->data.function.opList;
    if (node->data.function.oper == CUSTOM_OPER){
        node->data.function.ident = funcName;
    }
    while (curNode != NULL){
        curNode->parent = node;
        curNode = curNode->next;
    }
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
    node->type = VARIABLE_TYPE;
    if (type == NULL){
        node->val_type = NO_TYPE;
    } else if(strcmp("double", type) == 0) node->val_type = DOUBLE_TYPE;
    else node->val_type = INT_TYPE;

    return node;

}

SYM_TABLE_NODE *createLambdaSymbolTableNode(AST_NODE *value, char *id, char *type, ARG_TABLE_NODE *arg){
    SYM_TABLE_NODE *node;
    size_t nodeSize;

    nodeSize = sizeof(SYM_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = LAMBDA_TYPE;
    node->value = value;
    node->id = id;
    node->value->argTable = arg;
    if (type == NULL){
        node->val_type = NO_TYPE;
    } else if(strcmp("double", type) == 0) node->val_type = DOUBLE_TYPE;
    else node->val_type = INT_TYPE;
    return node;

}


ARG_TABLE_NODE *createArgTableNode(char *id){
    ARG_TABLE_NODE *node;
    size_t nodeSize;
    nodeSize = sizeof(ARG_TABLE_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");
    node->ident = id;
    return node;

}

SYM_TABLE_NODE *addToSymbolTable(SYM_TABLE_NODE *root, SYM_TABLE_NODE *new){
    new->next = root;
    return new;
}

ARG_TABLE_NODE *addToArgTable(ARG_TABLE_NODE *root, char *new){
    ARG_TABLE_NODE *temp = createArgTableNode(new);
    temp->next = root;
    return temp;
}

AST_NODE *linkSymbolTable(SYM_TABLE_NODE *table, AST_NODE *node){
    node->table = table;
    AST_NODE *traversal = node->table->value;
    while (traversal != NULL){
        traversal->parent = node;
        traversal = traversal->next;
    }
    return node;
}

AST_NODE *createCondNode(AST_NODE *cond, AST_NODE *trueSec, AST_NODE *falseSec){
    AST_NODE *node;
    size_t nodeSize;

    // allocate space (or error)
    nodeSize = sizeof(AST_NODE);
    if ((node = calloc(nodeSize, 1)) == NULL)
        yyerror("Memory allocation failed!");

    node->type = COND_NODE_TYPE;
    node->data.condition.cond = cond;
    node->data.condition.nodeTrue = trueSec;
    node->data.condition.nodeFalse = falseSec;

    node->data.condition.cond->parent = node;
    node->data.condition.nodeTrue->parent = node;
    node->data.condition.nodeFalse->parent = node;

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
        freeNode(node->data.function.opList);

        // Free up identifier string if necessary
        if (node->data.function.oper == CUSTOM_OPER)
        {
            free(node->data.function.ident);
        }
    }

    if (node->table != NULL){
        SYM_TABLE_NODE *current = node->table;
        while(current != NULL){
            SYM_TABLE_NODE *temp = current;
            current = current->next;
            free(temp);
        }
    }

    if(node->type == SYM_NODE_TYPE){
        free(node->data.symbol.identifier);
    }

    if (node->argTable != NULL){
        ARG_TABLE_NODE *current = node->argTable;
        while (current != NULL){
            ARG_TABLE_NODE *temp = current;
            current = current->next;
            free(temp);
        }
    }

    if (node->next != NULL){
        freeNode(node->next);
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
            result = evalFuncNode(node);
            break;
        case SYM_NODE_TYPE:
            result = evalSymNode(&node->data.symbol, node);
            break;
        case COND_NODE_TYPE:
            result = evalCondNode(&node->data.condition);
            break;

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
    AST_NODE *done = lookup(symNode->identifier, node);
    return eval(done);
}


RET_VAL evalFuncNode(AST_NODE *node)
{
    if (!node)
        return (RET_VAL){INT_TYPE, NAN};

    FUNC_AST_NODE *funcNode = &(node->data.function);

    RET_VAL result = {INT_TYPE, NAN};

    // TODO populate result with the result of running the function on its operands.
    // SEE: AST_NODE, AST_NODE_TYPE, FUNC_AST_NODE
    AST_NODE *traversal = funcNode->opList;
    double op1, op2;
    switch (funcNode->oper){
        case NEG_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.value.dval *= -1;
            break;

        case ABS_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.value.dval = fabs(result.value.dval);
            break;

        case EXP_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(funcNode->opList);
            break;

        case SQRT_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.type = DOUBLE_TYPE;
            result.value.dval = sqrt(result.value.dval);
            break;

        case ADD_OPER:
            result.value.dval = 0;
            result.type = INT_TYPE;
            while (traversal != NULL){
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                result.value.dval += eval(traversal).value.dval;
                traversal = traversal->next;
            }
            break;

        case SUB_OPER:
            result.value.dval = eval(traversal).value.dval;
            result.type = INT_TYPE;
            while (traversal != NULL){
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                traversal = traversal->next;
                result.value.dval -= eval(traversal).value.dval;
                traversal = traversal->next;
            }
            break;

        case MULT_OPER:
            result.type = INT_TYPE;
            if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                result.type = DOUBLE_TYPE;
            }
            result.value.dval = eval(traversal).value.dval;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function mult\n");
                exit(1);
            } else{
                traversal = traversal->next;
                while (traversal != NULL){
                    if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                        result.type = DOUBLE_TYPE;
                    }
                    result.value.dval *= eval(traversal).value.dval;
                    traversal = traversal->next;
                }
            }
            break;

        case DIV_OPER:
            result.type = DOUBLE_TYPE;
            result.value.dval = eval(traversal).value.dval;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function div\n");
                exit(1);
            } else{
                traversal = traversal->next;
                while (traversal != NULL){
                    result.value.dval /= eval(traversal).value.dval;
                    traversal = traversal->next;
                }
            }
            break;

        case REMAINDER_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function remainder\n");
                exit(1);
            } else{
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                op2 = eval(traversal).value.dval;
                result.value.dval = remainder(op1, op2);
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case LOG_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.type = DOUBLE_TYPE;
            result.value.dval = log(result.value.dval);
            break;

        case POW_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function pow\n");
                exit(1);
            } else{
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                if (traversal->type == NUM_NODE_TYPE && traversal->data.number.type == DOUBLE_TYPE){
                    result.type = DOUBLE_TYPE;
                }
                op2 = eval(traversal).value.dval;
                result.value.dval = pow(op1, op2);
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case MAX_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function max\n");
                exit(1);
            } else{
                RET_VAL ret = eval(traversal);
                op1 = ret.value.dval;
                NUM_TYPE type1 = ret.type;
                traversal = traversal->next;
                ret = eval(traversal);
                op2 = ret.value.dval;
                NUM_TYPE type2 = ret.type;
                result.value.dval = fmax(op1, op2);
                if ((op1 >= op2 && type1 == DOUBLE_TYPE) || (op2 >= op1 && type2 == DOUBLE_TYPE)) result.type = DOUBLE_TYPE;
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case MIN_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function min\n");
                exit(1);
            } else{
                RET_VAL ret = eval(traversal);
                op1 = ret.value.dval;
                NUM_TYPE type1 = ret.type;
                traversal = traversal->next;
                ret = eval(traversal);
                op2 = ret.value.dval;
                NUM_TYPE type2 = ret.type;
                result.value.dval = fmin(op1, op2);
                if ((op1 <= op2 && type1 == DOUBLE_TYPE) || (op2 <= op1 && type2 == DOUBLE_TYPE)) result.type = DOUBLE_TYPE;
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case EXP2_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.value.dval = exp2(result.value.dval);
            break;

        case CBRT_OPER:
            if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            result = eval(traversal);
            result.type = DOUBLE_TYPE;
            result.value.dval = cbrt(result.value.dval);
            break;

        case HYPOT_OPER:
            result.type = DOUBLE_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function hypot\n");
                exit(1);
            } else{
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                op2 = eval(traversal).value.dval;
                result.value.dval = hypot(op1, op2);
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case PRINT_OPER:{
            AST_NODE *temp = funcNode->opList;
            RET_VAL tem;
            while (temp != NULL){
                tem = eval(temp);
                temp = temp->next;
            }
            printf("PRINT: ");
            printFunc(funcNode->opList);
            printf("\n");

            result = tem;
                break;
            }

        case READ_OPER: {
            char temp[BUFSIZ];
            printf("read: ");
            scanf("%s", temp);
            getchar();
            if (strchr(temp, '.') != NULL) result.type = DOUBLE_TYPE;
            else result.type = INT_TYPE;
            result.value.dval = strtod(temp, NULL);
            node->type = NUM_NODE_TYPE;
            node->data.number.type = result.type;
            node->data.number.value.dval = result.value.dval;
            break;
        }

        case RAND_OPER:
            result.type = DOUBLE_TYPE;
            result.value.dval = ((double) rand() / RAND_MAX);
            node->type = NUM_NODE_TYPE;
            node->data.number.type = result.type;
            node->data.number.value.dval = result.value.dval;
            break;

        case LESS_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function less\n");
                exit(1);
            } else{
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                op2 = eval(traversal).value.dval;
                if (op1 < op2) result.value.dval = 1;
                else result.value.dval = 0;
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case GREATER_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function greater\n");
                exit(1);
            } else{
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                op2 = eval(traversal).value.dval;
                if (op1 > op2) result.value.dval = 1;
                else result.value.dval = 0;
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case EQUAL_OPER:
            result.type = INT_TYPE;
            if (traversal->next == NULL){
                yyerror("ERROR: Too few parameters for function equal\n");
                exit(1);
            } else{
                op1 = eval(traversal).value.dval;
                traversal = traversal->next;
                op2 = eval(traversal).value.dval;
                if (op1 == op2) result.value.dval = 1;
                else result.value.dval = 0;
                if (traversal->next != NULL) printf("WARNING: Too many parameters for func %s\n", funcNames[funcNode->oper]);
            }
            break;

        case CUSTOM_OPER: {
            RET_VAL_LIST *list = evalForArg(traversal);
            RET_VAL_LIST *root = list;
            AST_NODE *func = lookup(funcNode->ident, node);
            ARG_TABLE_NODE *currentArg = func->argTable;
            while (list != NULL && currentArg != NULL){
                currentArg->val = &list->val;
                currentArg = currentArg->next;
                list = list->next;
            }
            if(currentArg != NULL){
                yyerror("ERROR: NOT ENOUGH PARAMETERS FOR CUSTOM FUNCTION");
                exit(1);
            }
            if (list != NULL){
                printf("WARNING!: Too many parameters for function! Will only use the first in the list!");
            }
            freeRetValList(root);
            result = eval(func);
            break;
        }
    }
    return result;
}

char *operNames[] = {"negate", "absolute value of", "base e exponent of",
                  "square root of", "add", "subtract", "multiply", "divide", "remainder of", "logarithm of",
                  "power of", "maximum of", "minimum of", "base 2 exponent of",
                  "cube root of", "hypotenuse of", "reading", "randing", "printing"};

void printFunc(AST_NODE *node){
    double num = 0;
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
            //if (node->next != NULL) printFunc(node->next);
            break;
        case FUNC_NODE_TYPE:
            printf("( %s ", operNames[node->data.function.oper]);
            printFunc(node->data.function.opList);

            if (node->data.function.opList->next != NULL) {
                printf("with ");
                printFunc(node->data.function.opList->next);
            } else printf(")");
            break;
        case SYM_NODE_TYPE: {
            RET_VAL temp = eval(node);
            switch (temp.type) {
                case INT_TYPE:
                    printf("%.0lf ", temp.value.dval);
                    break;
                case DOUBLE_TYPE:
                    printf("%.2lf ", temp.value.dval);
                    break;
            }
            if (node->next != NULL) printFunc(node->next);
        }
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

AST_NODE *lookup(char *search, AST_NODE *origin){
    while (origin != NULL) {
        SYM_TABLE_NODE *currentTable = origin->table;
        ARG_TABLE_NODE *currentArgTable = origin->argTable;
        while (currentTable != NULL) {
            if (strcmp(currentTable->id, search) == 0) {
                if (currentTable->value->type == NUM_NODE_TYPE) {
                    if (currentTable->val_type != NO_TYPE) {
                        if (currentTable->val_type == INT_TYPE &&
                            currentTable->value->data.number.type == DOUBLE_TYPE) {
                            printf("WARNING: Precision loss in variable %s\n", search);
                        }
                        currentTable->value->data.number.type = currentTable->val_type;
                    }
                }
                return currentTable->value;
            }
            currentTable = currentTable->next;

        }
        while (currentArgTable != NULL){
            if (strcmp(search, currentArgTable->ident) == 0){
                AST_NODE temp = *origin;
                temp.type = NUM_NODE_TYPE;
                temp.data.number = *currentArgTable->val;
                return &temp;
            }
            currentArgTable = currentArgTable->next;
        }
        origin = origin->parent;
    }

    yyerror("Invalid symbol given!\n");
    exit(1);
}

AST_NODE *addToS_exprList(AST_NODE *new, AST_NODE *base){
    new->next = base;
    return new;
}

RET_VAL evalCondNode(COND_AST_NODE *condNode){
    RET_VAL result;
    double temp = eval(condNode->cond).value.dval;
    if (temp != 0) result = eval(condNode->nodeTrue);
    else result = eval(condNode->nodeFalse);
    return result;
}

RET_VAL_LIST *evalForArg(AST_NODE *current){
    RET_VAL_LIST *root;
    root = calloc(sizeof(RET_VAL_LIST), 1);
    RET_VAL tem = eval(current);
    root->val = tem;
    current = current->next;
    RET_VAL_LIST *cur = root;
    while (current != NULL){
        RET_VAL_LIST *value = calloc(sizeof(RET_VAL_LIST), 1);
        tem = eval(current);
        value->val = tem;
        cur->next = value;
        cur = cur->next;
        current = current->next;
    }

    return root;
}

void freeRetValList(RET_VAL_LIST *root){
    while (root != NULL){
        RET_VAL_LIST *temp = root;
        root = root->next;
        free(temp);
    }
}




