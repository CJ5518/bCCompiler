#include "semantics.h"
#include "treeUtils.h"


TokenData* makeDummyTokenData(const char* name) {
	TokenData* data = new TokenData();
	data->svalue = (char*)name;
	data->linenum = -1;
	return data;
}

TreeNode* makeDummyParm(ExpType tip) {
	if (tip == ExpType::UndefinedType) {
		return NULL;
	}
	return newDeclNode(DeclKind::ParamK, tip, makeDummyTokenData("*dummy*"));
}

TreeNode* newIOFunc(const char* name, ExpType ret, ExpType parm=ExpType::UndefinedType) {
	return  newDeclNode(DeclKind::FuncK, ret, makeDummyTokenData(name), makeDummyParm(parm));
}

TreeNode* loadIOLib(TreeNode* syntaxTree) {
	TreeNode* ioLib = newIOFunc("input", ExpType::Integer);
	//Return the root node, save it here
	TreeNode* og = ioLib;
	ioLib->sibling = newIOFunc("output", ExpType::Void, ExpType::Integer);
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("inputb", ExpType::Boolean);
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outputb", ExpType::Void, ExpType::Boolean);
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("inputc", ExpType::Char);
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outputc", ExpType::Void, ExpType::Char);
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outnl", ExpType::Void);
	ioLib = ioLib->sibling;
	//Append the rest of the syntax tree
	ioLib->sibling = syntaxTree;
	return og;
}

TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int& globalOffset) {
	syntaxTree = loadIOLib(syntaxTree);
	return syntaxTree;
}
