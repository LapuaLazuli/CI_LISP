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
- will need to debug carriage form feed in scanf