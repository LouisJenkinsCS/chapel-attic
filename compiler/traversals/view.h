#ifndef _VIEW_H_
#define _VIEW_H_

#include "traversal.h"
#include "baseAST.h"

#define MAX_VIEW_LINES 10000

class View : public Traversal {
 public:
  int indent;
  bool numberSymbols;
  long markAST;
  View::View(bool initNumberSymbols = false, int initMarkAST = -1);
  virtual void run(Vec<ModuleSymbol*>* modules);
  virtual void preProcessStmt(Stmt* stmt);
  virtual void postProcessStmt(Stmt* stmt);
  virtual void preProcessExpr(Expr* expr);
  virtual void postProcessExpr(Expr* expr);
  virtual void preProcessSymbol(Symbol* sym);
  virtual void postProcessSymbol(Symbol* sym);
  virtual void preProcessType(Type* type);
  virtual void postProcessType(Type* type);
};

extern void print_view(BaseAST* ast);
extern void print_view_noline(BaseAST* ast);
extern void nprint_view(BaseAST* ast);
extern void mark_view(BaseAST* ast, long id);

void view(void);

#endif
