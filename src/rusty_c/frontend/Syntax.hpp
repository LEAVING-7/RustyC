#pragma once
#include "Token.hpp"
#include "common.hpp"

#define IMPL_AS(Target)                                                                                                \
  template <typename T>                                                                                                \
    requires std::derived_from<T, Target>                                                                              \
  auto as()->T*                                                                                                        \
  {                                                                                                                    \
    return static_cast<T*>(this);                                                                                      \
  }                                                                                                                    \
  Target() = delete;
struct Node {
  Node() = default;
  virtual ~Node() = default;
};

//===----------------------------------------------------------------------===//
// Stmt
//===----------------------------------------------------------------------===//

// make life easier
#define DEFINE_IDS(...)                                                                                                \
  enum class Type { __VA_ARGS__ };                                                                                     \
  Type const mType;

struct Expr;

struct Stmt : public Node {
public:
  DEFINE_IDS(Item, Let, Expression);
  IMPL_AS(Stmt);

public:
  Stmt(Type type) : mType(type) {}
  ~Stmt() override = default;
};

struct ExprStmt final : public Stmt {
public:
  std::unique_ptr<Expr> mExpr;

public:
  ExprStmt(std::unique_ptr<Expr> expr) : Stmt(Stmt::Type::Expression), mExpr(std::move(expr)) {}
  ~ExprStmt() override final = default;
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
  ~LetStmt() override final = default;
};

struct StmtVisitor {
  virtual void visitStmt(Stmt* stmt);
  virtual void visit(ExprStmt* stmt) = 0;
  virtual void visit(LetStmt* stmt) = 0;
};

//===----------------------------------------------------------------------===//
// Expr
//===----------------------------------------------------------------------===//

struct Expr : Node {
public:
  DEFINE_IDS(WithBlock, WithoutBlock);
  IMPL_AS(Expr)

public:
  Expr(Expr::Type type) : mType(type) {}
  ~Expr() override = default;
};

//===----------------------------------------------------------------------===//
// ExprWithoutBlock
//===----------------------------------------------------------------------===//

struct ExprWithoutBlock : Expr {
public:
  DEFINE_IDS(Literal, Grouped, Operator);
  IMPL_AS(ExprWithoutBlock);

public:
  ExprWithoutBlock(Type type) : Expr(Expr::Type::WithoutBlock), mType(type) {}
  ~ExprWithoutBlock() override = default;
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
  Kind const mKind;
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
  DEFINE_IDS(ArithmeticOrLogical, Negation, Comparison, Assignment);
  IMPL_AS(OperatorExpr)

public:
  OperatorExpr(OperatorExpr::Type type) : ExprWithoutBlock(ExprWithoutBlock::Type::Operator), mType(type) {}
  ~OperatorExpr() override = default;
};

