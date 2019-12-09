Created by Nathan Moreno 

Model 1 (11-6-19)
- Finished implementing createNumberNode
- Finished implementing createFunctionNode
- Finished implementing evalNumNode
- Finished implementing printRetVal

Model 2 (11-9-19)
- Finished current implementation of eval
- Added helper function returnTypeSet
- Added letter tokenizer to lex file

Model 3 (11-12-19)
- Debugged Model 2, fully functional save for lex catching character '' as an error.
- Added symbol structs and helper functions 
- Function linkSymbolTable added
- Function addToSymbolTable added
- Function createSymbolTableNode added
- Function createSymbolNode added
- Function evalSymNode added
- Function lookup added
- ~~Needs critical debugging (ask Ryan)~~

Model 4 (11-13-19)
- Debugged Model 3 successfully
- Added casting fnctionality (int and double)
- NUM_TYPE NO_TYPE added
- LVAL IS GONE all work done in dval now
- Added warnings for double -> int precision loss
- added printing functionality! 
- Function printFunc added 
- ~~NOTE: sqrt is defaulting to null, fix!~~

Model 5 (11-20-19)
- added multiple parameter functionalty
- removed resultTypeSetter
- added s_expr_list to yacc files 
- added function addToS_exprList
- fixed sqrt bug
- modified various functions to account for opList

Model 6 (11-27-19)
- added read functionality
- updated lex and yacc files
- modified various functions
- ~~will need to debug carriage form feed in scanf~~

Model 7 (12-1-19)
- fixed carriage form feed in READ oper
- removed resultTypeSetter
- added less greater and equal comparisons
- read functionality debugged
- print function reworked
- evalFunc function reworked
- added cond functionality

Model 8 (12-4-19)
- implementing custom funcs
- evalForArg, createArgTableNode, addToArgTable, createLambdaSymbolTableNode, freeRetValList functions added
- arg type added
- RET_VAL_LIST added
- ~~TODO: DEBUG line 602 and find out why it ruins the list pointer~~
- test function: ((let (f lambda (x y) (add x y))) (f 1 (f 2 3)))

Model 9 (12-6-19)
- line 602 debugged. ciLisp is fully operational
- ~~TODO: debug further come Monday~~

Model 10 (12-9-19)
- added errors/warnings for too little/many parameters for custom functions, respectively
- cleaned up memory leak made by createLambdaSymbolTableNode


Known Issues:
- Custom recursive functions only work when stepped through or when run with valgrind memcheck no idea why

Helper Function Desciptions:
- lookup: looks up symbol and returns associated node
- linkSymbolTable: links symbol table to associated node
- addToS_exprList: adds new s_expr to list
- createLambdaSymbolTableNode: creates a function node with the associated symbol and custom operations
- evalForArg: evaluates parameters to be used for arg list insertion
- printFunc: Function used by PRINT to print evaluated function with formatting
- freeRetValList: frees the RET_VAL_LIST created for evalForArg


