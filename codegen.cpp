#include "codegen.h"
#include "emitcode.h"
#include "parser.tab.h"

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
			node->functionAddress = emitWhereAmI();
			emitRM("ST", 3,-1,1, "Store return address");
			//traverseGen the funcs other child, either a compound or some other stmt
			traverseGen(node->child[1], symtab);
			emitStandardClosing();
			emitComment("END FUNCTION", node->attr.name);
			toffset -= node->offset;
			//goffset++;
			break;
		case DeclKind::VarK: {
			if (node->isArray) {
				emitRM("LDC", 3, node->size, 6, "load size of array", node->attr.name);
				emitRM("ST", 3, node->offset, 1, "save size of array", node->attr.name);
			}
		} break;
	}
}
void caseStmtK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.stmt) {
		case StmtKind::CompoundK:
		//Start a compound
		emitComment("COMPOUND");
		toffset += node->offset;
		emitComment("TOFF set:", toffset);

		//Do var decls
		TreeNode* sibling = node->child[0];
		while (sibling) {
			traverseGen(sibling, symtab);
			sibling = sibling->sibling;
		}

		//Do its body
		emitComment("Compound Body");

		sibling = node->child[1];
		while (sibling) {
			traverseGen(sibling, symtab);
			sibling = sibling->sibling;
		}

		toffset -= node->offset;
		emitComment("TOFF set:", toffset);
		emitComment("END COMPOUND");
		break;
	}
}

