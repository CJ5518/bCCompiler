#include "codegen.h"
#include "emitcode.h"

void outputHeader(char* srcFile);



int foffset = -2;
int toffset = -2;
//Not sure about this one
int goffset = -2;

void traverse(TreeNode* node, SymbolTable* symtab) {
	if (!node) return;

	switch (node->nodekind) {
		case NodeKind::DeclK:
			switch (node->kind.decl) {
				case DeclKind::FuncK:
					emitLongComment();
					emitComment("FUNCTION", node->attr.name);
					//TOFFset is -2 - parameterCount, so we count the parameters here
					//Also we count the child, so we do -3
					emitComment("TOFF set:", -3 - siblingCount(node->child[0]));
					//Do some other stuff
					emitRM("ST", 3,-1,1, "Store return address");
					//traverse(node->child[0], symtab);
					emitStandardClosing();
			}
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