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
				if (syntaxTree->child[0] && syntaxTree->child[0]->type == ExpType::Void) {
					traverse(syntaxTree->child[0], symtab);
				}
				if (syntaxTree->child[1] && syntaxTree->child[1]->type == ExpType::Void) {
					traverse(syntaxTree->child[1], symtab);
				}
				switch (syntaxTree->attr.op) {
					case '=':
						//Make sure they are both of the same type, this code has copies, if you edit it please also edit copies
						if (syntaxTree->child[1]->type == syntaxTree->child[0]->type) {
							syntaxTree->type = syntaxTree->child[1]->type;
							syntaxTree->isArray = syntaxTree->child[1]->isArray;
						} else {
							printf("SEMANTIC ERROR(%d): '%s' requires operands of same type.\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
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
							printf("SEMANTIC ERROR(%d): '%s' requires operands of same type (also ints).\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
						}
					break;
					case INC:
					case DEC:
						if (syntaxTree->child[0]->type == ExpType::Integer) {
							syntaxTree->type = ExpType::Integer;
						} else {
							printf("SEMANTIC ERROR(%d): '%s' requires operands of type int.\n",
							syntaxTree->lineno, tokenToStr(syntaxTree->attr.op));
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
				syntaxTree->type = ((TreeNode*)symtab->lookup(id))->type;
				syntaxTree->isArray = ((TreeNode*)symtab->lookup(id))->isArray;
				syntaxTree->isStatic = ((TreeNode*)symtab->lookup(id))->isStatic;
				break;
			default:
			break;
		}
	}

	//DeclK is important for the symbol table
	if (syntaxTree->nodekind == NodeKind::DeclK) {
		if (syntaxTree->kind.decl == DeclKind::FuncK) {
			if (!symtab->insertGlobal(id, (void*)syntaxTree) && syntaxTree->lineno != -1) {
				//This is the part where an error needs to be thrown if there is a redefinition cjnote
				printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line UNKNOWN.\n", syntaxTree->lineno, id);
				exit(1);
			}
		} else {
			if (!symtab->insert(id, (void*)syntaxTree) && syntaxTree->lineno != -1) {
				//This is the part where an error needs to be thrown if there is a redefinition cjnote
				printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line UNKNOWN.\n", syntaxTree->lineno, id);
				exit(1);
			}
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

	if (shouldChangeScope(syntaxTree) && !isFuncSpecialCase) {
		symtab->leave();
	}
	traverse(syntaxTree->sibling, symtab);
}

TreeNode* semanticAnalysis(TreeNode* syntaxTree, bool shareCompoundSpace, bool noDuplicateUndefs, SymbolTable* symtab, int& globalOffset) {
	syntaxTree = loadIOLib(syntaxTree);
	traverse(syntaxTree, symtab);
	return syntaxTree;
}