void caseExpK(TreeNode* node, SymbolTable* symtab) {
	//Code generation

	bool oldShouldPrint = shouldPrintExpression;
	if (shouldPrintExpression) {
		emitComment("EXPRESSION");
		shouldPrintExpression = false;
	}
	switch (node->kind.exp) {
		case ExpKind::CallK: {
			int oldToffset = toffset;
			emitComment("CALL", node->attr.name);
			emitRM("ST", 1, toffset, 1, "Store fp in ghost frame for", node->attr.name);
			//cjnote: not sure why we do 2 here
			toffDec();
			toffDec();
			int parmCount = 1;
			TreeNode* parm = node->child[0];
			while (parm) {
				emitComment("Param", parmCount);
				parmCount++;
				traverseGen(parm, symtab);
				emitRM("ST", 3, toffset, 1, "Push parameter");
				toffDec();
				parm = parm->sibling;
			}

			emitComment("Param end", node->attr.name);
			emitRM("LDA", 1, oldToffset, 1, "Ghost frame becomes new active frame");
			emitRM("LDA", 3,1,7,"Return address in ac");
			emitRM("JMP", 7, -(emitWhereAmI() - ((TreeNode*)symtab->lookupGlobal(node->attr.name))->functionAddress+1), 7, "CALL", node->attr.name);
			emitRM("LDA", 3,0,2,"Save the result in ac");


			emitComment("Call end", node->attr.name);
			toffset = oldToffset;
			emitComment("TOFF set:", toffset);
		}
		break;
		//Some repeated code in this case, could be cleaned up but I don't care
		case ExpKind::AssignK: {
			char* varname = node->child[0]->attr.name;
			if (node->child[0]->attr.op == '[') {
				TreeNode* opChild = node->child[0];;
				TreeNode* rightChild = node->child[1];
				TreeNode* idChild = opChild->child[0];
				TreeNode* indexChild = opChild->child[1];
				opChild->codeGenDone = true;
				idChild->codeGenDone = true;

				traverseGen(indexChild, symtab);

				if (node->attr.op == INC || node->attr.op == DEC) {
					emitRM("LDA", 5, idChild->offset-1, 1, "Load address of base of array", idChild->attr.name);
					emitRO("SUB",5,5,3,"Compute offset of value");
					emitRM("LD", 3, node->child[0]->offset, 5, "load lhs variable", idChild->attr.name);
					if (node->attr.op == INC) {
						emitRM("LDA", 3, 1, 3, "increment value of", idChild->attr.name);
					} else {
						emitRM("LDA", 3, -1, 3, "decrement value of", idChild->attr.name);
					}
				} else { //Not INC or DEC
					emitRM("ST", 3, toffset, 1, "Push index");
					toffDec();
					traverseGen(rightChild, symtab);
					toffInc();
					emitRM("LD", 4, toffset, 1, "Pop index");
					emitRM("LDA", 5, idChild->offset-1, 1, "Load address of base of array", idChild->attr.name);
					emitRO("SUB",5,5,4,"Compute offset of value");

					if (node->attr.op == ADDASS || node->attr.op == SUBASS ||
						node->attr.op == MULASS || node->attr.op == DIVASS) {
						//The assign+op ops load into 4 instead of 3, only change between the two lines
						emitRM("LD", 4,0,5,"load lhs variable", idChild->attr.name);
						//Copied from somewhere below
						switch (node->attr.op) {
							case ADDASS:
							emitRO("ADD",3,4,3,"op +=");
							break;
							case SUBASS:
							emitRO("SUB",3,4,3,"op -=");
							break;
							case MULASS:
							emitRO("MUL",3,4,3,"op *=");
							break;
							case DIVASS:
							emitRO("DIV",3,4,3,"op /=");
							break;
							case INC:
							emitRM("LDA", 3, 1, 3, "increment value of", varname);
							break;
							case DEC:
							emitRM("LDA", 3, -1, 3, "decrement value of", varname);
						}
					} else {
						emitRM("LD", 3, node->child[0]->offset, 1, "load lhs variable", varname);
					}
				}

				emitRM("ST",3,0,5,"Store variable", idChild->attr.name);
			} else {
				traverseGen(node->child[1], symtab);
				if (node->attr.op != '=') {
					if (node->attr.op == ADDASS || node->attr.op == SUBASS ||
						node->attr.op == MULASS || node->attr.op == DIVASS) {
						//The assign+op ops load into 4 instead of 3, only change between the two lines
						emitRM("LD", 4, node->child[0]->offset, 1, "load lhs variable", varname);
					} else {
						emitRM("LD", 3, node->child[0]->offset, 1, "load lhs variable", varname);
					}
				}
				//Copied by something above
				switch (node->attr.op) {
					case ADDASS:
					emitRO("ADD",3,4,3,"op +=");
					break;
					case SUBASS:
					emitRO("SUB",3,4,3,"op -=");
					break;
					case MULASS:
					emitRO("MUL",3,4,3,"op *=");
					break;
					case DIVASS:
					emitRO("DIV",3,4,3,"op /=");
					break;
					case INC:
					emitRM("LDA", 3, 1, 3, "increment value of", varname);
					break;
					case DEC:
					emitRM("LDA", 3, -1, 3, "decrement value of", varname);
				}
				emitRM("ST", 3, node->child[0]->offset, 1, "Store variable", varname);
				node->child[0]->codeGenDone = true;
			}
		}
		break;
		case ExpKind::ConstantK: {
			switch (node->type) {
				case ExpType::String:
				emitStrLit(node->offset, node->attr.string);
				break;
				case ExpType::Integer:
				emitRM("LDC", 3, node->attr.value, 6, "Load integer constant");
				break;
				case ExpType::Char:
				emitRM("LDC", 3, (int)node->attr.cvalue, 6, "Load char constant");
				break;
				default:
				printf("CJERROR: Fell into default case in ExpKind::ConstantK node->type switch\n");
				break;
			}
		} break;
		case ExpKind::IdK: {
			if (node->isArray) {
				emitRM("LDA", 3, node->offset-1, 1,
				"Load address of base of array", node->attr.name);
			} else {
				emitRM("LD", 3, node->offset, 1, "Load variable", node->attr.name);
			}
		} break;\
		case ExpKind::OpK: {
			
			//Pre-op
			switch (node->attr.op) {
				case '[': {
				}
			}

			//Common code (for ops with 2 args)

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
				case '/': {
					emitRO("DIV", 3,4,3,"Op /");
				} break;
				case '*': {
					emitRO("MUL", 3,4,3,"Op *");
				} break;
				case '[': {
					emitRO("SUB", 3,4,3,"compute location from index");
					emitRM("LD", 3,0,3,"Load array element");
				} break;
			}
		}
		break;
	}
	shouldPrintExpression = oldShouldPrint;

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
	//traverseGen(node->sibling, symtab);
}

void doGlobalsAndStatics(TreeNode* node, bool isSiblingOfRoot = true);

void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}

	while (syntaxTree) {
		traverseGen(syntaxTree, symtab);
		syntaxTree = syntaxTree->sibling;
	}
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