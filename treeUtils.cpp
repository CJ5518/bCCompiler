#include <treeUtils.h>
#include <stdlib.h>
#include <string>


TreeNode* allocNode() {
	return (TreeNode*)malloc(sizeof(TreeNode));
}

TreeNode* cloneNode(TreeNode* oldNode) {
	if (oldNode == NULL) {
		return oldNode;
	}
	TreeNode* newNode = allocNode();
	//Copy basic stuff over to the new one
	memcpy(newNode, oldNode, sizeof(TreeNode));

	//Clone children
	int q = 0;
	for (q = 0; q < MAXCHILDREN; q++) {
		newNode->child[q] = cloneNode(oldNode->child[q]);
	}
	
	//While cloning the sibling, that sibling will clone its sibling, and so on
	newNode->sibling = cloneNode(oldNode->sibling);

	return newNode;
}


TreeNode* newGenericNode(TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2) {
	TreeNode* newNode = allocNode();
	newNode->lineno = token->linenum;
	newNode->attr.name = token->svalue;

	newNode->isArray = false;

	
	newNode->child[0] = c0;
	newNode->child[1] = c1;
	newNode->child[2] = c2;
	return newNode;
}

//I don't think we need all of these functions but BC provided them so here we go

TreeNode *newDeclNode(DeclKind kind, ExpType type, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2) {
	TreeNode* newNode = newGenericNode(token, c0, c1, c2);
	newNode->nodekind = NodeKind::DeclK;
	newNode->kind.decl = kind;
	newNode->type = type;
	
	return newNode;
}
TreeNode *newStmtNode(StmtKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2) {
	TreeNode* newNode = newGenericNode(token, c0, c1, c2);
	newNode->nodekind = NodeKind::StmtK;
	newNode->kind.stmt = kind;
	return newNode;
}
TreeNode *newExpNode(ExpKind kind, TokenData *token, TreeNode *c0L, TreeNode *c1L, TreeNode *c2L) {
	TreeNode* newNode = newGenericNode(token, c0L, c1L, c2L);
	newNode->nodekind = NodeKind::ExpK;
	newNode->kind.exp = kind;

	if (kind == ExpKind::IdK) {
		newNode->attr.name = token->svalue;
	}

	if (kind == ExpKind::ConstantK) {
		newNode->attr.value = token->nvalue;
		newNode->attr.string = token->svalue;
		newNode->attr.cvalue = token->cvalue;
	}

	newNode->attr.op = OpKind(token->tokenclass);

	//Gonna do some string compare shit
	//Make OpType an actual enum class

	return newNode;
}


extern char* largerTokens[500];

const char *tokenToStr(int type) {
	switch (type) {
		case '=':
		return "=";
		default: 
		return largerTokens[type];
	}
}
const char *expTypeToStr(ExpType type, bool isArray, bool isStatic) {
	if (isArray) {
		switch (type) {
			case ExpType::Boolean:
			return "array of type bool";
			case ExpType::Char:
			return "array of type char";
			case ExpType::Integer:
			return "array of type int";
			case ExpType::UndefinedType:
			return "array of type UNDEFINED";
			case ExpType::Void:
			return "array of type void";
			default: 
			return "array of BAD EXP TYPE";
		}
	} else {
		switch (type) {
			case ExpType::Boolean:
			return "type bool";
			case ExpType::Char:
			return "type char";
			case ExpType::Integer:
			return "type int";
			case ExpType::UndefinedType:
			return "type UNDEFINED";
			case ExpType::Void:
			return "type void";
			default: 
			return "BAD EXP TYPE";
		}
	}
}

const char* varKindToStr(VarKind kind) {
	switch (kind) {
		case VarKind::Global:
		return "Global";
		case VarKind::LocalStatic:
		return "LocalStatic";
		case VarKind::Local:
		return "Local";
		case VarKind::Parameter:
		return "Parameter";
		case VarKind::None:
		return "None";
		default:
		return "BAD VARKIND";
	}
}