struct ComparisonExpr final : OperatorExpr {
public:
  enum class Kind { Eq, Ne, Gt, Lt, Ge, Le, SIZE };

public:
  Kind const mKind;
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
  Kind const mKind;
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
  Kind const mKind;
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

struct AssignmentExpr final : OperatorExpr {
public:
  std::unique_ptr<Expr> mLeft;
  std::unique_ptr<Expr> mRight;

public:
  AssignmentExpr(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
      : OperatorExpr(OperatorExpr::Type::Assignment), mLeft(std::move(left)), mRight(std::move(right))
  {
  }
  ~AssignmentExpr() override final = default;

  static auto GetPrec() -> i32 { return 2; }
};

//===----------------------------------------------------------------------===//
// ExprWithBlock
//===----------------------------------------------------------------------===//

struct ExprWithBlock : Expr {
public:
  DEFINE_IDS(Block, Loop, If, IfLet, Match)
  IMPL_AS(ExprWithBlock);

public:
  ExprWithBlock(Type type) : Expr(Expr::Type::WithBlock), mType(type) {}
  ~ExprWithBlock() override = default;
};

struct BlockExpr final : ExprWithBlock {
public:
  std::vector<std::unique_ptr<Stmt>> mStmts;
  std::unique_ptr<ExprWithoutBlock> mReturn; // nullptr for none

public:
  BlockExpr(std::vector<std::unique_ptr<Stmt>>&& stmts, std::unique_ptr<ExprWithoutBlock> ret)
      : ExprWithBlock(ExprWithBlock::Type::Block), mStmts(std::move(stmts)), mReturn(std::move(ret))
  {
  }
  ~BlockExpr() override final = default;
};

struct IfExpr final : ExprWithBlock {
public:
  std::unique_ptr<Expr> mCond;
  std::unique_ptr<BlockExpr> mIf;
  std::unique_ptr<ExprWithBlock> mElse; // nullptr for none, {Block, If, IfLet} required

public:
  IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> _if, std::unique_ptr<ExprWithBlock> _else)
      : ExprWithBlock(ExprWithBlock::Type::If), mCond(std::move(cond)), mIf(std::move(_if)), mElse(std::move(_else))
  {
  }
  ~IfExpr() override final = default;
};

struct LoopExpr : ExprWithBlock {
public:
  DEFINE_IDS(InfiniteLoop, PredicateLoop);
  IMPL_AS(LoopExpr);

public:
  LoopExpr(Type type) : ExprWithBlock(ExprWithBlock::Type::Loop), mType(type) {}
  ~LoopExpr() override = default;
};

struct InfiniteLoopExpr final : LoopExpr {
public:
  std::unique_ptr<BlockExpr> mExpr;

public:
  InfiniteLoopExpr(std::unique_ptr<BlockExpr> expr) : LoopExpr(LoopExpr::Type::InfiniteLoop) {}
  ~InfiniteLoopExpr() override final = default;
};

struct PredicateLoopExpr final : LoopExpr {
public:
  std::unique_ptr<Expr> mCond;
  std::unique_ptr<BlockExpr> mExpr;

public:
  PredicateLoopExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> expr)
      : LoopExpr(LoopExpr::Type::PredicateLoop)
  {
  }
  ~PredicateLoopExpr() override final = default;
};

struct ExprVisitor {
  virtual void visitExpr(Expr* expr);

  virtual void visitExprWithoutBlock(ExprWithoutBlock* expr);
  virtual void visitOperatorExpr(OperatorExpr* expr);
  virtual void visit(LiteralExpr* expr) = 0;
  virtual void visit(GroupedExpr* expr) = 0;
  virtual void visit(ComparisonExpr* expr) = 0;
  virtual void visit(NegationExpr* expr) = 0;
  virtual void visit(ArithmeticOrLogicalExpr* expr) = 0;
  virtual void visit(AssignmentExpr* expr) = 0;

  virtual void visitExprWithBlock(ExprWithBlock* expr);
  virtual void visitLoopExpr(LoopExpr* expr);
  virtual void visit(BlockExpr* expr) = 0;
  virtual void visit(IfExpr* expr) = 0;
  virtual void visit(InfiniteLoopExpr* expr) = 0;
  virtual void visit(PredicateLoopExpr* expr) = 0;
};

struct StringifyStmt;
struct StringifyExpr : ExprVisitor {
  std::string& str;
  StringifyExpr(std::string& str, StringifyStmt& stmtVisitor) : str(str), mStmtVisitor(stmtVisitor) {}

private:
  StringifyStmt& mStmtVisitor;

  void visit(LiteralExpr* expr) override;
  void visit(GroupedExpr* expr) override;
  void visit(ComparisonExpr* expr) override;
  void visit(NegationExpr* expr) override;
  void visit(ArithmeticOrLogicalExpr* expr) override;
  void visit(AssignmentExpr* expr) override;

  void visit(BlockExpr* expr) override;
  void visit(IfExpr* expr) override;
  void visit(InfiniteLoopExpr* expr) override;
  void visit(PredicateLoopExpr* expr) override;
};
struct StringifyStmt : StmtVisitor {
  std::string& str;
  StringifyStmt(std::string& str) : mExprVisitor(str, *this), str(str) {}

private:
  StringifyExpr mExprVisitor;

  void visit(ExprStmt* stmt) override { mExprVisitor.visitExpr(stmt->mExpr.get()); };
  void visit(LetStmt* stmt) override
  {
    str += "let ";
    str += stmt->mName;
    mExprVisitor.visitExpr(stmt->mExpr.get());
  };
};
