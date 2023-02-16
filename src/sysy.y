%code requires {
  #include <memory>
  #include <string>
  #include <vector>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <vector>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  char char_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST> > *vec_ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE VOID
%token LT GT LE GE EQ NEQ
%token AND OR
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp 
                AddExp RelExp EqExp LAndExp LOrExp Decl ConstDecl BType
                ConstDef ConstInitVal BlockItem LVal ConstExp VarDecl VarDef
                InitVal NonIfStmt OpenStmt ClosedStmt FuncFParam CompUnit
%type <int_val> Number
%type <char_val> UnaryOp
%type <vec_ast_val> ConstDefVec BlockItemVec VarDefVec FuncFParamVec FuncRParamVec
                    ConstExpVec ExpVec ConstInitValVec InitValVec

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnitRoot
  : CompUnit {
    auto comp_unit_root = make_unique<CompUnitRootAST>();
    comp_unit_root->compunit = unique_ptr<BaseAST>($1);
    ast = move(comp_unit_root);
  }
  ;

CompUnit
  : FuncDef {
    auto ast = new CompUnitAST();
    ast->func_def = unique_ptr<BaseAST>($1);
    ast->type = CompUnitType::CompUnit_SinFunc;
    $$ = ast;
  }
  | Decl {
    auto ast = new CompUnitAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->type = CompUnitType::CompUnit_SinDecl;
    $$ = ast;
  }
  | CompUnit FuncDef {
    auto ast = new CompUnitAST();
    ast->compunit = unique_ptr<BaseAST>($1);
    ast->func_def = unique_ptr<BaseAST>($2);
    ast->type = CompUnitType::CompUnit_MulFunc;
    $$ = ast;
  }
  | CompUnit Decl {
    auto ast = new CompUnitAST();
    ast->compunit = unique_ptr<BaseAST>($1);
    ast->decl = unique_ptr<BaseAST>($2);
    ast->type = CompUnitType::CompUnit_MulDecl;
    $$ = ast;
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->type = FuncDefType::FuncDef_Noparam;
    $$ = ast;
  }
  | FuncType IDENT '(' FuncFParamVec ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->funcfparamvec = unique_ptr<vector<unique_ptr<BaseAST> > >($4);
    ast->block = unique_ptr<BaseAST>($6);
    ast->type = FuncDefType::FuncDef_Param;
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = FuncTypeType::FuncType_INT;
    $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast->type = FuncTypeType::FuncType_VOID;
    $$ = ast;
  }
  ;

FuncFParamVec
  : FuncFParam ',' FuncFParamVec {
    auto funcfparam = unique_ptr<BaseAST>($1);
    auto funcfparamvec = $3;
    funcfparamvec->push_back(move(funcfparam));
    $$ = funcfparamvec;
  }
  | FuncFParam {
    auto funcfparam = unique_ptr<BaseAST>($1);
    auto funcfparamvec = new vector<unique_ptr<BaseAST> >();
    funcfparamvec->push_back(move(funcfparam));
    $$ = funcfparamvec;
  }
  ;

FuncFParam
  : BType IDENT {
    auto ast = new FuncFParamAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->type = FuncFParamType::FuncFParam_Int;
    $$ = ast;
  }
  | BType IDENT '[' ']' {
    auto ast = new FuncFParamAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->type = FuncFParamType::FuncFParam_Arr_Sin;
    $$ = ast;
  }
  | BType IDENT '[' ']' ConstExpVec {
    auto ast = new FuncFParamAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->constexpvec = unique_ptr<vector<unique_ptr<BaseAST> > >($5);
    ast->type = FuncFParamType::FuncFParam_Arr_Mul;
    $$ = ast;
  }
  ;

