#include "semantics.h"
#include "treeUtils.h"

TreeNode* loadIOLib(TreeNode* syntaxTree) {
	TreeNode* ioLib = newDeclNode(DeclKind::FuncK, ExpType::Integer);
	return syntaxTree;
}

TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int& globalOffset) {
	return syntaxTree;
}
