#include "semantics.h"
#include "treeUtils.h"
#include "parser.tab.h"
#include "yyerror.h"

int goffsetsem = 0;
int toffsetsem = 0;

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
	ioLib->functionAddress = 1;
	//Return the root node, save it here
	TreeNode* og = ioLib;
	ioLib->sibling = newIOFunc("output", ExpType::Void, ExpType::Integer);
	ioLib->sibling->functionAddress = 6;
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("inputb", ExpType::Boolean);
	ioLib->sibling->functionAddress = 12;
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outputb", ExpType::Void, ExpType::Boolean);
	ioLib->sibling->functionAddress = 17;
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("inputc", ExpType::Char);
	ioLib->sibling->functionAddress = 23;
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outputc", ExpType::Void, ExpType::Char);
	ioLib->sibling->functionAddress = 28;
	ioLib = ioLib->sibling;
	ioLib->sibling = newIOFunc("outnl", ExpType::Void);
	ioLib->sibling->functionAddress = 34;
	ioLib = ioLib->sibling;
	//Append the rest of the syntax tree
	ioLib->sibling = syntaxTree;
	return og;
}

void warningsEmitter(std::string, void* voidNode) {
	TreeNode* node = (TreeNode*)voidNode;
	if (!node->isUsed && strcmp(node->attr.name, "main") != 0 && node->lineno >= 0) {
		emitUnusedVariableWarning(node->lineno, node->attr.name, node->varKind == VarKind::Parameter);
	} else {
		if (!node->isAssigned && node->nodekind == NodeKind::DeclK && node->kind.decl == DeclKind::VarK) {
			emitUninitializedVariableWarning(node->lineno, node->attr.name);
		}
	}
}

