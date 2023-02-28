#include "frontend/Lexer.hpp"
#include "frontend/Parser.hpp"
#include <iostream>

struct PrintExpr : ExprVisitor {
  std::string mString;
  void literalExpr(LiteralExpr* expr) override { mString += ToString(expr->mValue); }
  void groupedExpr(GroupedExpr* expr) override
  {
    mString += '(';
    this->expr(expr->mExpr.get());
    mString += ')';
  };
  void comparisonExpr(ComparisonExpr* expr) override
  {
    mString += '(';
    this->expr(expr->mLeft.get());
    switch (expr->mKind) {
    case ComparisonExpr::Kind::Eq:
      mString += "==";
      break;
    case ComparisonExpr::Kind::Ne:
      mString += "!=";
      break;
    case ComparisonExpr::Kind::Gt:
      mString += ">";
      break;
    case ComparisonExpr::Kind::Lt:
      mString += '<';
      break;
    case ComparisonExpr::Kind::Ge:
      mString += ">=";
      break;
    case ComparisonExpr::Kind::Le:
      mString += "<=";
      break;
    case ComparisonExpr::Kind::SIZE:
      break;
    }
    this->expr(expr->mRight.get());
    mString += ')';
  };
  void negationExpr(NegationExpr* expr) override
  {
    mString += '(';
    switch (expr->mKind) {
    case NegationExpr::Kind::Neg:
      mString += '-';
      break;
    case NegationExpr::Kind::Not:
      mString += '!';
      break;
    case NegationExpr::Kind::SIZE:
      break;
    }
    this->expr(expr->mRight.get());
    mString += ')';
  };
  void arithmeticOrLogicalExpr(ArithmeticOrLogicalExpr* expr) override
  {
    mString += '(';
    this->expr(expr->mLeft.get());
    switch (expr->mKind) {
    case ArithmeticOrLogicalExpr::Kind::Add:
      mString += '+';
      break;
    case ArithmeticOrLogicalExpr::Kind::Sub:
      mString += '-';
      break;
    case ArithmeticOrLogicalExpr::Kind::Mul:
      mString += '*';
      break;
    case ArithmeticOrLogicalExpr::Kind::Div:
      mString += '/';
      break;
    case ArithmeticOrLogicalExpr::Kind::Rem:
      mString += '%';
      break;
    case ArithmeticOrLogicalExpr::Kind::BitAnd:
      mString += '&';
      break;
    case ArithmeticOrLogicalExpr::Kind::BitOr:
      mString += '|';
      break;
    case ArithmeticOrLogicalExpr::Kind::BitXor:
      mString += '^';
      break;
    case ArithmeticOrLogicalExpr::Kind::Shl:
      mString += "<<";
      break;
    case ArithmeticOrLogicalExpr::Kind::Shr:
      mString += ">>";
      break;
    case ArithmeticOrLogicalExpr::Kind::SIZE:
      break;
    }
    this->expr(expr->mRight.get());
    mString += ')';
  };
};

int main(int argc, char* argv[])
{
  auto codes = R"(
  let x = a + (b) * -c * !-d + e;
)";
  std::vector tokens = Lexer{codes}.tokenize();
  auto parser = Parser{tokens};
  puts("shit");
  auto ast = parser.parseLetStmt();
  puts("shit");
  // PrintExpr(ast->mExpr.get());
  PrintExpr visitor{};
  visitor.expr(ast->mExpr.get());
  puts(visitor.mString.c_str());
  constexpr auto x = 4 - -!-6;
}