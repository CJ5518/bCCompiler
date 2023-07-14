#include "codegen.h"
#include "emitcode.h"
#include "parser.tab.h"

void outputHeader(char* srcFile);


//Not sure about this one
int foffset = -2;
int toffset = -2;
int goffset = 0;
//Where do we break to?
std::vector<int> breakPosition;
std::multimap<std::string, TreeNode*> staticsAndGlobals;

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


void traverseGen(TreeNode* node, SymbolTable* symtab, bool doStaticsAndGlobals=false, bool printArraySize=false);

void loadIdK(TreeNode* node, SymbolTable* symtab, int intoReg=3) {
	int isntGlobal = !(node->varKind == VarKind::Global || node->isStatic);
	if (node->isArray) {
		if (node->varKind == VarKind::Parameter) {
			emitRM("LD", intoReg, node->offset, isntGlobal, "Load address of base of array", node->attr.name);
		} else {
			emitRM("LDA", intoReg, node->offset-1, isntGlobal,
				"Load address of base of array", node->attr.name);
		}
	} else {
		emitRM("LD", intoReg, node->offset, isntGlobal, "Load variable", node->attr.name);
	}
	node->codeGenDone = true;
}

void caseDeclK(TreeNode* node, SymbolTable* symtab, bool loadArraySize=false) {
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
				if (node->varKind != VarKind::Global || loadArraySize) {
					emitRM("LDC", 3, node->size, 6, "load size of array", node->attr.name);
					emitRM("ST", 3, node->offset, !(node->varKind == VarKind::Global || node->isStatic), "save size of array", node->attr.name);
				}
				//If we have a string constant attached
				if (node->child[0]) {
					shouldPrintExpression = false;
					traverseGen(node->child[0], symtab);

					if (!loadArraySize) {
						emitRM("LDA", 4, node->offset - 1, !(node->varKind == VarKind::Global || node->isStatic), "address of lhs");
						emitRM("LD", 5, 1, 3, "size of rhs");
						emitRM("LD", 6, 1, 4, "size of lhs");
						emitRO("SWP", 5,6,6,"pick smallest size");
						emitRO("MOV",4,3,5,"array op =");
					} else {
						emitRM("ST", 3, node->offset - 1,0,"Store variable", node->attr.name);
					}

					shouldPrintExpression = true;
				}
			} else {
				//If this guy has an initializer
				if (node->child[0]) {
					shouldPrintExpression = false;
					traverseGen(node->child[0], symtab);
					emitRM("ST", 3, node->offset, !(node->varKind == VarKind::Global || node->isStatic), "Store variable", node->attr.name);
					shouldPrintExpression = true;
				}
			}
		} break;
	}
}
void caseStmtK(TreeNode* node, SymbolTable* symtab) {
	switch (node->kind.stmt) {
		case StmtKind::ReturnK: {
			emitComment("RETURN");
			shouldPrintExpression = false;
			traverseGen(node->child[0], symtab);
			if (node->child[0]) {
				emitRM("LDA", 2,0,3, "Copy result to return register");
			}
			emitRM("LD", 3,-1,1,"Load return address");
			emitRM("LD",1,0,1,"Adjust fp");
			emitRM("JMP",7,0,3,"Return");
			shouldPrintExpression = true;
		} break;
		case StmtKind::CompoundK: {
			//Start a compound
			emitComment("COMPOUND");
			int oldtoffset = toffset;
			toffset = node->offset;
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

			toffset = oldtoffset;
			emitComment("TOFF set:", toffset);
			emitComment("END COMPOUND");
		} break;

		case StmtKind::IfK: {
			emitComment("IF");
			shouldPrintExpression = false;
			traverseGen(node->child[0], symtab);
			shouldPrintExpression = true;
			emitComment("THEN");
			int location = emitWhereAmI();
			emitSkip(1);
			traverseGen(node->child[1], symtab);
			//If we have an else statement
			if (node->child[2]) {
				emitSkip(1);
			}
			backPatchAJumpToHere("JZR", 3, location, "Jump around the THEN if false [backpatch]");
			if (node->child[2]) {
				location = emitWhereAmI();
				emitComment("ELSE");
				traverseGen(node->child[2], symtab);
				backPatchAJumpToHere("JMP", 7, location-1, "Jump around the ELSE [backpatch]");
			}
			emitComment("END IF");
		} break;

		case StmtKind::ForK: {
			shouldPrintExpression = false;
			toffset = node->offset;
			emitComment("TOFF set:", toffset);
			int oldtoffset = toffset;
			emitComment("FOR");
			TreeNode* rangeNode = node->child[1];

			traverseGen(rangeNode->child[0], symtab);
			emitRM("ST", 3,node->child[0]->offset,1,"save starting value in index variable");

			traverseGen(rangeNode->child[1], symtab);
			emitRM("ST", 3,node->child[0]->offset-1,1,"save stop value");

			//Has the third value
			if (rangeNode->child[2]) {
				traverseGen(rangeNode->child[2], symtab);
			} else {
				//Use default
				emitRM("LDC", 3,1,6, "default increment by 1");
			}
			emitRM("ST", 3,node->child[0]->offset-2,1,"save step value");

			//Standard stuff
			int goBackLocation = emitWhereAmI();
			emitRM("LD",4,node->child[0]->offset,1,"loop index");
			emitRM("LD",5,node->child[0]->offset-1,1,"stop value");
			emitRM("LD",3,node->child[0]->offset-2,1,"step value");
			emitRO("SLT", 3,4,5, "Op <");
			emitRM("JNZ",3,1,7,"Jump to loop body");
			breakPosition.push_back(emitWhereAmI());

			//Skip one and do the compound
			emitSkip(1);
			int otherJmpLocation = emitWhereAmI();
			shouldPrintExpression = true;
			traverseGen(node->child[2], symtab);
			
			emitComment("Bottom of loop increment and jump");
			emitRM("LD", 3, node->child[0]->offset, 1, "Load index");
			emitRM("LD", 5, node->child[0]->offset - 2, 1, "Load step");
			emitRO("ADD", 3,3,5,"increment");
			emitRM("ST", 3,node->child[0]->offset,1,"store back to index");
			emitRM("JMP", 7, goBackLocation - emitWhereAmI()-1, 7,"go to beginning of loop");
			backPatchAJumpToHere("JMP", 7, otherJmpLocation-1, "Jump past loop [backpatch]");

			emitComment("END LOOP");
			breakPosition.pop_back();
			
			toffset = oldtoffset;
		} break;
		

		case StmtKind::BreakK: {
			emitComment("BREAK");
			emitRM("JMP", 7, breakPosition.back() - emitWhereAmI() - 1, 7, "break");
		} break;

		case StmtKind::WhileK: {
			int oldtoffset = toffset;
			emitComment("WHILE");

			shouldPrintExpression = false;
			int goBackLocation = emitWhereAmI();
			traverseGen(node->child[0], symtab);
			shouldPrintExpression = true;
			emitRM("JNZ", 3, 1, 7, "Jump to while part");
			breakPosition.push_back(emitWhereAmI());
			int otherJmpLocation = emitWhereAmI();
			

			emitComment("DO");
			emitSkip(1);
			traverseGen(node->child[1], symtab);

			
			emitRM("JMP", 7, goBackLocation - emitWhereAmI() - 1, 7,"go to beginning of loop");
			backPatchAJumpToHere("JMP", 7, otherJmpLocation, "Jump past loop [backpatch]");

			breakPosition.pop_back();
			toffset = oldtoffset;
			emitComment("END WHILE");
		} break;
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

			//If we're setting an element in an array to something
			if (node->child[0]->attr.op == '[') {
				TreeNode* opChild = node->child[0];;
				TreeNode* rightChild = node->child[1];
				TreeNode* idChild = opChild->child[0];
				TreeNode* indexChild = opChild->child[1];
				opChild->codeGenDone = true;

				traverseGen(indexChild, symtab);

				if (node->attr.op == INC || node->attr.op == DEC) {
					loadIdK(idChild, symtab, 5);
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
					loadIdK(idChild, symtab, 5);
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
						//emitRM("LD", 3, node->child[0]->offset, 1, "load lhs variable", varname);
					}
				}

				emitRM("ST",3,0,5,"Store variable", idChild->attr.name);
				//If we're setting an array to an array
			} else if (node->child[1] && node->child[1]->isArray) {
				TreeNode* right = node->child[1];
				TreeNode* left = node->child[0];
				loadIdK(right, symtab, 3);
				emitRM("LDA", 4, left->offset-1,1, "address of lhs");
				emitRM("LD", 5, 1, 3, "size of rhs");
				emitRM("LD", 6, 1, 4, "size of lhs");
				emitRO("SWP", 5,6,6,"pick smallest size");
				emitRO("MOV", 4,3,5,"array op =");
				left->codeGenDone = true;
				right->codeGenDone = true;
			} else { //otherwise, regular vars all around
				traverseGen(node->child[1], symtab);
				if (!node->child[1]) {
					
				}
				if (node->attr.op != '=') {
					if (node->attr.op == ADDASS || node->attr.op == SUBASS ||
						node->attr.op == MULASS || node->attr.op == DIVASS) {
						//The assign+op ops load into 4 instead of 3, only change between the two lines
						emitRM("LD", 4, node->child[0]->offset, !(node->child[0]->varKind == VarKind::Global || node->child[0]->isStatic), "load lhs variable", varname);
					} else {
						emitRM("LD", 3, node->child[0]->offset, !(node->child[0]->varKind == VarKind::Global || node->child[0]->isStatic), "load lhs variable", varname);
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
				emitRM("ST", 3, node->child[0]->offset, !(node->child[0]->varKind == VarKind::Global || node->child[0]->isStatic), "Store variable", varname);
				node->child[0]->codeGenDone = true;
			}
		}
		break;
		case ExpKind::ConstantK: {
			switch (node->type) {
				case ExpType::String:
				emitStrLit(node->offset - 1, node->attr.string);
				emitRM("LDA", 3, node->offset - 1, 0, "Load address of char array");
				break;
				case ExpType::Integer:
				emitRM("LDC", 3, node->attr.value, 6, "Load integer constant");
				break;
				case ExpType::Char:
				emitRM("LDC", 3, (int)node->attr.cvalue, 6, "Load char constant");
				break;
				case ExpType::Boolean:
				emitRM("LDC", 3, node->attr.value, 6, "Load Boolean constant");
				break;
				default:
				printf("CJERROR: Fell into default case in ExpKind::ConstantK node->type switch\n");
				break;
			}
		} break;
		case ExpKind::IdK: {
			loadIdK(node, symtab);
		} break;
		case ExpKind::OpK: {
			
			//Pre-op
			switch (node->attr.op) {
				case '[': {
					loadIdK(node->child[0], symtab);
				}
			}

			traverseGen(node->child[0], symtab);
			//If it's a unary op
			if (!node->child[1]) {
				switch (node->attr.op) {
					case '-':
					emitRO("NEG",3,3,3,"Op unary -");
					break;
					case '*':
					emitRM("LD",3,1,3,"Load array size");
					break;
					case '?': {
						//If we are doing this on an array
						if (node->child[0]->isArray) {
							//We basically just emit the same code for this:
							//array[?*array];
							emitRM("ST", 3, toffset, 1, "Push left side");
							toffDec();
							node->child[0]->codeGenDone = false;
							traverseGen(node->child[0], symtab);
							emitRM("LD",3,1,3,"Load array size");
							emitRO("RND", 3,3,6,"Op ?");
							toffInc();
							emitRM("LD", 4, toffset, 1, "Pop left into ac1");
							emitRO("SUB", 3,4,3,"compute location from index");
							emitRM("LD", 3,0,3,"Load array element");
						} else {
							emitRO("RND", 3,3,6,"Op ?");
						}
					}
					break;
					case NOT:
					emitRM("LDC", 4,1,6, "Load 1");
					emitRO("XOR",3,3,4,"Op XOR to get logical not");
					break;
					default:
					printf("CJERROR: Fell out of a unary op switch in codegen\n");
					break;
				}
			} else {
				//Common code (for ops with 2 args)
				emitRM("ST", 3, toffset, 1, "Push left side");
				toffDec();
				traverseGen(node->child[1], symtab);
				toffInc();
				emitRM("LD", 4, toffset, 1, "Pop left into ac1");

				if (node->child[1]->isArray && node->child[0]->isArray) {
					//Double array ops, significantly more involved
					emitRM("LD",5,1,3,"AC2 <- |RHS|");
					emitRM("LD",6,1,4,"AC3 <- |LHS|");
					emitRM("LDA",2,0,5,"R2 <- |RHS|");
					emitRO("SWP",5,6,6,"pick smallest size");
					emitRM("LD",6,1,4,"AC3 <- |LHS|");
					emitRO("CO",4,3,5,"setup array compare  LHS vs RHS");
					emitRO("TNE",5,4,3,"if not equal then test (AC1, AC)");
					emitRO("JNZ",5,2,7,"jump not equal");
					emitRM("LDA",3,0,2,"AC1 <- |RHS|");
					emitRM("LDA",4,0,6,"AC <- |LHS|");
				}
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
					case '%': {
						emitRO("MOD", 3,4,3,"Op %");
					} break;
					case EQ: {
						emitRO("TEQ",3,4,3,"Op ==");
					} break;
					case LEQ: {
						emitRO("TLE",3,4,3,"Op <=");
					} break;
					case GEQ: {
						emitRO("TGE",3,4,3,"Op >=");
					} break;
					case NEQ: {
						emitRO("TNE",3,4,3,"Op !=");
					} break;
					case '<': {
						emitRO("TLT",3,4,3,"Op <");
					} break;
					case '>': {
						emitRO("TGT",3,4,3,"Op >");
					} break;
					case AND: {
						emitRO("AND",3,4,3,"Op AND");
					} break;
					case OR: {
						emitRO("OR",3,4,3,"Op OR");
					} break;
					case '[': {
						emitRO("SUB", 3,4,3,"compute location from index");
						emitRM("LD", 3,0,3,"Load array element");
					} break;
					case MIN:
					//The second 3 is useless, just leaving it like this because convention
					//reg[3] = min(reg[3], reg[4])
					emitRO("SWP", 3,4,3,"Op :<:");
					break;
					case MAX:
					//reg[3] = max(reg[4], reg[3])
					emitRO("SWP", 4,3,3,"Op :>:");
					break;
				}
			}
		}
		break;
	}
	shouldPrintExpression = oldShouldPrint;

}

