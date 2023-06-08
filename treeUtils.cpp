#include <treeUtils.h>
#include <stdlib.h>


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
	newNode->attr.op = token->tokenclass;
	
	newNode->child[0] = c0;
	newNode->child[1] = c1;
	newNode->child[2] = c2;
	return newNode;
}

//I don't think we need all of these functions but BC provided them so here we go

TreeNode *newDeclNode(DeclKind kind, ExpType type, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2) {
	TreeNode* newNode = newGenericNode(token, c0, c1, c2);
	newNode->nodekind = DeclK;
	newNode->kind.decl = kind;
	newNode->attr.name = token->svalue;
	newNode->type = type;
	
	return newNode;
}
TreeNode *newStmtNode(StmtKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2) {
	TreeNode* newNode = newGenericNode(token, c0, c1, c2);
	newNode->nodekind = StmtK;
	newNode->kind.stmt = kind;
	return newNode;
}
TreeNode *newExpNode(ExpKind kind, TokenData *token, TreeNode *c0L, TreeNode *c1L, TreeNode *c2L) {
	TreeNode* newNode = newGenericNode(token, c0L, c1L, c2L);
	newNode->nodekind = ExpK;
	newNode->kind.exp = kind;
	return newNode;
}
const char *tokenToStr(int type) {
	return "NotImplemented";
}
const char *expTypeToStr(ExpType type, bool isArray, bool isStatic) {
	switch (type) {
		case ExpType::Boolean:
		return "boolean";
		case ExpType::Char:
		return "char";
		case ExpType::Integer:
		return "integer";
		case ExpType::UndefinedType:
		return "UNDEFINED";
		case ExpType::Void:
		return "void";
		default: 
		return "BAD EXP TYPE";
	}
}

//Use fprintf
void printTreeNode(FILE *out, TreeNode *node, bool showExpType, bool showAllocation) {
	fprintf(out, "Node number: %d\n", node->nodeNum);
	fprintf(out, "Line number: %d\n", node->lineno);
	fprintf(out, "nodekind: %d", node->nodekind);
	switch (node->nodekind) {
		case NodeKind::DeclK:
			fprintf(out, "%d\n", node->kind.decl);
		break;
		case NodeKind::ExpK:
			fprintf(out, "%d\n", node->kind.exp);
		break;
		case NodeKind::StmtK:
			fprintf(out, "%d\n", node->kind.stmt);
		break;
		default:
			fprintf(out, "Bad NodeKind in node\n");
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
void printTreeNodeBC(FILE *listing,
				   TreeNode *tree,
				   bool showExpType,
				   bool showAllocation)
{
   // print a declaration node
	if (tree->nodekind == DeclK) {
   switch (tree->kind.decl) {
   case VarK:
			printf("Var: %s ", tree->attr.name);
			printf("of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   case FuncK:
			printf("Func: %s ", tree->attr.name);
			//EDITED
			printf("returns type %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   case ParamK:
			printf("Parm: %s ", tree->attr.name);
			printf("of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
   default:
	   fprintf(listing, "Unknown declaration node kind: %d",
		  tree->kind.decl);
	   break;
   }
	}

	// print a statement node
	else if (tree->nodekind == StmtK) {
   switch (tree->kind.stmt) {
   case IfK:
	   fprintf(listing, "If");
	   break;
   case WhileK:
	   fprintf(listing, "While");
	   break;
   case CompoundK:
	   fprintf(listing, "Compound");
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
		case ForK:
	   fprintf(listing, "For");
			if (showAllocation) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
	   break;
		case RangeK:
	   fprintf(listing, "Range");
	   break;
   case ReturnK:
	   fprintf(listing, "Return");
	   break;
   case BreakK:
	   fprintf(listing, "Break");
	   break;
   default:
	   fprintf(listing, "Unknown  statement node kind: %d",
		  tree->kind.stmt);
	   break;
   }
	}

	// print an expression node
	else if (tree->nodekind == ExpK) {
   switch (tree->kind.exp) {
   case AssignK:
	   fprintf(listing, "Assign: %s", tokenToStr(tree->attr.op));
	   break;
   case OpK:
	   fprintf(listing, "Op: %s", tokenToStr(tree->attr.op));
	   break;
   case ConstantK:
			switch (tree->type) {
			case Boolean:
	  fprintf(listing, "Const %s", (tree->attr.value) ?  "true" : "false");
				break;
			case Integer:
	  fprintf(listing, "Const %d", tree->attr.value);
				break;
			case Char:
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
			case Void:
			case UndefinedType:
				fprintf(listing, "SYSTEM ERROR: parse tree contains invalid type for constant: %s\n", expTypeToStr(tree->type));
	   }
	   break;
   case IdK:
	   fprintf(listing, "Id: %s", tree->attr.name);
	   break;
   case CallK:
	   fprintf(listing, "Call: %s", tree->attr.name);
	   break;
   default:
	   fprintf(listing, "Unknown expression node kind: %d", tree->kind.exp);
	   break;
   }
   if (showExpType) {
	   fprintf(listing, " of %s", expTypeToStr(tree->type, tree->isArray, tree->isStatic));
   }
		if (showAllocation) {
			if (tree->kind.exp == IdK || tree->kind.exp == ConstantK && tree->type == Char && tree->isArray) {
				printf(" [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
			}
		}
	}
	else fprintf(listing, "Unknown class of node: %d",
	   tree->nodekind);

	fprintf(listing, " [line: %d]", tree->lineno);
}






void printDepth(FILE* file, int depth) {
	for (int q = 0; q < depth; q++) {
		fprintf(file, ".   ");
	}
}

//Prints the tree all nice-like
void printTree(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation) {

	if (syntaxTree == NULL) {
		fprintf(out, "NULL\n");
		return;
	}

	int depth = 1;
	printTreeNodeBC(out, syntaxTree, showExpType, showAllocation);
	while (syntaxTree) {
		
		TreeNode* next = syntaxTree->sibling;
		while (next) {
			printDepth(out, depth);
			printTreeNodeBC(out, next, showExpType, showAllocation);
			next = next->sibling;
		}

		fprintf(out, "\n");
		for (int q = 0; q < MAXCHILDREN; q++) {
			if (syntaxTree->child[q]) {
				printDepth(out, depth);
				fprintf(out, "Child: %d  ", q);
				printTreeNodeBC(out, syntaxTree->child[q], showExpType, showAllocation);
				fprintf(out, "\n");
			}
		}
		break;
		depth++;
	}
}