Stmt
  : OpenStmt {
    auto ast = new StmtAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->type = StmtType::Stmt_Open;
    $$ = ast;
  }
  | ClosedStmt {
    auto ast = new StmtAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->type = StmtType::Stmt_Closed;
    $$ = ast;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt {
    auto ast = new OpenStmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->type = OpenStmtType::Open_If;
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto ast = new OpenStmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->openstmt = unique_ptr<BaseAST>($7);
    ast->type = OpenStmtType::Open_Else;
    $$ = ast;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto ast = new OpenStmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->type = OpenStmtType::Open_While;
    $$ = ast;
  }
  ;

ClosedStmt
  : NonIfStmt {
    auto ast = new ClosedStmtAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->type = ClosedStmtType::Closed_NonIf;
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
    auto ast = new ClosedStmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->closedstmt = unique_ptr<BaseAST>($7);
    ast->type = ClosedStmtType::Closed_If;
    $$ = ast;
  }
  | WHILE '(' Exp ')' ClosedStmt {
    auto ast = new ClosedStmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    ast->type = ClosedStmtType::Closed_While;
    $$ = ast;
  }
  ;

NonIfStmt
  : RETURN Exp ';' {
    auto ast = new NonIfStmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    ast->type = NonIfStmtType::NonIf_Ret;
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new NonIfStmtAST();
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    ast->type = NonIfStmtType::NonIf_Lval;
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new NonIfStmtAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = NonIfStmtType::NonIf_Exp;
    $$ = ast;
  }
  | ';' {
    auto ast = new NonIfStmtAST();
    ast->exp = nullptr;
    ast->type = NonIfStmtType::NonIf_Null;
    $$ = ast;
  }
  | Block {
    auto ast = new NonIfStmtAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = NonIfStmtType::NonIf_Block;
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new NonIfStmtAST();
    ast->exp = nullptr;
    ast->type = NonIfStmtType::NonIf_Ret_Null;
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new NonIfStmtAST();
    ast->type = NonIfStmtType::NonIf_Break;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new NonIfStmtAST();
    ast->type = NonIfStmtType::NonIf_Continue;
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->exp = unique_ptr<BaseAST>($2);
    ast->type = PrimaryType::Primary_Exp;
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->lval = unique_ptr<BaseAST>($1);
    ast->type = PrimaryType::Primary_Lval;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->number = $1;
    ast->type = PrimaryType::Primary_Number;
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = UnaryType::Unary_PrimaryExp;
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->unaryop = $1;
    ast->exp = unique_ptr<BaseAST>($2);
    ast->type = UnaryType::Unary_UnaryExp;
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->ident = *unique_ptr<string>($1);
    ast->type = UnaryType::Unary_NoParam;
    $$ = ast;
  }
  | IDENT '(' FuncRParamVec ')' {
    auto ast = new UnaryExpAST();
    ast->ident = *unique_ptr<string>($1);
    ast->funcrparamvec = unique_ptr<vector<unique_ptr<BaseAST> > >($3);
    ast->type = UnaryType::Unary_Param;
    $$ = ast;
  }
  ;

FuncRParamVec
  : Exp {
    auto exp = unique_ptr<BaseAST>($1);
    auto funcrparamvec = new vector<unique_ptr<BaseAST> >();
    funcrparamvec->push_back(move(exp));
    $$ = funcrparamvec;
  }
  | Exp ',' FuncRParamVec {
    auto exp = unique_ptr<BaseAST>($1);
    auto funcrparamvec = $3;
    funcrparamvec->push_back(move(exp));
    $$ = funcrparamvec;
  }
  ;

UnaryOp
  : '+' {
    $$ = '+';
  }
  | '-' {
    $$ = '-';
  }
  | '!' {
    $$ = '!';
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<BaseAST>($1);
    ast->type = MulType::Mul_UnaryExp;
    $$ = ast;
  }
  | MulExp '*' UnaryExp {
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->type = MulType::Mul_MulExp;
    $$ = ast;
  }
  | MulExp '/' UnaryExp {
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->type = MulType::Mul_DivExp;
    $$ = ast;
  }
  | MulExp '%' UnaryExp {
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<BaseAST>($3);
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->type = MulType::Mul_ModExp;
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->type = AddType::Add_MulExp;
    $$ = ast;
  }
  | AddExp '+' MulExp {
    auto ast = new AddExpAST();
    ast->mulexp = unique_ptr<BaseAST>($3);
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->type = AddType::Add_AddExp;
    $$ = ast;
  }
  | AddExp '-' MulExp {
    auto ast = new AddExpAST();
    ast->mulexp = unique_ptr<BaseAST>($3);
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->type = AddType::Add_MinusExp;
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->type = RelType::Rel_AddExp;
    $$ = ast;
  }
  | RelExp LT AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->type = RelType::Rel_LTExp;
    $$ = ast;
  }
  | RelExp GT AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->type = RelType::Rel_GTExp;
    $$ = ast;
  }
  | RelExp LE AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->type = RelType::Rel_LEExp;
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($3);
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->type = RelType::Rel_GEExp;
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->type = EqType::Eq_RelExp;
    $$ = ast;
  }
  | EqExp EQ RelExp {
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<BaseAST>($3);
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->type = EqType::Eq_EQExp;
    $$ = ast;
  }
  | EqExp NEQ RelExp {
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<BaseAST>($3);
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->type = EqType::Eq_NEQExp;
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->type = LAndType::LAnd_EqExp;
    $$ = ast;
  }
  | LAndExp AND EqExp {
    auto ast = new LAndExpAST();
    ast->eqexp = unique_ptr<BaseAST>($3);
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->type = LAndType::LAnd_ANDExp;
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->type = LOrType::LOr_LAndExp;
    $$ = ast;
  }
  | LOrExp OR LAndExp {
    auto ast = new LOrExpAST();
    ast->landexp = unique_ptr<BaseAST>($3);
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->type = LOrType::LOr_ORExp;
    $$ = ast;
  }
  ;

