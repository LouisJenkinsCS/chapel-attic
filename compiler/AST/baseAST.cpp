#include "astutil.h"
#include "baseAST.h"
#include "expr.h"
#include "passes.h"
#include "stmt.h"
#include "stringutil.h"
#include "symbol.h"
#include "type.h"
#include "yy.h"

static void cleanModuleList();

//
// declare global vectors gSymExprs, gCallExprs, gFnSymbols, ...
//
#define decl_gvecs(type) Vec<type*> g##type##s
foreach_ast(decl_gvecs);

static int uid = 1;

#define decl_counters(type)                                             \
  int n##type = g##type##s.n, k##type = n##type*sizeof(type)/1024

#define sum_gvecs(type) g##type##s.n

void printStatistics(const char* pass) {
  static int last_nasts = -1;
  static int maxK = -1, maxN = -1;

  if (!strcmp(pass, "makeBinary")) {
    if (strstr(fPrintStatistics, "m")) {
      fprintf(stderr, "Maximum # of ASTS: %d\n", maxN);
      fprintf(stderr, "Maximum Size (KB): %d\n", maxK);
    }
  }

  int nasts = foreach_ast_sep(sum_gvecs, +);

  if (last_nasts == nasts) {
    fprintf(stderr, "%23s%s\n", "", pass);
    return;
  }

  foreach_ast(decl_counters);

  int nStmt = nCondStmt + nBlockStmt + nGotoStmt;
  int kStmt = kCondStmt + kBlockStmt + kGotoStmt;
  int nExpr = nUnresolvedSymExpr + nSymExpr + nDefExpr + nCallExpr + nNamedExpr;
  int kExpr = kUnresolvedSymExpr + kSymExpr + kDefExpr + kCallExpr + kNamedExpr;
  int nSymbol = nModuleSymbol+nVarSymbol+nArgSymbol+nTypeSymbol+nFnSymbol+nEnumSymbol+nLabelSymbol;
  int kSymbol = kModuleSymbol+kVarSymbol+kArgSymbol+kTypeSymbol+kFnSymbol+kEnumSymbol+kLabelSymbol;
  int nType = nPrimitiveType+nEnumType+nClassType;
  int kType = kPrimitiveType+kEnumType+kClassType;

  if (strstr(fPrintStatistics, "n"))
    fprintf(stderr, "   Interfaces: %d\n", kInterfaceSymbol);  //FIXME: Make this part of statistics

  fprintf(stderr, "%7d asts (%6dK) %s\n", nStmt+nExpr+nSymbol+nType, kStmt+kExpr+kSymbol+kType, pass);

  if (nStmt+nExpr+nSymbol+nType > maxN)
    maxN = nStmt+nExpr+nSymbol+nType;

  if (kStmt+kExpr+kSymbol+kType > maxK)
    maxK = kStmt+kExpr+kSymbol+kType;

  if (strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Stmt %9d  Cond %9d  Block %9d  Goto  %9d\n",
            nStmt, nCondStmt, nBlockStmt, nGotoStmt);
  if (strstr(fPrintStatistics, "k") && strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Stmt %9dK Cond %9dK Block %9dK Goto  %9dK\n",
            kStmt, kCondStmt, kBlockStmt, kGotoStmt);
  if (strstr(fPrintStatistics, "k") && !strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Stmt %6dK Cond %6dK Block %6dK Goto  %6dK\n",
            kStmt, kCondStmt, kBlockStmt, kGotoStmt);

  if (strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Expr %9d  Unre %9d  Sym  %9d  Def   %9d  Call  %9d  Named %9d\n",
            nExpr, nUnresolvedSymExpr, nSymExpr, nDefExpr, nCallExpr, nNamedExpr);
  if (strstr(fPrintStatistics, "k") && strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Expr %9dK Unre %9dK Sym  %9dK Def   %9dK Call  %9dK Named %9dK\n",
            kExpr, kUnresolvedSymExpr, kSymExpr, kDefExpr, kCallExpr, kNamedExpr);
  if (strstr(fPrintStatistics, "k") && !strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Expr %6dK Unre %6dK Sym  %6dK Def   %6dK Call  %6dK Named %6dK\n",
            kExpr, kUnresolvedSymExpr, kSymExpr, kDefExpr, kCallExpr, kNamedExpr);

  if (strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Sym  %9d  Mod  %9d  Var   %9d  Arg   %9d  Type %9d  Fn %9d  Enum %9d  Label %9d\n",
            nSymbol, nModuleSymbol, nVarSymbol, nArgSymbol, nTypeSymbol, nFnSymbol, nEnumSymbol, nLabelSymbol);
  if (strstr(fPrintStatistics, "k") && strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Sym  %9dK Mod  %9dK Var   %9dK Arg   %9dK Type %9dK Fn %9dK Enum %9dK Label %9dK\n",
            kSymbol, kModuleSymbol, kVarSymbol, kArgSymbol, kTypeSymbol, kFnSymbol, kEnumSymbol, kLabelSymbol);
  if (strstr(fPrintStatistics, "k") && !strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Sym  %6dK Mod  %6dK Var   %6dK Arg   %6dK Type %6dK Fn %6dK Enum %6dK Label %6dK\n",
            kSymbol, kModuleSymbol, kVarSymbol, kArgSymbol, kTypeSymbol, kFnSymbol, kEnumSymbol, kLabelSymbol);

  if (strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Type %9d  Prim  %9d  Enum %9d  Class %9d \n",
            nType, nPrimitiveType, nEnumType, nClassType);
  if (strstr(fPrintStatistics, "k") && strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Type %9dK Prim  %9dK Enum %9dK Class %9dK\n",
            kType, kPrimitiveType, kEnumType, kClassType);
  if (strstr(fPrintStatistics, "k") && !strstr(fPrintStatistics, "n"))
    fprintf(stderr, "    Type %6dK Prim  %6dK Enum %6dK Class %6dK\n",
            kType, kPrimitiveType, kEnumType, kClassType);
  last_nasts = nasts;
}

