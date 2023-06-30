#include "codegen.h"
#include "emitcode.h"

void outputHeader(char* srcFile);

int instructionCount = 38;


void traverse(FILE* out, TreeNode* node, SymbolTable* symtab) {
	switch (node->nodekind) {
		case NodeKind::DeclK:
			switch (node->kind.decl) {
				case DeclKind::FuncK:
					emitLongComment();
					emitComment("FUNCTION", node->attr.name);
					emitComment("TOFF set:", "UNKNOWN");
					//Do some other stuff
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
	traverse(codeOut, syntaxTree, symtab);
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