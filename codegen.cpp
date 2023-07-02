#include "codegen.h"
#include "emitcode.h"

void outputHeader(char* srcFile);


//Not sure about this one
int foffset = -2;
int toffset = -2;
//Not sure about this one
int goffset = -2;

//The start of the main function, need this for the very end
int mainIndex = -1;


//Helper functions
//Returns -1 if node is NULL
int siblingCountWithoutStatics(TreeNode* node) {
	if (!node)
		return -1;
	int q = 0;
	if (node->isStatic)
		q--;
	while (node->sibling) {
		node = node->sibling;
		if (!node->isStatic)
			q++;
	}
	return q;
}


void traverse(TreeNode* node, SymbolTable* symtab);

void caseDeclK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.decl) {
		case DeclKind::FuncK:
			emitComment("");
			emitLongComment();
			emitComment("FUNCTION", node->attr.name);

			if (strcmp("main", node->attr.name) == 0) {
				mainIndex = emitWhereAmI();
			}


			//TOFFset is -2 - parameterCount, so we count the parameters here
			//Also we count the child, so we do -3, which works because siblingCount returns -1 if there is no child
			toffset = -3 - siblingCountWithoutStatics(node->child[0]);
			emitComment("TOFF set:", toffset);
			//Do some other stuff
			emitRM("ST", 3,-1,1, "Store return address");
			//Traverse the funcs other child, either a compound or some other stmt
			traverse(node->child[1], symtab);
			emitStandardClosing();
			emitComment("END FUNCTION", node->attr.name);
			break;
	}
}
void caseStmtK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.stmt) {
		case StmtKind::CompoundK:
		//Start a compound
		emitComment("COMPOUND");
		toffset -= siblingCountWithoutStatics(node->child[0]) + 1;
		emitComment("TOFF set:", toffset);

		//Do its body
		emitComment("Compound Body");
		traverse(node->child[1], symtab);

		//Undo toffset changes, end compound
		toffset += siblingCountWithoutStatics(node->child[0]) + 1;
		emitComment("TOFF set:", toffset);
		emitComment("END COMPOUND");
		break;
	}
}

//Bit of a rough function right now, not sure how it will interact with arrays
//and the rest of the code, useful for the globals and statics only right now
//Used in syntax like int x:4;
void emitConstantVariable(TreeNode* node, int where) {
	if (!node || node->nodekind != NodeKind::DeclK || node->kind.decl != DeclKind::VarK) {
		return;
	}
	if (!node->child[0])
		return;
	if (node->child[0]->nodekind != NodeKind::ExpK || node->child[0]->kind.exp != ExpKind::ConstantK)
		return;
	char* name = node->attr.name;

	switch (node->type) {
		case ExpType::Integer:
		emitRM("LDC", 3, node->child[0]->attr.value, 6, "Load integer constant");
		break;
		case ExpType::Boolean:
		emitRM("LDC", 3, node->child[0]->attr.value, 6, "Load Boolean constant");
		break;
		case ExpType::Char:
		emitRM("LDC", 3, node->child[0]->attr.cvalue, 6, "Load char constant");
		break;
		case ExpType::String:
		emitRM("LDC", 3, node->child[0]->attr.value, 6, "Load STRING constant");
		break;
	}

	emitRM("ST", 3, where, 0, "Store variable", name);
}

void caseExpK(TreeNode* node, SymbolTable* symtab) {
	emitComment("EXPRESSION");
	switch (node->kind.exp) {
		case ExpKind::CallK: {
			emitComment("CALL", node->attr.name);
		}
		break;
		case ExpKind::AssignK:
		break;
	} 
}

void traverse(TreeNode* node, SymbolTable* symtab) {
	if (!node) return;

	switch (node->nodekind) {
		//DECLARATION KIND
		case NodeKind::DeclK:
		caseDeclK(node, symtab);
		break;
		//STATEMENT KIND
		case NodeKind::StmtK:
		caseStmtK(node, symtab);
		break;
		//EXPRESSION KIND
		case NodeKind::ExpK:
		caseExpK(node, symtab);
		break;
	}

	traverse(node->sibling, symtab);
}

