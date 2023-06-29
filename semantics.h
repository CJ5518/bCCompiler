#ifndef SEMANTICS_H
#define SEMANTICS_H
#include "treeNodes.h"

class SymbolTable {

};

class Scope {

};

TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int globalOffset);


#endif
