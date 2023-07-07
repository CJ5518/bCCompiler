#include "codegen.h"
#include "emitcode.h"

void outputHeader(char* srcFile);


//Not sure about this one
int foffset = -2;
int toffset = -2;
int goffset = 0;

//Flags
bool shouldPrintExpression = true;

void toffDec() {
	toffset--;
	emitComment("TOFF dec:", toffset);
}
void toffInc() {
	toffset++;
	emitComment("TOFF inc:", toffset);
}

//The start of the main function, need this for the very end
int mainIndex = -1;


//Returns 0 if node is NULL
int countOffsets(TreeNode* node) {
	if (!node)
		return 0;
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


void traverseGen(TreeNode* node, SymbolTable* symtab);

void caseDeclK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.decl) {
		//This isn't very good, needs some fixing up
		case DeclKind::FuncK:
			emitComment("");
			emitLongComment();
			//goffset--;
			emitComment("FUNCTION", node->attr.name);

			if (strcmp("main", node->attr.name) == 0) {
				mainIndex = emitWhereAmI();
			}


			toffset += node->offset;
			emitComment("TOFF set:", toffset);
			//Do some other stuff
			emitRM("ST", 3,-1,1, "Store return address");
			//traverseGen the funcs other child, either a compound or some other stmt
			traverseGen(node->child[1], symtab);
			emitStandardClosing();
			emitComment("END FUNCTION", node->attr.name);
			toffset -= node->offset;
			//goffset++;
			break;
		case DeclKind::VarK: {
		} break;
	}
}
void caseStmtK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.stmt) {
		case StmtKind::CompoundK:
		//Start a compound
		emitComment("COMPOUND");
		toffset -= node->offset;
		emitComment("TOFF set:", toffset);

		//Do its body
		emitComment("Compound Body");
		traverseGen(node->child[1], symtab);

		toffset += node->offset;
		emitComment("TOFF set:", toffset);
		emitComment("END COMPOUND");
		break;
	}
}

void caseExpK(TreeNode* node, SymbolTable* symtab) {
	//Code generation

	if (shouldPrintExpression)
		emitComment("EXPRESSION");
	switch (node->kind.exp) {
		case ExpKind::CallK: {
			emitComment("CALL", node->attr.name);
		}
		break;
		case ExpKind::AssignK:
		break;
		case ExpKind::ConstantK: {
			switch (node->type) {
				case ExpType::String:
				emitStrLit(node->offset, node->attr.string);
				break;
				case ExpType::Integer:
				emitRM("LDC", 3, node->attr.value, 6, "Load integer constant");
				break;
			}
		} break;
		case ExpKind::IdK: {
			if (node->isArray) {
				emitRM("LD", 3, node->offset, 1,
				"Load address of base of array", node->attr.name);
			} else {
				emitRM("LD", 3, node->offset, 1, "Load variable", node->attr.name);
			}
		} break;
		case ExpKind::OpK: {
			
			//Pre-op
			switch (node->attr.op) {
				case '[': {
				}
			}

			//Common code (for ops with 2 args)

			bool oldShouldPrint = shouldPrintExpression;
			shouldPrintExpression = false;
			traverseGen(node->child[0], symtab);
			emitRM("ST", 3, toffset, 1, "Push left side");
			toffDec();
			traverseGen(node->child[1], symtab);
			toffInc();
			emitRM("LD", 4, toffset, 1, "Pop left into ac1");


			//Post op
			switch (node->attr.op) {
				//Some code in here might be useful for other ops
				case '+': {
					emitRO("ADD", 3, 4, 3, "Op +");
				} break;
				case '-':  {
					emitRO("SUB", 3,4,3,"Op -");
				} break;
				case '[': {
					emitRO("SUB", 3,4,3,"compute location from index");
					emitRM("LD", 3,0,3,"Load array element");
				} break;
			}
			shouldPrintExpression = oldShouldPrint;
		}
		break;
	}
}

void traverseGen(TreeNode* node, SymbolTable* symtab) {
	if (!node) return;

	if (node->codeGenDone)
		return;

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

	node->codeGenDone = true;

	traverseGen(node->child[0], symtab);
	traverseGen(node->child[1], symtab);
	traverseGen(node->child[2], symtab);
	traverseGen(node->sibling, symtab);
}

void doGlobalsAndStatics(TreeNode* node, bool isSiblingOfRoot = true);

void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}
	traverseGen(syntaxTree, symtab);
	//After the tree is done, write out the final segment
	backPatchAJumpToHere(0, "Jump to init [backpatch]");

	emitComment("INIT");
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