void traverseGen(TreeNode* node, SymbolTable* symtab, bool doStaticsAndGlobals, bool printArraySize) {
	if (!node) return;

	if (node->codeGenDone)
		return;
	
	//Statics and globals
	if ((node->isStatic || node->varKind == VarKind::Global) && node->nodekind == NodeKind::DeclK && node->kind.decl == DeclKind::VarK) {
		staticsAndGlobals.insert(std::pair<std::string, TreeNode*>(node->attr.name, node));
		if (!doStaticsAndGlobals)
			return;
	}

	switch (node->nodekind) {
		//DECLARATION KIND
		case NodeKind::DeclK:
		caseDeclK(node, symtab, printArraySize);
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


void codegen(FILE* codeOut, char* srcFile, TreeNode* syntaxTree, SymbolTable* symtab, int globalOffset, bool linenumFlagIn) {
	goffset = symtab->goffset;
	outputHeader(srcFile);
	//Skip past the IO nodes
	for (int q = 0; q < 7; q++) {
		syntaxTree = syntaxTree->sibling;
	}

	while (syntaxTree) {
		if (syntaxTree->kind.decl == DeclKind::VarK) {
			if (syntaxTree->varKind != VarKind::Global) {
				printf("CJERROR: This thingy here on line 550 currently (subject to change) is not as it should be\n");
			} else {
				//Global var decl, handle the char array exception
				if (syntaxTree->child[0] && syntaxTree->type == ExpType::Char && syntaxTree->isArray) {
					traverseGen(syntaxTree, symtab, true, false);
					syntaxTree->codeGenDone = false;
					syntaxTree->child[0]->codeGenDone = false;
				}
			}
		}
		traverseGen(syntaxTree, symtab);
		syntaxTree = syntaxTree->sibling;
	}
	//After the tree is done, write out the final segment
	backPatchAJumpToHere(0, "Jump to init [backpatch]");

	emitComment("INIT");
	emitRM("LDA", 1, goffset, 0, "set first frame at end of globals");
	emitRM("ST", 1,0,1, "store old fp (point to self)");
	emitComment("INIT GLOBALS AND STATICS");
	for (auto itr = staticsAndGlobals.begin(); itr != staticsAndGlobals.end(); itr++) {
		TreeNode* node = itr->second;
		traverseGen(node, symtab, true, true);
	}
	emitComment("END INIT GLOBALS AND STATICS");
	emitRM("LDA",3,1,7,"Return address in ac");
	emitRM("JMP", 7, mainIndex - emitWhereAmI() - 1,7,"Jump to main");
	emitRO("HALT",0,0,0,"DONE!");
	emitComment("END INIT");
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