// for debugging purposes only
void trace_remove(BaseAST* ast, char flag) {
  // crash if deletedIdHandle is not initialized but deletedIdFilename is
  if (deletedIdON) {
    fprintf(deletedIdHandle, "%d %c %p %d\n",
            currentPassNo, flag, ast, ast->id);
  }
  if (ast->id == breakOnDeleteID) {
    if (deletedIdON) fflush(deletedIdHandle);
    gdbShouldBreakHere();
  }
}

#define clean_gvec(type)                        \
  int i##type = 0;                              \
  forv_Vec(type, ast, g##type##s) {             \
    if (isAlive(ast) || isRootModuleWithType(ast, type)) { \
      g##type##s.v[i##type++] = ast;            \
    } else {                                    \
      trace_remove(ast, 'x');                   \
      delete ast;                               \
    }                                           \
  }                                             \
  g##type##s.n = i##type

void cleanAst() {
  cleanModuleList();
  //
  // clear back pointers to dead ast instances
  //
  forv_Vec(TypeSymbol, ts, gTypeSymbols) {
    for(int i = 0; i < ts->type->methods.n; i++) {
      FnSymbol* method = ts->type->methods.v[i];
      if (method && !isAliveQuick(method))
        ts->type->methods.v[i] = NULL;
      if (ClassType* ct = toClassType(ts->type)) {
        if (ct->defaultConstructor && !isAliveQuick(ct->defaultConstructor))
          ct->defaultConstructor = NULL;
        if (ct->destructor && !isAliveQuick(ct->destructor))
          ct->destructor = NULL;
      }
    }
    for(int i = 0; i < ts->type->dispatchChildren.n; i++) {
      Type* type = ts->type->dispatchChildren.v[i];
      if (type && !isAlive(type))
        ts->type->dispatchChildren.v[i] = NULL;
    }
  }

  // check iterator-resume-label/goto data before nodes are free'd
  verifyNcleanRemovedIterResumeGotos();
  verifyNcleanCopiedIterResumeGotos();

  //
  // clean global vectors and delete dead ast instances
  //
  foreach_ast(clean_gvec);
}


