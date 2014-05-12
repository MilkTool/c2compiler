/* Copyright 2013,2014 Bas van den Berg
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

// TEMP
#include <stdio.h>

#include "Builder/DepVisitor.h"
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/Expr.h"

using namespace C2;

void DepVisitor::run() {
    fprintf(stderr, "CHECKING %s\n", decl->getName().c_str());
    checkDecl(decl);
}

void DepVisitor::checkDecl(const Decl* D) {
    switch (D->getKind()) {
    case DECL_FUNC:
        checkFunctionDecl(cast<FunctionDecl>(D));
        break;
    case DECL_VAR:
        checkVarDecl(cast<VarDecl>(D));
        break;
    case DECL_ENUMVALUE:
        assert(0);
        break;
    case DECL_ALIASTYPE:
        checkType(cast<AliasTypeDecl>(D)->getRefType());
        break;
    case DECL_STRUCTTYPE:
        {
            const StructTypeDecl* S = cast<StructTypeDecl>(D);
            for (unsigned i=0; i<S->numMembers(); i++) {
                checkDecl(S->getMember(i));
            }
            break;
        }
    case DECL_ENUMTYPE:
        {
            const EnumTypeDecl* E = cast<EnumTypeDecl>(D);
            checkType(E->getType());
            // TODO check constant init values
            break;
        }
    case DECL_FUNCTIONTYPE:
        // TODO
        break;
    case DECL_ARRAYVALUE:
        // TODO
        break;
    case DECL_USE:
        break;
    }
}

void DepVisitor::checkFunctionDecl(const FunctionDecl* F) {
    // return Type
    checkType(F->getReturnType());

    // args
    for (unsigned i=0; i<F->numArgs(); i++) {
        checkVarDecl(F->getArg(i));
    }

    // check body
    checkCompoundStmt(F->getBody());
}

void DepVisitor::checkVarDecl(const VarDecl* V) {
    checkType(V->getType());

    if (V->getInitValue()) checkExpr(V->getInitValue());
}

void DepVisitor::checkType(QualType Q) {
    const Type* T = Q.getTypePtr();
    switch (T->getTypeClass()) {
    case TC_BUILTIN:
        return;
    case TC_POINTER:
        checkType(cast<PointerType>(T)->getPointeeType());
        break;
    case TC_ARRAY:
        {
            const ArrayType* A = cast<ArrayType>(T);
            checkType(A->getElementType());
            if (A->getSizeExpr()) checkExpr(A->getSizeExpr());
            break;
        }
    case TC_UNRESOLVED:
        addDep(cast<UnresolvedType>(T)->getDecl());
        break;
    case TC_ALIAS:
        addDep(cast<AliasType>(T)->getDecl());
        break;
    case TC_STRUCT:
        addDep(cast<StructType>(T)->getDecl());
        break;
    case TC_ENUM:
        addDep(cast<EnumType>(T)->getDecl());
        break;
    case TC_FUNCTION:
        // TODO fix for FunctionTypeDecl
        addDep(cast<FunctionType>(T)->getDecl());
        break;
    case TC_PACKAGE:
        assert(0);
        break;
    }
}

void DepVisitor::checkStmt(const Stmt* S) {
    switch (S->getKind()) {
    case STMT_RETURN:
        {
            const ReturnStmt* R = cast<ReturnStmt>(S);
            if (R->getExpr()) checkExpr(R->getExpr());
            break;
        }
    case STMT_EXPR:
        checkExpr(cast<Expr>(S));
        break;
    case STMT_IF:
        {
            const IfStmt* I = cast<IfStmt>(S);
            checkExpr(I->getCond());
            checkStmt(I->getThen());
            if (I->getElse()) checkStmt(I->getElse());
            break;
        }
    case STMT_WHILE:
    case STMT_DO:
    case STMT_FOR:
    case STMT_SWITCH:
    case STMT_CASE:
    case STMT_DEFAULT:
        // TODO
        break;
    case STMT_BREAK:
    case STMT_CONTINUE:
    case STMT_LABEL:
    case STMT_GOTO:
        break;
    case STMT_COMPOUND:
        checkCompoundStmt(cast<CompoundStmt>(S));
        break;
    }
}

void DepVisitor::checkCompoundStmt(const CompoundStmt* C) {
    const StmtList& stmts = C->getStmts();
    for (unsigned i=0; i<stmts.size(); i++) {
        checkStmt(stmts[i]);
    }
}


void DepVisitor::checkExpr(const Expr* E) {
    switch (E->getKind()) {
    case EXPR_INTEGER_LITERAL:
    case EXPR_FLOAT_LITERAL:
    case EXPR_BOOL_LITERAL:
    case EXPR_CHAR_LITERAL:
    case EXPR_STRING_LITERAL:
    case EXPR_NIL:
        break;
    case EXPR_IDENTIFIER:
        addDep(cast<IdentifierExpr>(E)->getDecl());
        break;
    case EXPR_TYPE:
        assert(0);
        break;
    case EXPR_CALL:
        {
            const CallExpr* C = cast<CallExpr>(E);
            checkExpr(C->getFn());
            for (unsigned i=0; i<C->numArgs(); i++) {
                checkExpr(C->getArg(i));
            }
            break;
        }
    case EXPR_INITLIST:
        // TODO
        break;
    case EXPR_DECL:
        checkVarDecl(cast<DeclExpr>(E)->getDecl());
        break;
    case EXPR_BINOP:
        {
            const BinaryOperator* B = cast<BinaryOperator>(E);
            checkExpr(B->getLHS());
            checkExpr(B->getRHS());
            break;
        }
    case EXPR_CONDOP:
    case EXPR_UNARYOP:
    case EXPR_BUILTIN:
    case EXPR_ARRAYSUBSCRIPT:
        // TODO
        break;
    case EXPR_MEMBER:
        {
            const MemberExpr* M = cast<MemberExpr>(E);
            if (M->isPkgPrefix()) {
                addDep(M->getDecl());
            } else {
                checkExpr(M->getBase());
            }
            break;
        }
    case EXPR_PAREN:
        checkExpr(cast<ParenExpr>(E)->getExpr());
        break;
    }
}

void DepVisitor::addDep(const Decl* D) {
    assert(D);
    if (decl == D) return;
    // Skip local VarDecls
    if (const VarDecl* V = dyncast<VarDecl>(D)) {
        switch (V->getVarKind()) {
        case VARDECL_GLOBAL:
            break;
        case VARDECL_LOCAL:
        case VARDECL_PARAM:
            return;
        case VARDECL_MEMBER:
            break;
        }
    }
    // Convert EnumConstants -> EnumTypeDecl (via EnumType)
    if (const EnumConstantDecl* ECD = dyncast<EnumConstantDecl>(D)) {
        QualType Q = ECD->getType();
        const EnumType* T = cast<EnumType>(Q.getTypePtr());
        D = T->getDecl();
    }

    for (unsigned i=0; i<deps.size(); i++) {
        if (deps[i] == D) return;
    }
    deps.push_back(D);
    fprintf(stderr, "  %s -> %s\n", decl->getName().c_str(), D->getName().c_str());
}
