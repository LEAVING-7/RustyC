#pragma once
#include "Token.hpp"
#include "Types.hpp"
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
#define DEFINE_TYPES(...)                                                                                              \
  enum class Type { __VA_ARGS__ };                                                                                     \
  Type const mType;                                                                                                    \
  static auto ToString(Type ty)->char const*                                                                           \
  {                                                                                                                    \
    static constexpr char const* literals[]{VA_ARGS_TO_STRING(__VA_ARGS__)};                                           \
    return literals[static_cast<std::underlying_type_t<Type>>(ty)];                                                    \
  }

#define DEFINE_KINDS(...)                                                                                              \
  enum class Kind { __VA_ARGS__, SIZE };                                                                               \
  Kind const mKind;                                                                                                    \
  static auto ToString(Kind kind)->char const*                                                                         \
  {                                                                                                                    \
    static constexpr char const* literals[]{VA_ARGS_TO_STRING(__VA_ARGS__)};                                           \
    return literals[static_cast<std::underlying_type_t<Kind>>(kind)];                                                  \
  }

#define DEFINE_LOC                                                                                                     \
  char const* const mLoc;                                                                                              \
  auto getLoc() const->char const*                                                                                     \
  {                                                                                                                    \
    return mLoc;                                                                                                       \
  }

#define LOC_PARAM , char const* loc
#define LOC_INIT , mLoc(loc)

struct Expr;

struct Stmt : public Node {
public:
  DEFINE_TYPES(Let, Expression);
  IMPL_AS(Stmt);

public:
  Stmt(Type type) : mType(type) {}
  ~Stmt() override = default;
};

struct BlockExpr;

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
  std::unique_ptr<TypeBase> mExpectType;

  DEFINE_LOC
public:
  LetStmt(std::string const& name, std::unique_ptr<TypeBase> expectType, std::unique_ptr<Expr> expr LOC_PARAM)
      : Stmt(Stmt::Type::Let), mName(name), mExpr(std::move(expr)), mExpectType(std::move(expectType)) LOC_INIT
  {
  }
  ~LetStmt() override final = default;
};

//===----------------------------------------------------------------------===//
// Item
//===----------------------------------------------------------------------===//

struct Item : Node {
public:
  DEFINE_KINDS(Module, ExternCrate, UseDeclaration, Function, TypeAlias, Struct, Enumeration, Union, ConstantItem,
               StaticItem, Trait, Implementation, ExternBlock);
  IMPL_AS(Item);

public:
  Item(Kind kind) : mKind(kind) {}
  ~Item() override = default;
};

struct FunctionItem final : public Item {
public:
  std::string mName;
  std::vector<std::string> mParamNames;
  std::unique_ptr<FunctionType> mFnType;
  std::unique_ptr<BlockExpr> mBody; // if null, it's a declaration

  DEFINE_LOC
public:
  FunctionItem(std::string const& name, std::vector<std::string>&& argNames, std::unique_ptr<FunctionType> fnType,
               std::unique_ptr<BlockExpr> body LOC_PARAM)
      : Item(Item::Kind::Function), mName(name), mParamNames(std::move(argNames)), mFnType(std::move(fnType)),
        mBody(std::move(body)) LOC_INIT
  {
  }
  ~FunctionItem() override = default;
  bool isDeclaration() const { return mBody == nullptr; }
};

struct ExternalBlockItem : public Item {
public:
  std::string mABI;
  std::vector<std::unique_ptr<FunctionItem>> mItems; // TODO: static item or function item

public:
  ExternalBlockItem(std::string const& abi, std::vector<std::unique_ptr<FunctionItem>> items)
      : Item(Item::Kind::ExternBlock), mABI(abi), mItems(std::move(items))
  {
  }
  ~ExternalBlockItem() override = default;
};

struct Crate final {
public:
  std::vector<std::unique_ptr<Item>> mItems;

public:
  Crate(std::vector<std::unique_ptr<Item>> items) : mItems(std::move(items)) {}
  ~Crate() = default;
};

//===----------------------------------------------------------------------===//
// Expr
//===----------------------------------------------------------------------===//

struct Expr : Node {
public:
  DEFINE_TYPES(WithBlock, WithoutBlock);
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
  DEFINE_TYPES(Literal, Grouped, Operator, Call, Return);
  IMPL_AS(ExprWithoutBlock);

public:
  ExprWithoutBlock(Type type) : Expr(Expr::Type::WithoutBlock), mType(type) {}
  ~ExprWithoutBlock() override = default;
};