// ModuleSymbols cache a pointer to their initialization function
// This pointer has to be zeroed out specially before the matching function
// definition is deleted from module body.
static void cleanModuleList()
{
  // Walk the module list.
  forv_Vec(ModuleSymbol, mod, gModuleSymbols) {
    // Zero the initFn pointer if the function is now dead.
    if (mod->initFn && !isAlive(mod->initFn))
      mod->initFn = NULL;
  }
}


void destroyAst() {
  #define destroy_gvec(type)                    \
    forv_Vec(type, ast, g##type##s) {           \
      trace_remove(ast, 'z');                   \
      delete ast;                               \
    }
  foreach_ast(destroy_gvec);
}


void
verify() {
  #define verify_gvec(type)                       \
    forv_Vec(type, ast, g##type##s) {             \
      ast->verify();                              \
    }
  foreach_ast(verify_gvec);
}


int breakOnID = -1;
int breakOnDeleteID = -1;

int lastNodeIDUsed() {
  return uid - 1;
}


// This is here so that we can break on the creation of a particular
// BaseAST instance in gdb.
static void checkid(int id) {
  if (id == breakOnID) {
    gdbShouldBreakHere();
  }
}


BaseAST::BaseAST(AstTag type) :
  astTag(type),
  id(uid++),
  astloc(yystartlineno, yyfilename)
{
  checkid(id);
  if (astloc.lineno == -1) {
    if (currentAstLoc.lineno) {
      astloc = currentAstLoc;
    }
  }
}


const char* BaseAST::stringLoc(void) {
  const int tmpBuffSize = 256;
  char tmpBuff[tmpBuffSize];

  snprintf(tmpBuff, tmpBuffSize, "%s:%d", fname(), linenum());
  return astr(tmpBuff);
}

// stringLoc for debugging only
char* stringLoc(BaseAST* ast);
char* stringLoc(int id);
BaseAST* aid(int id);

char* stringLoc(BaseAST* ast) {
  if (!ast)
    return (char*)"<no node provided>";

  const int tmpBuffSize = 256;
  static char tmpBuff[tmpBuffSize];

  snprintf(tmpBuff, tmpBuffSize, "%s:%d", ast->fname(), ast->linenum());
  return tmpBuff;
}

char* stringLoc(int id) {
  BaseAST* ast = aid(id);
  if (ast)
    return stringLoc(aid(id));
  else
    return (char*)"<the given ID does not correspond to any AST node>";
}

ModuleSymbol* BaseAST::getModule() {
  if (!this)
    return NULL;
  if (ModuleSymbol* x = toModuleSymbol(this))
    return x;
  else if (Type* x = toType(this))
    return x->symbol->getModule();
  else if (Symbol* x = toSymbol(this))
    return x->defPoint->getModule();
  else if (Expr* x = toExpr(this))
    return x->parentSymbol->getModule();
  else
    INT_FATAL(this, "Unexpected case in BaseAST::getModule()");
  return NULL;
}


FnSymbol* BaseAST::getFunction() {
  if (!this)
    return NULL;
  if (ModuleSymbol* x = toModuleSymbol(this))
    return x->initFn;
  else if (FnSymbol* x = toFnSymbol(this))
    return x;
  else if (Type* x = toType(this))
    return x->symbol->getFunction();
  else if (Symbol* x = toSymbol(this))
    return x->defPoint->getFunction();
  else if (Expr* x = toExpr(this))
    return x->parentSymbol->getFunction();
  else
    INT_FATAL(this, "Unexpected case in BaseAST::getFunction()");
  return NULL;
}