void doGlobalsAndStatics(TreeNode* node, bool isSiblingOfRoot = true);

void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}
	traverse(syntaxTree, symtab);
	//After the tree is done, write out the final segment
	backPatchAJumpToHere(0, "Jump to init [backpatch]");

	emitComment("INIT");

	//Doesn't count statics, no good, put this all in doGlobalsAndStatics
	//emitRM("LDA", 1, -symtab->countGlobalVariables(), 0, "set first frame at end of globals");
	//emitRM("ST", 1,0,1,"store old fp (point to self)");
	doGlobalsAndStatics(syntaxTree);
}

//This one gives the order to print them in
std::multimap<std::string, TreeNode*> globalsAndStatics;
//This one gives us the actual index
std::vector<TreeNode*> globalsAndStaticsIndex;

void doGlobalsAndStatics(TreeNode* node, bool isSiblingOfRoot) {
	if (!node) return;
	if (isSiblingOfRoot) {
		if (node->nodekind == NodeKind::DeclK) {
			if (node->kind.decl == DeclKind::FuncK) {
				//Is a function, check it for statics
				doGlobalsAndStatics(node->child[0], false);
				doGlobalsAndStatics(node->child[1], false);
				doGlobalsAndStatics(node->child[2], false);
			} else if (node->kind.decl == DeclKind::VarK) {
				//A global var
				globalsAndStatics.insert(std::pair<std::string, TreeNode*>((std::string)node->attr.name, node));
				globalsAndStaticsIndex.push_back(node);
			} else {
				printf("CJERROR: Bad node->kind.decl is sibling of root\n");
			}
		} else {
			printf("CJERROR: Bad node is sibling of root\n");
		}
	} else {
		if (node->nodekind == NodeKind::DeclK && node->kind.decl == DeclKind::VarK && node->isStatic) {
			globalsAndStatics.insert(std::pair<std::string, TreeNode*>((std::string)node->attr.name, node));
			globalsAndStaticsIndex.push_back(node);
		} else {
			doGlobalsAndStatics(node->child[0], false);
			doGlobalsAndStatics(node->child[1], false);
			doGlobalsAndStatics(node->child[2], false);
			doGlobalsAndStatics(node->sibling, false);
		}
		return;
	}
	if (node->sibling) {
		doGlobalsAndStatics(node->sibling);
	} else {
		//We're done, we've traversed the tree
		emitRM("LDA", 1, -globalsAndStaticsIndex.size(), 0, "set first frame at end of globals");
		emitRM("ST", 1,0,1, "store old fp (point to self)");
		emitComment("INIT GLOBALS AND STATICS");

		//For every entry
		for (std::multimap<std::string , TreeNode*>::iterator it=globalsAndStatics.begin(); it!=globalsAndStatics.end(); it++) {
			//action(it->first, it->second);
			for (int q = 0; q < globalsAndStaticsIndex.size(); q++) {
				if (it->second == globalsAndStaticsIndex[q]) {
					emitConstantVariable(it->second, -q);
				}
			}
		}
		emitComment("END INIT GLOBALS AND STATICS");
		emitRM("LDA", 3,1,7, "Return address in ac");
		emitRM("JMP", 7, mainIndex - emitWhereAmI() - 1, 7, "Jump to main");
		emitRO("HALT", 0,0,0,"DONE!");
		emitComment("END INIT");
	}
}





void outputHeader(char* srcFile) {
	//Print header
	emitComment("bC compiler version bC-Su23");
	emitComment("File compiled: ", srcFile);
	emitComment("");
	emitLongComment();
	//Output the IO code
	//https://stackoverflow.com/a/3501681
	FILE* fp;
	char* line = NULL;
	size_t len = 0;
	ssize_t read;

	fp = fopen("./ioCode.txt", "r");
	if (fp == NULL) {
		printf("CJERROR: Could not open the ./ioCode.txt file!\n");
		exit(EXIT_FAILURE);
	}

	while ((read = getline(&line, &len, fp)) != -1) {
		printf("%s", line);
	}

	fclose(fp);
	if (line)
		free(line);
}