struct GroupedExpr final : ExprWithoutBlock {
public:
  std::unique_ptr<Expr> mExpr;
  DEFINE_LOC
public:
  GroupedExpr(std::unique_ptr<Expr> expr LOC_PARAM)
      : ExprWithoutBlock(ExprWithoutBlock::Type::Grouped), mExpr(std::move(expr)) LOC_INIT
  {
  }
  ~GroupedExpr() override final = default;
};

struct LiteralExpr final : ExprWithoutBlock {
public:
  enum class Kind { Bool, I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, String, Identifier };
  using ValueType = std::variant<bool, i8, i16, i32, i64, u8, u16, u32, u64, float, double, std::string>;

public:
  Kind const mKind;
  ValueType mValue;
  DEFINE_LOC
public:
  LiteralExpr(Kind type, ValueType const& value LOC_PARAM)
      : ExprWithoutBlock(ExprWithoutBlock::Type::Literal), mValue(value), mKind(type) LOC_INIT
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
  DEFINE_TYPES(Unary, Binary);
  IMPL_AS(OperatorExpr);

public:
  OperatorExpr(OperatorExpr::Type type) : ExprWithoutBlock(ExprWithoutBlock::Type::Operator), mType(type) {}
  ~OperatorExpr() override = default;
};

struct BinaryExpr final : OperatorExpr {
public:
  DEFINE_KINDS(Add, Sub, Mul, Div, Rem, BitAnd, BitOr, BitXor, Shl, Shr, Eq, Ne, Gt, Lt, Ge, Le, Assignment);

  DEFINE_LOC
public:
  std::unique_ptr<Expr> mLeft;
  std::unique_ptr<Expr> mRight;

public:
  BinaryExpr(BinaryExpr::Kind kind, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right LOC_PARAM)
      : OperatorExpr(OperatorExpr::Type::Binary), mKind(kind), mLeft(std::move(left)), mRight(std::move(right)) LOC_INIT
  {
  }
  ~BinaryExpr() override = default;

  static auto BindingPower(BinaryExpr::Kind kind) -> std::tuple<i32, i32>;
  static auto MapKind(TokenKind tok) -> BinaryExpr::Kind;
};

struct UnaryExpr final : OperatorExpr {
public:
  DEFINE_KINDS(Neg, Not);

public:
  std::unique_ptr<Expr> mRight;

public:
  UnaryExpr(UnaryExpr::Kind kind, std::unique_ptr<Expr> right)
      : OperatorExpr(OperatorExpr::Type::Unary), mKind(kind), mRight(std::move(right))
  {
  }
  ~UnaryExpr() override = default;

  static auto BindingPower(UnaryExpr::Kind kind) -> std::tuple<i32>;
  static auto MapKind(TokenKind tok) -> UnaryExpr::Kind;
};

struct CallExpr final : ExprWithoutBlock {
public:
  std::string mCallee;
  std::vector<std::unique_ptr<Expr>> mArgs;

  DEFINE_LOC
public:
  CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>>&& args LOC_PARAM)
      : ExprWithoutBlock(ExprWithoutBlock::Type::Call), mCallee(std::move(callee)), mArgs(std::move(args)) LOC_INIT
  {
  }
  ~CallExpr() override final = default;
};

struct ReturnExpr final : ExprWithoutBlock {
public:
  std::unique_ptr<Expr> mExpr;

  DEFINE_LOC
public:
  ReturnExpr(std::unique_ptr<Expr> expr LOC_PARAM)
      : ExprWithoutBlock(ExprWithoutBlock::Type::Return), mExpr(std::move(expr)) LOC_INIT
  {
  }
  ~ReturnExpr() override final = default;
};

//===----------------------------------------------------------------------===//
// ExprWithBlock
//===----------------------------------------------------------------------===//

struct ExprWithBlock : Expr {
public:
  DEFINE_TYPES(Block, Loop, If, IfLet, Match)
  IMPL_AS(ExprWithBlock);

public:
  ExprWithBlock(Type type) : Expr(Expr::Type::WithBlock), mType(type) {}
  ~ExprWithBlock() override = default;
};

