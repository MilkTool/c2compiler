/* Copyright 2013 Bas van den Berg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STMT_H
#define STMT_H

#include <string>
#include <vector>

#include <clang/Basic/SourceLocation.h>
#include "OwningVector.h"

using clang::SourceLocation;

namespace llvm {
class Value;
}

namespace C2 {

class StringBuilder;
class StmtVisitor;
class Expr;
class CodeGenContext;

enum StmtType {
    STMT_RETURN = 0,
    STMT_EXPR,
    STMT_IF,
    STMT_WHILE,
    STMT_DO,
    STMT_SWITCH,
    STMT_CASE,
    STMT_DEFAULT,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_LABEL,
    STMT_GOTO,
    STMT_COMPOUND,
};


class Stmt {
public:
    Stmt();
    virtual ~Stmt();
    virtual StmtType stype() = 0;
    virtual void acceptS(StmtVisitor& v) = 0;
    virtual void print(int indent, StringBuilder& buffer) = 0;
    virtual void generateC(int indent, StringBuilder& buffer) = 0;
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0;
    void dump();
private:
    Stmt(const Stmt&);
    Stmt& operator= (const Stmt&);
};


typedef std::vector<Stmt*> StmtList;
typedef OwningVector<Stmt> StmtList2;

class ReturnStmt : public Stmt {
public:
    ReturnStmt(Expr* value_);
    virtual ~ReturnStmt();
    virtual StmtType stype() { return STMT_RETURN; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    Expr* value;
    // TODO clang::SourceLocation
};


class IfStmt : public Stmt {
public:
    IfStmt(SourceLocation ifLoc,
           Expr* condition, Stmt* thenStmt,
           SourceLocation elseLoc, Stmt* elseStmt);
    virtual ~IfStmt();
    virtual StmtType stype() { return STMT_IF; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    enum { VAR, COND, THEN, ELSE, END_EXPR };
    Stmt* SubExprs[END_EXPR];

    SourceLocation IfLoc;
    SourceLocation ElseLoc;
};


class WhileStmt : public Stmt {
public:
    WhileStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_);
    virtual ~WhileStmt();
    virtual StmtType stype() { return STMT_WHILE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
    Stmt* Cond;
    Stmt* Then;
};


class DoStmt : public Stmt {
public:
    DoStmt(SourceLocation Loc_, Expr* Cond_, Stmt* Then_);
    virtual ~DoStmt();
    virtual StmtType stype() { return STMT_DO; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
    Stmt* Cond;
    Stmt* Then;
};


class SwitchStmt : public Stmt {
public:
    SwitchStmt(SourceLocation Loc_, Expr* Cond_, StmtList2& Cases_);
    virtual ~SwitchStmt();
    virtual StmtType stype() { return STMT_SWITCH; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
    Expr* Cond;
    StmtList2 Cases;
};


class CaseStmt : public Stmt {
public:
    CaseStmt(SourceLocation Loc_, Expr* Cond_, StmtList2& Stmts_);
    virtual ~CaseStmt();
    virtual StmtType stype() { return STMT_CASE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
    Expr* Cond;
    StmtList2 Stmts;
};


class DefaultStmt : public Stmt {
public:
    DefaultStmt(SourceLocation Loc_, StmtList2& Stmts_);
    virtual ~DefaultStmt();
    virtual StmtType stype() { return STMT_DEFAULT; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
    StmtList2 Stmts;
};


class BreakStmt : public Stmt {
public:
    BreakStmt(SourceLocation Loc_);
    virtual ~BreakStmt();
    virtual StmtType stype() { return STMT_BREAK; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
};


class ContinueStmt : public Stmt {
public:
    ContinueStmt(SourceLocation Loc_);
    virtual ~ContinueStmt();
    virtual StmtType stype() { return STMT_CONTINUE; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Loc;
};


class LabelStmt : public Stmt {
public:
    LabelStmt(const char* name_, SourceLocation Loc_, Stmt* subStmt_);
    virtual ~LabelStmt();
    virtual StmtType stype() { return STMT_LABEL; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    std::string name;
    SourceLocation Loc;
    Stmt* subStmt;
};


class GotoStmt : public Stmt {
public:
    GotoStmt(const char* name_, SourceLocation GotoLoc_, SourceLocation LabelLoc_);
    virtual ~GotoStmt();
    virtual StmtType stype() { return STMT_GOTO; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    std::string name;
    SourceLocation GotoLoc;
    SourceLocation LabelLoc;
};


class CompoundStmt : public Stmt {
public:
    CompoundStmt(SourceLocation l, SourceLocation r, StmtList2& stmts_);
    virtual ~CompoundStmt();
    virtual StmtType stype() { return STMT_COMPOUND; }
    virtual void acceptS(StmtVisitor& v);

    virtual void print(int indent, StringBuilder& buffer);
    virtual void generateC(int indent, StringBuilder& buffer);
    virtual llvm::Value* codeGen(CodeGenContext& context);
private:
    SourceLocation Left;
    SourceLocation Right;
    StmtList2 Stmts;
};


class StmtVisitor {
public:
    virtual ~StmtVisitor() {}
    virtual void visit(C2::Stmt&) { assert(0); }    // add subclass below
    virtual void visit(ReturnStmt&) {}
    virtual void visit(IfStmt&) {}
    virtual void visit(WhileStmt&) {}
    virtual void visit(DoStmt&) {}
    virtual void visit(SwitchStmt&) {}
    virtual void visit(CaseStmt&) {}
    virtual void visit(DefaultStmt&) {}
    virtual void visit(BreakStmt&) {}
    virtual void visit(ContinueStmt&) {}
    virtual void visit(LabelStmt&) {}
    virtual void visit(GotoStmt&) {}
    virtual void visit(CompoundStmt&) {}
};

#define STMT_VISITOR_ACCEPT(a) void a::acceptS(StmtVisitor& v) { v.visit(*this); }

template <class T> class StmtTypeCaster : public StmtVisitor {
public:
    virtual void visit(T& node_) {
        node = &node_;
    }
    static T* getType(C2::Stmt& node_) {
        StmtTypeCaster<T> visitor(node_);
        return visitor.node;
    }
private:
    StmtTypeCaster(C2::Stmt& n) : node(0) {
        n.acceptS(*this);
    }
    T* node;
};

}

#endif