Decl
  : VarDecl {
    auto ast = new DeclAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = DeclType::Decl_Var;
    $$ = ast;
  }
  | ConstDecl {
    auto ast = new DeclAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = DeclType::Decl_Const;
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDefVec ';' {
    auto ast = new ConstDeclAST();
    ast->btype = unique_ptr<BaseAST>($2);
    ast->constdefvec = unique_ptr<vector<unique_ptr<BaseAST> > >($3);
    $$ = ast;
  }
  ;

ConstDefVec
  : ConstDef {
    auto constdef = unique_ptr<BaseAST>($1);
    auto constdefvec = new vector<unique_ptr<BaseAST> >();
    constdefvec->push_back(move(constdef));
    $$ = constdefvec;
  }
  | ConstDef ',' ConstDefVec {
    auto constdef = unique_ptr<BaseAST>($1);
    auto constdefvec = $3;
    constdefvec->push_back(move(constdef));
    $$ = constdefvec;
  }
  ;

BType
  : INT {
    auto ast = new BTypeAST();
    ast->int_type = string("int");
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    ast->type = ConstDefType::ConstDef_Int;
    $$ = ast;
  }
  | IDENT ConstExpVec '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constexpvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->constinitval = unique_ptr<BaseAST>($4);
    ast->type = ConstDefType::ConstDef_Arr;
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    ast->type = ConstInitValType::ConstInitVal_Exp;
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new ConstInitValAST();
    ast->type = ConstInitValType::ConstInitVal_Null;
    $$ = ast;
  }
  | '{' ConstInitValVec '}' {
    auto ast = new ConstInitValAST();
    ast->constinitvalvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->type = ConstInitValType::ConstInitVal_Vec;
    $$ = ast;
  }
  ;

ConstInitValVec
  : ConstInitVal {
    auto constinitval = unique_ptr<BaseAST>($1);
    auto constinitvalvec = new vector<unique_ptr<BaseAST> >();
    constinitvalvec->push_back(move(constinitval));
    $$ = constinitvalvec;
  }
  | ConstInitVal ',' ConstInitValVec {
    auto constinitval = unique_ptr<BaseAST>($1);
    auto constinitvalvec = $3;
    constinitvalvec->push_back(move(constinitval));
    $$ = constinitvalvec;
  }
  ;

