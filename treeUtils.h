#ifndef _UTIL_H_
#define _UTIL_H_
#include "treeNodes.h"
#include "scanType.h"
#include <string.h>

//Clones a node, does a deep copy and clones all children and siblings
TreeNode *cloneNode(TreeNode *currnode);
TreeNode *newDeclNode(DeclKind kind,
					  ExpType type,
					  TokenData *token=NULL,
					  TreeNode *c0=NULL,
					  TreeNode *c1=NULL,
					  TreeNode *c2=NULL);  // save TokenData block!!
TreeNode *newStmtNode(StmtKind kind,
					  TokenData *token,
					  TreeNode *c0=NULL,
					  TreeNode *c1=NULL,
					  TreeNode *c2=NULL);
TreeNode *newExpNode(ExpKind kind,
					 TokenData *token,
					 TreeNode *c0=NULL,
					 TreeNode *c1=NULL,
					 TreeNode *c2=NULL);
const char *tokenToStr(int type);
const char *expTypeToStr(ExpType type, bool isArray=false, bool isStatic=false);

void printTreeNode(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation);
void printTree(FILE *out, TreeNode *syntaxTree, bool showExpType, bool showAllocation);

#endif