Type* BaseAST::getValType() {
  Type* type = typeInfo();
  INT_ASSERT(type);
  if (type->symbol->hasFlag(FLAG_REF))
    return type->getField("_val")->type;
  else if (type->symbol->hasFlag(FLAG_WIDE))
    return type->getField("addr")->getValType();
  else
    return type;
}

Type* BaseAST::getRefType() {
  Type* type = typeInfo();
  INT_ASSERT(type);
  if (type->symbol->hasFlag(FLAG_REF))
    return type;
  else if (type->symbol->hasFlag(FLAG_WIDE))
    return type->getField("addr")->type;
  else
    return type->refType;
}

Type* BaseAST::getWideRefType() {
  Type* type = typeInfo();
  INT_ASSERT(type);
  if (type->symbol->hasFlag(FLAG_REF))
    return wideRefMap.get(type);
  else if (type->symbol->hasFlag(FLAG_WIDE))
    return type;
  else
    return wideRefMap.get(type->getRefType());
}


const char* astTagName[E_BaseAST+1] = {
  "SymExpr",
  "UnresolvedSymExpr",
  "DefExpr",
  "CallExpr",
  "NamedExpr",
  "BlockStmt",
  "CondStmt",
  "GotoStmt",
  "Expr",

  "ModuleSymbol",
  "VarSymbol",
  "ArgSymbol",
  "TypeSymbol",
  "FnSymbol",
  "EnumSymbol",
  "LabelSymbol",
  "Symbol",

  "PrimitiveType",
  "EnumType",
  "ClassType",
  "Type",

  "BaseAST"
};

astlocT currentAstLoc(0,NULL);

Vec<ModuleSymbol*> mainModules; // Contains main modules
Vec<ModuleSymbol*> userModules; // Contains user + main modules
Vec<ModuleSymbol*> allModules;  // Contains all modules

void registerModule(ModuleSymbol* mod) {
  switch (mod->modTag) {
  case MOD_MAIN:
    mainModules.add(mod);
  case MOD_USER:
    userModules.add(mod);
  case MOD_STANDARD:
  case MOD_INTERNAL:
    if (strcmp(mod->name, "_root"))
      allModules.add(mod);
    break;
  default:
    INT_FATAL(mod, "Unable to register module");
  }
}

#define SUB_SYMBOL(x)                                   \
  do {                                                  \
    if (x)                                              \
      if (Symbol* y = map->get(x))                      \
        x = y;                                          \
  } while (0)

#define SUB_LABEL(x)                                    \
  do {                                                  \
    if (x)                                              \
      if (LabelSymbol* y = toLabelSymbol(map->get(x)))  \
        x = y;                                          \
  } while (0)

#define SUB_TYPE(x)                                     \
  do {                                                  \
    if (x)                                              \
      if (Symbol* y = map->get(x->symbol))              \
        x = y->type;                                    \
  } while (0)

void update_symbols(BaseAST* ast, SymbolMap* map) {
  if (SymExpr* sym_expr = toSymExpr(ast)) {
    SUB_SYMBOL(sym_expr->var);
  } else if (DefExpr* defExpr = toDefExpr(ast)) {
    SUB_TYPE(defExpr->sym->type);
  } else if (BlockStmt* bs = toBlockStmt(ast)) {
    SUB_LABEL(bs->breakLabel);
    SUB_LABEL(bs->continueLabel);
  } else if (VarSymbol* ps = toVarSymbol(ast)) {
    SUB_TYPE(ps->type);
  } else if (FnSymbol* ps = toFnSymbol(ast)) {
    SUB_TYPE(ps->type);
    SUB_TYPE(ps->retType);
    SUB_SYMBOL(ps->_this);
    SUB_SYMBOL(ps->_outer);
  } else if (ArgSymbol* ps = toArgSymbol(ast)) {
    SUB_TYPE(ps->type);
  }
  AST_CHILDREN_CALL(ast, update_symbols, map);
}
