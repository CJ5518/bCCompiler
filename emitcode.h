#ifndef EMIT_CODE_H__
#define EMIT_CODE_H__

//
//  REGISTER DEFINES for optional use in calling the 
//  routines below.
//
#define GP   0	//  The global pointer
#define FP   1	//  The local frame pointer
#define RT   2	//  Return value
#define AC   3  //  Accumulator
#define AC1  4  //  Accumulator
#define AC2  5  //  Accumulator
#define AC3  6  //  Accumulator
#define PC   7	//  The program counter

//
//  No comment please...
//
#define NO_COMMENT (char *)""


//
//  The following functions were borrowed from Tiny compiler code generator
//
int emitWhereAmI();           // gives where the next instruction will be placed
int emitSkip(int howMany);    // emitSkip(0) tells you where the next instruction will be placed
void emitNewLoc(int loc);     // set the instruction counter back to loc

void emitComment(const char *c);
void emitComment(const char *c, const char *cc);
void emitComment(const char *c, int n);

void emitLongComment();
void emitStandardClosing();

void emitGoto(int d, long long int s, const char *c);
void emitGoto(int d, long long int s, const char *c, const char *cc);
void emitGotoAbs(int a, const char *c);
void emitGotoAbs(int a, const char *c, const char *cc);

void emitRM(const char *op, long long int r, long long int d, long long int s, const char *c);
void emitRM(const char *op, long long int r, long long int d, long long int s, const char *c, const char *cc);
void emitRMAbs(const char *op, long long int r, long long int a, const char *c);
void emitRMAbs(const char *op, long long int r, long long int a, const char *c, const char *cc);

void emitRO(const char *op, long long int r, long long int s, long long int t, const char *c);
void emitRO(const char *op, long long int r, long long int s, long long int t, const char *c, const char *cc);

void backPatchAJumpToHere(int addr, const char *comment);
void backPatchAJumpToHere(const char *cmd, int reg, int addr, const char *comment);

int emitStrLit(int goffset, const char *s); // for char arrays

#endif
