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


void traverseGen(TreeNode* node, SymbolTable* symtab, bool firstPass);

void caseDeclK(TreeNode* node, SymbolTable* symtab, bool firstPass) {
	if (firstPass) {
		switch (node->kind.decl) {
			case DeclKind::VarK: {
				node->offset = 1;
				if (node->varKind == VarKind::Global || node->isStatic) {
					if (node->isArray) {
						//Check for child, initializer
						traverseGen(node->child[0], symtab, firstPass);
						node->offset = goffset;
						goffset -= node->size;
					}
				}
			} break;
			case DeclKind::FuncK: {
				//If this function has parameters
				if (node->child[0]) {
					//Parms always have offset of one
					int count = 0;
					TreeNode* sibling = node->child[0];
					while (sibling) {
						count++;
						sibling = sibling->sibling;
					}
					node->offset = count;
				}
			} break;
		}
	} else {
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


				toffset -= node->offset;
				emitComment("TOFF set:", toffset);
				//Do some other stuff
				emitRM("ST", 3,-1,1, "Store return address");
				//traverseGen the funcs other child, either a compound or some other stmt
				traverseGen(node->child[1], symtab, firstPass);
				emitStandardClosing();
				emitComment("END FUNCTION", node->attr.name);
				toffset += node->offset;
				//goffset++;
				break;
			case DeclKind::VarK: {

			} break;
		}
	}
}
void caseStmtK(TreeNode* node, SymbolTable* symtab, bool firstPass) {
	if (firstPass) {
		switch (node->kind.stmt) {
			case StmtKind::CompoundK: {
				//If we have parameters
				if (node->child[0]) {
					traverseGen(node->child[0], symtab, firstPass);
					TreeNode* sibling = node->child[0];
					while (sibling) {
						node->offset++;
						sibling = sibling->sibling;
					}
				}
			} break;
		}
	} else {
		switch (node->kind.stmt) {
			case StmtKind::CompoundK:
			//Start a compound
			emitComment("COMPOUND");
			toffset -= node->offset;
			emitComment("TOFF set:", toffset);

			//Do its body
			emitComment("Compound Body");
			traverseGen(node->child[1], symtab, firstPass);

			toffset += node->offset;
			emitComment("TOFF set:", toffset);
			emitComment("END COMPOUND");
			break;
		}
	}
}

void caseExpK(TreeNode* node, SymbolTable* symtab, bool firstPass) {
	if (firstPass) {
		switch (node->kind.exp) {
			case ExpKind::CallK: {
				emitComment("CALL", node->attr.name);
			}
			break;
			case ExpKind::AssignK:
			break;
			case ExpKind::ConstantK: {
				switch(node->type) {
					case ExpType::String:
					node->offset = goffset;
					goffset -= strlen(node->attr.string);
					break;
					case ExpType::Integer: break;
					default:
					printf("CJERROR: Fell out of node->type in node->kind.exp\n");
					break;
				}
			}
			break;
		}
	} else {
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
			case ExpKind::OpK:
			switch (node->attr.op) {
				//Some code in here might be useful for other ops
				case '+': {
					bool oldShouldPrint = shouldPrintExpression;
					shouldPrintExpression = false;
					traverseGen(node->child[0], symtab, firstPass);
					emitRM("ST", 3, toffset, 1, "Push left side");
					toffDec();
					traverseGen(node->child[1], symtab, firstPass);
					toffInc();
					emitRM("LD", 4, toffset, 1, "Pop left into ac1");
					emitRO("ADD", 3, 4, 3, "Op +");
					shouldPrintExpression = oldShouldPrint;

				}
			}
		} 
	}
}

void traverseGen(TreeNode* node, SymbolTable* symtab, bool firstPass) {
	if (!node) return;

	if ((firstPass && node->codeGenFirstPass) || node->codeGenSecondPass)
		return;

	switch (node->nodekind) {
		//DECLARATION KIND
		case NodeKind::DeclK:
		caseDeclK(node, symtab, firstPass);
		break;
		//STATEMENT KIND
		case NodeKind::StmtK:
		caseStmtK(node, symtab, firstPass);
		break;
		//EXPRESSION KIND
		case NodeKind::ExpK:
		caseExpK(node, symtab, firstPass);
		break;
	}

	if (firstPass) {
		node->codeGenFirstPass = true;
	} else {
		node->codeGenSecondPass = true;
	}

	traverseGen(node->child[0], symtab, firstPass);
	traverseGen(node->child[1], symtab, firstPass);
	traverseGen(node->child[2], symtab, firstPass);
	traverseGen(node->sibling, symtab, firstPass);
}

void doGlobalsAndStatics(TreeNode* node, bool isSiblingOfRoot = true);

void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}
	traverseGen(syntaxTree, symtab, true);
	traverseGen(syntaxTree, symtab, false);
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