// print a node without a newline
void printTreeNode(FILE *listing,
				   TreeNode *tree,
				   bool showExpType,
				   bool showAllocation)
{
	if (!tree) {
		fprintf(listing, "NULL");
		return;
	}
   // print a declaration node
	if (tree->nodekind == NodeKind::DeclK) {
   switch (tree->kind.decl) {
   case DeclKind::VarK:
			printf("Var: %s ", tree->attr.name);
			printf("of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   case DeclKind::FuncK:
			printf("Func: %s ", tree->attr.name);
			//EDITED
			printf("returns %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   case DeclKind::ParamK:
			printf("Parm: %s ", tree->attr.name);
			printf("of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   default:
	   fprintf(listing, "Unknown declaration node kind: %d",
		  (int)tree->kind.decl);
	   break;
   }
	}

	// print a statement node
	else if (tree->nodekind == NodeKind::StmtK) {
   switch (tree->kind.stmt) {
   case StmtKind::IfK:
	   fprintf(listing, "If");
	   break;
   case StmtKind::WhileK:
	   fprintf(listing, "While");
	   break;
   case StmtKind::CompoundK:
	   fprintf(listing, "Compound");
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
		case StmtKind::ForK:
	   fprintf(listing, "For");
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
		case StmtKind::RangeK:
	   fprintf(listing, "Range");
	   break;
   case StmtKind::ReturnK:
	   fprintf(listing, "Return");
	   break;
   case StmtKind::BreakK:
	   fprintf(listing, "Break");
	   break;
   default:
	   fprintf(listing, "Unknown  statement node kind: %d",
		  (int)tree->kind.stmt);
	   break;
   }
	}

	// print an expression node
	else if (tree->nodekind == NodeKind::ExpK) {
		switch (tree->kind.exp) {
		case ExpKind::AssignK:
			fprintf(listing, "Assign: %s", tokenToStr(tree->attr.op));
			break;
		case ExpKind::OpK:
			//If this is the '-'
			if (tokenToStr(tree->attr.op)[0] == '-' && tokenToStr(tree->attr.op)[1] == 0) {
				//Check if unary op (unary should have only one child)
				if (tree->child[1]) {
					fprintf(listing, "Op: -");
				} else {
					fprintf(listing, "Op: chsign");
				}
				//If this is the other special case unary operator
			} else if (tokenToStr(tree->attr.op)[0] == '*' && tokenToStr(tree->attr.op)[1] == 0){
				if (tree->child[1]) {
					fprintf(listing, "Op: *");
				} else {
					fprintf(listing, "Op: sizeof");
				}
			} else {
				fprintf(listing, "Op: %s", tokenToStr(tree->attr.op));
			}
			break;
		case ExpKind::ConstantK:
				switch (tree->type) {
				case ExpType::Boolean:
		fprintf(listing, "Const %s", (tree->attr.value) ?  "true" : "false");
					break;
				case ExpType::Integer:
		fprintf(listing, "Const %d", tree->attr.value);
					break;
				case ExpType::Char:
					if (tree->isArray) {
						fprintf(listing, "Const ");
						printf("\"");
						for (int i=0; i<tree->size-1; i++) {
							printf("%c", tree->attr.string[i]);
						}
						printf("\"");
					}
		else fprintf(listing, "Const '%c'", tree->attr.cvalue);
					break;
				case ExpType::Void:
				case ExpType::UndefinedType:
					fprintf(listing, "SYSTEM ERROR: parse tree contains invalid type for constant: %s\n", expTypeToStr(tree->type));
		}
		break;
		case ExpKind::IdK:
			fprintf(listing, "Id: %s", tree->attr.name);
			break;
		case ExpKind::CallK:
			fprintf(listing, "Call: %s", tree->attr.name);
			//IMPORTANT Maybe?
			break;
		default:
			fprintf(listing, "Unknown expression node kind: %d", (int)tree->kind.exp);
			break;
		}
		if (showExpType) {
			fprintf(listing, " of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
		}
		if (showAllocation) {
			if (tree->kind.exp == ExpKind::IdK || tree->kind.exp == ExpKind::ConstantK && tree->type == ExpType::Char && tree->isArray) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
		}
	}
	else fprintf(listing, "Unknown class of node: %d",
	   (int)tree->nodekind);

	fprintf(listing, " [line: %d]", tree->lineno);
}






void printDepth(FILE* file, int depth) {
	for (int q = 0; q < depth; q++) {
		fprintf(file, ".   ");
	}
}

void printTreeRecursive(FILE* out, TreeNode* tree, bool showExpType, bool showAllocation, int depth, int siblingCount = 1) {
	if (tree == NULL) {
		return;
	}


	//Print this node
	printTreeNode(out, tree, showExpType, showAllocation);
	fprintf(out, "\n");

	//Print children
	for (int q = 0; q < MAXCHILDREN; q++) {
		if (tree->child[q]) {
			printDepth(out, depth + 1);
			//Two spaces after this because BC loves her weird spacing
			fprintf(out, "Child: %d  ", q);
			printTreeRecursive(out, tree->child[q], showExpType, showAllocation, depth + 1);
		}
	}

	//Print this sibling
	//Each sibling will print it's next sibling, and so on, so no loop needed
	TreeNode* sibling = tree->sibling;
	if (sibling) {
		printDepth(out, depth);
		fprintf(out, "Sibling: %d  ", siblingCount);
		printTreeRecursive(out, sibling, showExpType, showAllocation, depth, siblingCount + 1);
	}
	
}

//Prints the tree all nice-like
void printTree(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation) {

	if (syntaxTree == NULL) {
		fprintf(out, "NULL\n");
		return;
	}

	printTreeRecursive(out, syntaxTree, showExpType, showAllocation, 0);
}