bool shouldChangeScope(TreeNode* node) {
	//Statements oftentimes create new scopes
	if (node->nodekind == NodeKind::StmtK) {
		switch (node->kind.stmt) {
			case StmtKind::BreakK:
			case StmtKind::ReturnK:
			case StmtKind::RangeK:
			case StmtKind::WhileK:
			case StmtKind::IfK:
			break;
			case StmtKind::ForK:
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
	if (syntaxTree->semanticsDone)
		return;

	//Used pretty universally, typicall 'the' id in question
	char* id = strdup(syntaxTree->attr.name);

	if (shouldChangeScope(syntaxTree) && !isFuncSpecialCase) {
		symtab->enter("NewScope from " + (std::string)id);
	}

	//Expression kind is the important one for the USAGE semantic analysis
	if (syntaxTree->nodekind == NodeKind::ExpK) {
		switch (syntaxTree->kind.exp) {
			case ExpKind::AssignK:
				//Make sure both children have been traversed first
				if (syntaxTree->child[0]) {
					traverse(syntaxTree->child[0], symtab);
				}
				if (syntaxTree->child[1]) {
					traverse(syntaxTree->child[1], symtab);
				}
				switch (syntaxTree->attr.op) {
					case '=':
						if (syntaxTree->child[0]->nodekind != NodeKind::ExpK) {
							((TreeNode*)symtab->lookup(syntaxTree->child[0]->child[0]->attr.name))->isAssigned = true;
						}
						//Make sure they are both of the same type, this code has copies, if you edit it please also edit copies
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type) {
							syntaxTree->type = syntaxTree->child[1]->type;
							syntaxTree->isArray = syntaxTree->child[1]->isArray;
						} else {
							emitEqualsDifferingTypesError(syntaxTree->lineno, syntaxTree->child[0]->type, syntaxTree->child[1]->type);
						}
						break;
					case ADDASS:
					case SUBASS:
					case MULASS:
					case DIVASS:
						//Make sure they are both of the same type, this code has copies, if you edit it please also edit copies
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type && syntaxTree->child[0]->type == ExpType::Integer) {
							syntaxTree->type = syntaxTree->child[1]->type;
						} else {
						}
					break;
					case INC:
					case DEC:
						if (syntaxTree->child[0]->type == ExpType::Integer) {
							syntaxTree->type = ExpType::Integer;
						} else {
						}
					break;
				}
				break;
			case ExpKind::OpK:
				//Make sure both children have been traversed first, copied from above
				if (syntaxTree->child[0] && syntaxTree->child[0]->type == ExpType::Void) {
					traverse(syntaxTree->child[0], symtab);
				}
				if (syntaxTree->child[1] && syntaxTree->child[1]->type == ExpType::Void) {
					traverse(syntaxTree->child[1], symtab);
				}
				switch (syntaxTree->attr.op) {
					//NON-UNARY LOGIC BLOCK
					case EQ:
					case GEQ:
					case LEQ:
					case NEQ:
					case '<':
					case '>':
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type) {
							syntaxTree->type = ExpType::Boolean;
						} else {
							printf("SEMANTIC ERROR(%d): '%s' requires operands of same type.\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
						}
					break;
					//END BLOCK

					//ARRAY INDEX BLOCK
					case '[':
						if (syntaxTree->child[0]->isArray) {
							syntaxTree->type = syntaxTree->child[0]->type;
						} else {
							printf("CJERROR(%d): trying to index something that isn't an array\n", syntaxTree->lineno);
						}
					break;
					//END BLOCK

					//UNARY/NON-UNARY ARITHMETIC BLOCK
					case '*':
						//If this is a unary operator
						if (!syntaxTree->child[1]) {
							if (syntaxTree->child[0]->isArray) {
								syntaxTree->type = ExpType::Integer;
							}
							break;
						} else {
							//otherwise do the same code as the next stuff
							//so left blank
						}
					case '-':
					case '?':
						//If unary and also not passed through from the *
						if (!syntaxTree->child[1] && syntaxTree->attr.op == '-' || syntaxTree->attr.op == '?') {
							if (syntaxTree->child[0]->type == ExpType::Integer) {
								syntaxTree->type = ExpType::Integer;
							}
							break;
						} else {
							//otherwise do the same code as the next stuff
							//so left blank
						}
					case '+':
					case '%':
					case '/':
					case MIN:
					case MAX:
						//Make sure they are both of the same type, copied
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type && syntaxTree->child[0]->type == ExpType::Integer) {
							syntaxTree->type = syntaxTree->child[1]->type;
						} else {
							printf("SEMANTIC ERROR(%d): '%s' requires operands of same type (also ints).\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
						}
					break;
					//END BLOCK

					//UNARY BOOLEAN ONLY LOGIC BLOCK
					case NOT:
						if (syntaxTree->child[0]->type == ExpType::Boolean) {
							syntaxTree->type = ExpType::Boolean;
						}
					break;
					//END BLOCK

					//NON-UNARY BOOLEAN ONLY LOGIC BLOCK
					case AND:
					case OR:
						//Make sure they are both of the same type, copied
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type && syntaxTree->child[0]->type == ExpType::Boolean) {
							syntaxTree->type = syntaxTree->child[1]->type;
						} else {
							printf("SEMANTIC ERROR(%d): '%s' requires operands of same type (also bools).\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
						}
					break;
					//END BLOCK
				}
				break;
			case ExpKind::IdK:
			case ExpKind::CallK:
				if (!symtab->lookup(id)) {
					emitUndeclaredVariableError(syntaxTree->lineno, id);
				} else {
					syntaxTree->type = ((TreeNode*)symtab->lookup(id))->type;
					syntaxTree->isArray = ((TreeNode*)symtab->lookup(id))->isArray;
					syntaxTree->isStatic = ((TreeNode*)symtab->lookup(id))->isStatic;
					syntaxTree->offset = ((TreeNode*)symtab->lookup(id))->offset;
					syntaxTree->size = ((TreeNode*)symtab->lookup(id))->size;
					syntaxTree->varKind = ((TreeNode*)symtab->lookup(id))->varKind;
					((TreeNode*)symtab->lookup(id))->isUsed = true;
				}
				break;
			case ExpKind::ConstantK:
				switch(syntaxTree->type) {
					case ExpType::String:
					syntaxTree->offset = goffsetsem;
					goffsetsem -= strlen(syntaxTree->attr.string) - 1;
					break;
					case ExpType::Boolean:
					case ExpType::Char:
					case ExpType::Integer:
					break;
					default:
					printf("CJERROR: Fell out of node->type in node->kind.exp\n");
					break;
				}
			default:
			break;
		}
	}

	//DeclK is important for insertion to the symbol table
	if (syntaxTree->nodekind == NodeKind::DeclK) {
		if (syntaxTree->kind.decl == DeclKind::FuncK) {
			toffsetsem = -2;
			//Not sure why this needed to use insert global specifically
			//Actually wait it might be because functions make a new scope and are always global
			if (!symtab->insertGlobal(id, (void*)syntaxTree) && syntaxTree->lineno != -1) {
				//This is the part where an error needs to be thrown if there is a redefinition cjnote
				printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line UNKNOWN.\n", syntaxTree->lineno, id);
				exit(1);
			}

			//If we have params
			if (syntaxTree->child[0]) {
				syntaxTree->offset = 0;
				TreeNode* sibling = syntaxTree->child[0];
				while (sibling) {
					//Params always have size of 1, so offset increases by one each time
					syntaxTree->offset--;
					//Add it to the symbol table
					symtab->insert(sibling->attr.name, sibling);
					//cjnote: not sure about the offsets here
					sibling->offset = syntaxTree->offset - 1;
					sibling->semanticsDone = true;
					sibling = sibling->sibling;
					toffsetsem--;
				}
			}
		} else { //If we are a variable, parms don't get here (VarK)
				if (syntaxTree->child[0]) {
					syntaxTree->isAssigned = true;
				}
				if (syntaxTree->varKind == VarKind::Global || syntaxTree->isStatic) {
					traverse(syntaxTree->child[0], symtab);
					syntaxTree->offset = goffsetsem;
					if (syntaxTree->isArray) {
						goffsetsem -= syntaxTree->size;
					}
					goffsetsem--;
				} else {
					syntaxTree->offset = toffsetsem;
					if (syntaxTree->isArray) {
						toffsetsem -= syntaxTree->size;
					}
					toffsetsem--;
				}
			if (!symtab->insert(id, (void*)syntaxTree) && syntaxTree->lineno != -1) {
				//This is the part where an error needs to be thrown if there is a redefinition cjnote
				printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line UNKNOWN.\n", syntaxTree->lineno, id);
				exit(1);
			}
		}
	}
	
	if (syntaxTree->nodekind == NodeKind::StmtK) {
		switch (syntaxTree->kind.stmt) {
			case StmtKind::CompoundK: {
				//If we have variables
				if (syntaxTree->child[0]) {
					int oldtoffset = toffsetsem;
					TreeNode* sibling = syntaxTree->child[0];
					while (sibling) {
						traverse(sibling, symtab);
						syntaxTree->offset = toffsetsem;

						sibling = sibling->sibling;
					}
					traverse(syntaxTree->child[1],symtab);
					toffsetsem = oldtoffset;
				} else {
					syntaxTree->offset = toffsetsem;
				}
			} break;

			case StmtKind::ForK: {
				syntaxTree->child[0]->offset = toffsetsem;
				toffsetsem-=3;
				syntaxTree->offset = toffsetsem;
				bool res = symtab->insert(syntaxTree->child[0]->attr.name, (void*)syntaxTree->child[0]);
				syntaxTree->semanticsDone = true;
				syntaxTree->child[0]->semanticsDone = true;
			} break;

			case StmtKind::WhileK: {
				syntaxTree->offset -= 3;
			} break;

			case StmtKind::ReturnK: {
				traverse(syntaxTree->child[0], symtab);
				if (syntaxTree->child[0]->isArray) {
					emitCannotReturnArrayError(syntaxTree->lineno);
				}
			} break;

			case StmtKind::IfK: {
				traverse(syntaxTree->child[0], symtab);
				if (syntaxTree->child[0]->type != ExpType::Boolean) {
					emitBooleanInIfError(syntaxTree->lineno, syntaxTree->child[0]->type);
				}
			}

		}
	}

	syntaxTree->semanticsDone = true;

	traverse(syntaxTree->child[0], symtab);
	traverse(syntaxTree->child[1], symtab, //Handle the function compound special case
		syntaxTree->child[1] &&
		syntaxTree->nodekind == NodeKind::DeclK &&
		syntaxTree->kind.decl == DeclKind::FuncK &&
		syntaxTree->child[1]->nodekind == NodeKind::StmtK &&
		syntaxTree->child[1]->kind.stmt == StmtKind::CompoundK
	);
	traverse(syntaxTree->child[2], symtab);

	if (shouldChangeScope(syntaxTree) && !isFuncSpecialCase) {
		symtab->applyToAll(warningsEmitter);
		symtab->leave();
	}
	traverse(syntaxTree->sibling, symtab);
}



TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int& globalOffset) {
	syntaxTree = loadIOLib(syntaxTree);
	traverse(syntaxTree, symtab);
	symtab->goffset = goffsetsem;


	//Do warnings/errors
	symtab->applyToAllGlobal(warningsEmitter);

	return syntaxTree;
}
