#include <treeUtils.h>


TreeNode *cloneNode(TreeNode *currnode) {

}
TreeNode *newDeclNode(DeclKind kind,
	ExpType type,
	TokenData *token,
	TreeNode *c0,
	TreeNode *c1,
	TreeNode *c2) { // save TokenData block!!

}
TreeNode *newStmtNode(StmtKind kind,
	TokenData *token,
	TreeNode *c0,
	TreeNode *c1,
	TreeNode *c2) {

	}
TreeNode *newExpNode(ExpKind kind,
	TokenData *token,
	TreeNode *c0L,
	TreeNode *c1L,
	TreeNode *c2L) {

	}
char *tokenToStr(int type) {

}
char *expTypeToStr(ExpType type, bool isArray, bool isStatic) {

}

void printTreeNode(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation) {

}
void printTree(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation) {

}