ConstExpVec
  : '[' ConstExp ']' {
    auto constexp = unique_ptr<BaseAST>($2);
    auto constexpvec = new vector<unique_ptr<BaseAST> >();
    constexpvec->push_back(move(constexp));
    $$ = constexpvec;
  }
  | '[' ConstExp ']' ConstExpVec {
    auto constexp = unique_ptr<BaseAST>($2);
    auto constexpvec = $4;
    constexpvec->push_back(move(constexp));
    $$ = constexpvec;
  }
  ;

Block
  : '{' BlockItemVec '}' {
    auto ast = new BlockAST();
    ast->blockitemvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->type = BlockType::Block_Items;
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new BlockAST();
    ast->type = BlockType::Block_Null;
    $$ = ast;
  }
  ;

BlockItemVec
  : BlockItem BlockItemVec {
    auto blockitem = unique_ptr<BaseAST>($1);
    auto blockitemvec = $2;
    blockitemvec->push_back(move(blockitem));
    $$ = blockitemvec;
  }
  | BlockItem {
    auto blockitem = unique_ptr<BaseAST>($1);
    auto blockitemvec = new vector<unique_ptr<BaseAST> >();
    blockitemvec->push_back(move(blockitem));
    $$ = blockitemvec;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->type = BlockItemType::BlockItem_Decl;
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->type = BlockItemType::BlockItem_Stmt;
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    ast->expvec = nullptr;
    ast->type = LValType::LVal_Int;
    $$ = ast;
  }
  | IDENT ExpVec {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    ast->expvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->type = LValType::LVal_Arr;
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

VarDecl
  : FuncType VarDefVec ';' {
    auto ast = new VarDeclAST();
    ast->functype = unique_ptr<BaseAST>($1);
    ast->vardefvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$ = ast;
  }
  ;

VarDefVec
  : VarDef {
    auto vardef = unique_ptr<BaseAST>($1);
    auto vardefvec = new vector<unique_ptr<BaseAST> >();
    vardefvec->push_back(move(vardef));
    $$ = vardefvec;
  }
  | VarDef ',' VarDefVec {
    auto vardef = unique_ptr<BaseAST>($1);
    auto vardefvec = $3;
    vardefvec->push_back(move(vardef));
    $$ = vardefvec;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->type = VarDefType::VarDef_Int_NO_Init;
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->type = VarDefType::VarDef_Int_Init;
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT ConstExpVec {
    auto ast = new VarDefAST();
    ast->type = VarDefType::VarDef_Arr_NO_Init;
    ast->ident = *unique_ptr<string>($1);
    ast->constexpvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    $$ = ast;
  }
  | IDENT ConstExpVec '=' InitVal {
    auto ast = new VarDefAST();
    ast->type = VarDefType::VarDef_Arr_Init;
    ast->ident = *unique_ptr<string>($1);
    ast->constexpvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->initval = unique_ptr<BaseAST>($4);
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = InitValType::InitVal_Exp;
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new InitValAST();
    ast->type = InitValType::InitVal_Null;
    $$ = ast;
  }
  | '{' InitValVec '}' {
    auto ast = new InitValAST();
    ast->initvalvec = unique_ptr<vector<unique_ptr<BaseAST> > >($2);
    ast->type = InitValType::InitVal_Vec;
    $$ = ast;
  }
  ;

InitValVec
  : InitVal {
    auto initval = unique_ptr<BaseAST>($1);
    auto initvalvec = new vector<unique_ptr<BaseAST> >();
    initvalvec->push_back(move(initval));
    $$ = initvalvec;
  }
  | InitVal ',' InitValVec {
    auto initval = unique_ptr<BaseAST>($1);
    auto initvalvec = $3;
    initvalvec->push_back(move(initval));
    $$ = initvalvec;
  }
  ;

ExpVec
  : '[' Exp ']' {
    auto exp = unique_ptr<BaseAST>($2);
    auto expvec = new vector<unique_ptr<BaseAST> >();
    expvec->push_back(move(exp));
    $$ = expvec;
  }
  | '[' Exp ']' ExpVec {
    auto exp = unique_ptr<BaseAST>($2);
    auto expvec = $4;
    expvec->push_back(move(exp));
    $$ = expvec;
  }
  ;
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