struct BlockExpr final : ExprWithBlock {
public:
  std::vector<std::unique_ptr<Stmt>> mStmts;
  std::vector<std::unique_ptr<Item>> mItems;
  std::unique_ptr<Expr> mReturn; // nullptr for none

public:
  BlockExpr(std::vector<std::unique_ptr<Stmt>>&& stmts, std::vector<std::unique_ptr<Item>>&& items,
            std::unique_ptr<Expr> ret)
      : ExprWithBlock(ExprWithBlock::Type::Block), mStmts(std::move(stmts)), mReturn(std::move(ret)),
        mItems(std::move(items))
  {
  }
  ~BlockExpr() override final = default;
};

struct IfExpr final : ExprWithBlock {
public:
  std::unique_ptr<Expr> mCond;
  std::unique_ptr<BlockExpr> mThen;
  std::unique_ptr<ExprWithBlock> mElse; // nullptr for none, {Block, If, IfLet} required

  DEFINE_LOC
public:
  IfExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> _if, std::unique_ptr<ExprWithBlock> _else LOC_PARAM)
      : ExprWithBlock(ExprWithBlock::Type::If), mCond(std::move(cond)), mThen(std::move(_if)),
        mElse(std::move(_else)) LOC_INIT
  {
  }
  ~IfExpr() override final = default;
};

struct LoopExpr : ExprWithBlock {
public:
  DEFINE_TYPES(InfiniteLoop, PredicateLoop);
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

  DEFINE_LOC
public:
  PredicateLoopExpr(std::unique_ptr<Expr> cond, std::unique_ptr<BlockExpr> expr LOC_PARAM)
      : LoopExpr(LoopExpr::Type::PredicateLoop), mCond(std::move(cond)), mExpr(std::move(expr)) LOC_INIT
  {
  }
  ~PredicateLoopExpr() override final = default;
};

struct StmtVisitor {
  virtual void visitStmt(Stmt* stmt);
  virtual void visit(ExprStmt* stmt) = 0;
  virtual void visit(LetStmt* stmt) = 0;
  virtual void visitItem(Item* item);
  virtual void visit(FunctionItem* item) = 0;
};

struct ExprVisitor {
  virtual void visitExpr(Expr* expr);

  virtual void visitExprWithoutBlock(ExprWithoutBlock* expr);
  virtual void visitOperatorExpr(OperatorExpr* expr);
  virtual void visit(LiteralExpr* expr) = 0;
  virtual void visit(GroupedExpr* expr) = 0;
  virtual void visit(UnaryExpr* expr) = 0;
  virtual void visit(BinaryExpr* expr) = 0;

  virtual void visitExprWithBlock(ExprWithBlock* expr);
  virtual void visitLoopExpr(LoopExpr* expr);
  virtual void visit(BlockExpr* expr) = 0;
  virtual void visit(IfExpr* expr) = 0;
  virtual void visit(InfiniteLoopExpr* expr) = 0;
  virtual void visit(PredicateLoopExpr* expr) = 0;
  virtual void visit(CallExpr* expr) = 0;
  virtual void visit(ReturnExpr* expr) = 0;
};

struct StringifyStmt;
struct StringifyExpr final : ExprVisitor {
  std::string& str;
  StringifyExpr(std::string& str, StringifyStmt& stmtVisitor) : str(str), mStmtVisitor(stmtVisitor) {}

private:
  StringifyStmt& mStmtVisitor;

  void visit(LiteralExpr* expr) override;
  void visit(GroupedExpr* expr) override;

  void visit(UnaryExpr* expr) override;
  void visit(BinaryExpr* expr) override;

  void visit(BlockExpr* expr) override;
  void visit(IfExpr* expr) override;
  void visit(InfiniteLoopExpr* expr) override;
  void visit(PredicateLoopExpr* expr) override;

  void visit(CallExpr* expr) override;
  void visit(ReturnExpr* expr) override;
};

struct StringifyStmt : StmtVisitor {
  std::string& str;
  StringifyStmt(std::string& str) : mExprVisitor(str, *this), str(str) {}

private:
  StringifyExpr mExprVisitor;

  void visit(ExprStmt* stmt) override;
  void visit(LetStmt* stmt) override;
  void visit(FunctionItem* item) override;
};

#undef DEFINE_TYPES
#undef DEFINE_KINDS