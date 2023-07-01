#include "codegen.h"
#include "emitcode.h"

void outputHeader(char* srcFile);


//Not sure about this one
int foffset = -2;
int toffset = -2;
//Not sure about this one
int goffset = -2;


void traverse(TreeNode* node, SymbolTable* symtab);

void caseDeclK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.decl) {
		case DeclKind::FuncK:
			emitLongComment();
			emitComment("FUNCTION", node->attr.name);
			//TOFFset is -2 - parameterCount, so we count the parameters here
			//Also we count the child, so we do -3, which works because siblingCount returns -1 if there is no child
			toffset = -3 - siblingCount(node->child[0]);
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
		toffset -= siblingCount(node->child[0]) + 1;
		emitComment("TOFF set:", toffset);

		//Do its body
		emitComment("Compound Body");
		traverse(node->child[1], symtab);

		//Undo toffset changes, end compound
		toffset += siblingCount(node->child[0]) + 1;
		emitComment("TOFF set:", toffset);
		emitComment("END COMPOUND");
		break;
	}
}
void caseExpK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.exp) {
		case ExpKind::AssignK:
			emitComment("EXPRESSION");
			switch (node->attr.op) {
				case '=':
				break;
			}
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
		break;
	}
}

void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}
	traverse(syntaxTree, symtab);
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
	emitComment("");
}