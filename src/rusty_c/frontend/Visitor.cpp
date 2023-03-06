#include "Visitor.hpp"

struct ToStringVisitor : public Visitor<void> {
  std::string mResult;

  void walk(Crate* crate)
  {
    for (auto& item : crate->mItems) {
      walkItem(item.get());
    }
  }
  void walk(BinaryExpr* expr)
  {
    walkExpr(expr->mLeft.get());
    mResult += ' ';
    mResult += BinaryExpr::ToString(expr->mKind);
    mResult += ' ';
    walkExpr(expr->mRight.get());
  }
  void walk(BlockExpr* expr)
  {
    mResult += "{";
    for (auto& item : expr->mItems) {
      walkItem(item.get());
    }
    for (auto& stmt : expr->mStmts) {
      walkStmt(stmt.get());
    }
    if (expr->mReturn) {
      walkExpr(expr->mReturn.get());
    }
    mResult += "}";
  }
  void walk(CallExpr* expr)
  {
    mResult += expr->mCallee;
    mResult += "(";
    for (auto& arg : expr->mArgs) {
      walkExpr(arg.get());
    }
    mResult += ")";
  }
  void walk(GroupedExpr* expr)
  {
    mResult += '(';
    walkExpr(expr->mExpr.get());
    mResult += ')';
  }
  void walk(IfExpr* expr)
  {
    mResult += "if (";
    walkExpr(expr->mCond.get());
    mResult += ") ";
    walkExpr(expr->mThen.get());
    if (expr->mElse) {
      mResult += " else ";
      walkExpr(expr->mElse.get());
    }
  }
  void walk(InfiniteLoopExpr* expr)
  {
    mResult += "loop";
    walk(expr->mExpr.get());
  }
  void walk(LiteralExpr* expr)
  {
    mResult += std::visit(
        [expr]<typename T>(T const& v) {
          if constexpr (std::is_same_v<T, std::string>) {
            if (expr->mKind == LiteralExpr::Kind::String) {
              return '"' + v + '"';
            } else {
              return v;
            }
          } else {
            return std::to_string(v);
          }
        },
        expr->mValue);
  }
  void walk(PredicateLoopExpr* expr)
  {
    mResult += "while (";
    walkExpr(expr->mCond.get());
    mResult += ") ";
    walk(expr->mExpr.get());
  }
  void walk(ReturnExpr* expr)
  {
    mResult += "return";
    if (expr->mExpr) {
      mResult += ' ';
      walkExpr(expr->mExpr.get());
    }
  }
  void walk(UnaryExpr* expr)
  {
    mResult += UnaryExpr::ToString(expr->mKind);
    mResult += ' ';
    walkExpr(expr->mRight.get());
  }
  void walk(LetStmt* stmt)
  {
    mResult += "let ";
    mResult += stmt->mName;
    if (stmt->mExpectType) {
      mResult += ":";
      mResult += TypeToString(stmt->mExpectType.get());
    }
    mResult += "=";
    walkExpr(stmt->mExpr.get());
    mResult += ';';
  }
  void walk(Item* item)
  {
    switch (item->mKind) {
    case Item::Kind::Function:
      return walk(static_cast<FunctionItem*>(item));
    default:
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  void walk(FunctionItem* item)
  {
    mResult += "fn ";
    mResult += item->mName;
    mResult += '(';
    for (int i = 0; i < item->mParamNames.size(); ++i) {
      if (i != 0) {
        mResult += ",";
      }
      mResult += item->mParamNames[i];
      mResult += ":";
      mResult += TypeToString(item->mFnType->mParams[i].get());
    }
    mResult += ")->";
    mResult += TypeToString(item->mFnType->mRet.get());
    walk(item->mBody.get());
  }
  void walk(ExprStmt* stmt)
  {
    walkExpr(stmt->mExpr.get());
    mResult += ';';
  }
};

auto CrateToString(Crate* crate) -> std::string
{
  ToStringVisitor visitor;
  visitor.walk(crate);
  return visitor.mResult;
}
