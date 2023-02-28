#pragma once
#include "Token.hpp"
#include "common.hpp"

#define IMPL_AS(Target)                                                                                                \
  template <typename T>                                                                                                \
    requires std::derived_from<T, Target>                                                                              \
  auto as()->T*                                                                                                        \
  {                                                                                                                    \
    return static_cast<T*>(this);                                                                                      \
  }

struct Node {
  Node() = default;
  virtual ~Node() = default;
};

// ============================== Stmt ==============================
struct Expr;

struct Stmt : public Node {
public:
  enum class Type { Item, Let, Expression };

public:
  Type mType;

public:
  Stmt(Type type) : mType(type) {}
};

struct ExpressionStmt final : Stmt {
public:
};

struct LetStmt final : Stmt {
public:
  std::string mName;
  std::unique_ptr<Expr> mExpr;

public:
  LetStmt(std::string const& name, std::unique_ptr<Expr> expr)
      : Stmt(Stmt::Type::Let), mName(name), mExpr(std::move(expr))
  {
  }
};

// ============================== Expr ==============================

struct Expr : public Node {
public:
  enum class Type { WithBlock, WithoutBlock };

public:
  Type mType;

public:
  Expr(Expr::Type type) : mType(type) {}
  ~Expr() override = default;

  IMPL_AS(Expr)
};

struct ExprWithoutBlock : Expr {
public:
  enum class Type { Literal, Grouped, Operator };

public:
  Type mType;

public:
  ExprWithoutBlock(Type type) : Expr(Expr::Type::WithoutBlock), mType(type) {}
  ~ExprWithoutBlock() override = default;
};

struct ExprWithBlock : Expr {
public:
  enum class Type { Block, Loop, If, IfLet, Match };

public:
  Type mType;

public:
  ExprWithBlock(Type type) : Expr(Expr::Type::WithBlock), mType(type) {}
  ~ExprWithBlock() override = default;
};

struct GroupedExpr final : ExprWithoutBlock {
public:
  std::unique_ptr<Expr> mExpr;

public:
  GroupedExpr(std::unique_ptr<Expr> expr) : ExprWithoutBlock(ExprWithoutBlock::Type::Grouped), mExpr(std::move(expr)) {}
  ~GroupedExpr() override final = default;
};

struct LiteralExpr final : ExprWithoutBlock {
public:
  enum class Kind { Bool, I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, String };
  using ValueType = std::variant<bool, i8, i16, i32, i64, u8, u16, u32, u64, float, double, std::string>;

public:
  Kind mKind;
  ValueType mValue;

public:
  LiteralExpr(Kind type, ValueType const& value)
      : ExprWithoutBlock(ExprWithoutBlock::Type::Literal), mValue(value), mKind(type)
  {
  }
  ~LiteralExpr() override final = default;
};

template <typename T>
concept TokenMap = requires(T expr) {
                     T::Kind::SIZE;
                     typename T::Kind;
                     {
                       T::MapOp(std::declval<TokenKind>())
                       } -> std::convertible_to<typename T::Kind>;
                     {
                       T::GetPrec(std::declval<typename T::Kind>())
                       } -> std::convertible_to<i32>;
                   };

struct OperatorExpr : ExprWithoutBlock {
public:
  enum class Type { ArithmeticOrLogical, Negation, Comparison };

public:
  Type mType;

public:
  OperatorExpr(OperatorExpr::Type type) : ExprWithoutBlock(ExprWithoutBlock::Type::Operator), mType(type) {}
  ~OperatorExpr() override = default;
  IMPL_AS(OperatorExpr)
};

struct ComparisonExpr final : OperatorExpr {
public:
  enum class Kind { Eq, Ne, Gt, Lt, Ge, Le, SIZE };

public:
  Kind mKind;
  std::unique_ptr<Expr> mLeft;
  std::unique_ptr<Expr> mRight;

public:
  ComparisonExpr(Kind kind, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : OperatorExpr(OperatorExpr::Type::Comparison), mKind(kind), mLeft(std::move(left)), mRight(std::move(right))
  {
  }
  ~ComparisonExpr() override final = default;

  static auto MapOp(TokenKind token) -> Kind;
  static auto GetPrec(Kind type) -> i32;

  static_assert(TokenMap<ComparisonExpr>);
};

struct NegationExpr final : OperatorExpr {
public:
  enum class Kind { Neg, Not, SIZE };
  static std::array<i32, 2> sPrecedence;

public:
  Kind mKind;
  std::unique_ptr<Expr> mRight;

public:
  NegationExpr(Kind kind, std::unique_ptr<Expr> expr)
      : OperatorExpr(OperatorExpr::Type::Negation), mKind(kind), mRight(std::move(expr))
  {
  }
  ~NegationExpr() override final = default;

  static auto MapOp(TokenKind token) -> Kind;
  static auto GetPrec(Kind type) -> i32;
};

struct ArithmeticOrLogicalExpr final : OperatorExpr {

public:
  enum class Kind { Add, Sub, Mul, Div, Rem, BitAnd, BitOr, BitXor, Shl, Shr, SIZE };
  static std::map<Kind, i32> sPrecedence;

public:
  Kind mKind;
  std::unique_ptr<Expr> mLeft;
  std::unique_ptr<Expr> mRight;

public:
  ArithmeticOrLogicalExpr(ArithmeticOrLogicalExpr::Kind type, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : OperatorExpr(OperatorExpr::Type::ArithmeticOrLogical), mKind(type), mLeft(std::move(left)),
        mRight(std::move(right))
  {
  }
  ~ArithmeticOrLogicalExpr() override final = default;

  static auto MapOp(TokenKind token) -> Kind;
  static auto GetPrec(Kind type) -> i32;

  static_assert(TokenMap<ArithmeticOrLogicalExpr>);
};

struct ExprVisitor {
  virtual void expr(Expr* expr);
  virtual void exprWithoutBlock(ExprWithoutBlock* expr);
  virtual void operatorExpr(OperatorExpr* expr);
  virtual void literalExpr(LiteralExpr* expr) = 0;
  virtual void groupedExpr(GroupedExpr* expr) = 0;
  virtual void comparisonExpr(ComparisonExpr* expr) = 0;
  virtual void negationExpr(NegationExpr* expr) = 0;
  virtual void arithmeticOrLogicalExpr(ArithmeticOrLogicalExpr* expr) = 0;

  virtual void exprWithBlock(ExprWithBlock* expr);
};
