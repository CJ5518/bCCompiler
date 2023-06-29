#include "semantics.h"
#include "treeUtils.h"
#include "parser.tab.h"


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

bool shouldChangeScope(TreeNode* node) {
	//Statements oftentimes create new scopes
	if (node->nodekind == NodeKind::StmtK) {
		switch (node->kind.stmt) {
			case StmtKind::BreakK:
			case StmtKind::ReturnK:
			case StmtKind::RangeK:
			case StmtKind::ForK:
			case StmtKind::WhileK:
			case StmtKind::IfK:
			break;
			case StmtKind::CompoundK:
				return true;
			break;
			default:
			printf("CJERROR: Ran into default case in a switch on kind.stmt, somethings wrong\n");
		}
	}

	if (node->nodekind == NodeKind::DeclK) {
		if (node->kind.decl == DeclKind::FuncK) {
			return true;
		}
	}
	return false;
}

void traverse(TreeNode* syntaxTree, SymbolTable* symtab, bool isFuncSpecialCase=false) {
	if (!syntaxTree)
		return;

	//Used pretty universally, typicall 'the' id in question
	char* id = strdup(syntaxTree->attr.name);

	//Expression kind is the important one for the USAGE semantic analysis
	if (syntaxTree->nodekind == NodeKind::ExpK) {
		switch (syntaxTree->kind.exp) {
			case ExpKind::CallK:
				syntaxTree->type = ((TreeNode*)symtab->lookup(id))->type;
				break;
			default:
			break;
		}
	}

	if (shouldChangeScope(syntaxTree) && !isFuncSpecialCase) {
		symtab->enter("NewScope");
	}

	//DeclK is important for the symbol table
	if (syntaxTree->nodekind == NodeKind::DeclK) {
		if (!symtab->insert(id, (void*)syntaxTree) && syntaxTree->lineno != -1) {
			//This is the part where an error needs to be thrown if there is a redefinition
			printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line UNKNOWN.\n", syntaxTree->lineno, id);
			exit(1);
		}
	}

	traverse(syntaxTree->child[0], symtab);
	traverse(syntaxTree->child[1], symtab, //Handle the function compound special case
		syntaxTree->child[1] &&
		syntaxTree->nodekind == NodeKind::DeclK &&
		syntaxTree->kind.decl == DeclKind::FuncK &&
		syntaxTree->child[1]->nodekind == NodeKind::StmtK &&
		syntaxTree->child[1]->kind.stmt == StmtKind::CompoundK
	);
	traverse(syntaxTree->child[2], symtab);
	traverse(syntaxTree->sibling, symtab);

	if (shouldChangeScope(syntaxTree) && !isFuncSpecialCase) {
		symtab->leave();
	}
}

TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int& globalOffset) {
	syntaxTree = loadIOLib(syntaxTree);
	traverse(syntaxTree, symtab);
	return syntaxTree;
}
