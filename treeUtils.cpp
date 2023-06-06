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
	//IMPORTANT I am unsure about these following lines
	if (c0) {
		c0->lineno = token->linenum;
		c0->attr.op = token->tokenclass;
	}
	if (c1) {
		c1->lineno = token->linenum;
		c1->attr.op = token->tokenclass;
	}
	if (c2) {
		c2->lineno = token->linenum;
		c2->attr.op = token->tokenclass;
	}
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
	return "NotImplemented";
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

void printDepth(FILE* file, int depth) {
	for (int q = 0; q < depth; q++) {
		fprintf(file, "* ");
	}
}

void printTree(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation) {
	fprintf(out, "Printing Tree:\n");

	if (syntaxTree == NULL) {
		fprintf(out, "NULL\n");
		return;
	}

	int depth = 1;
	printTreeNode(out, syntaxTree, showExpType, showAllocation);
	while (syntaxTree) {

		printDepth(out, depth);
		fprintf(out, "Siblings:\n");
		TreeNode* next = syntaxTree->sibling;
		while (next) {
			printDepth(out, depth);
			printTreeNode(out, next, showExpType, showAllocation);
			next = next->sibling;
		}

		printDepth(out, depth);
		fprintf(out, "Children:\n");
		for (int q = 0; q < MAXCHILDREN; q++) {
			if (syntaxTree->child[q]) {
				printDepth(out, depth);
				fprintf(out, "Child %d:", q);
				printDepth(out, depth);
				printTreeNode(out, syntaxTree->child[q], showExpType, showAllocation);
			}
		}
		break;
		depth++;
	}
}