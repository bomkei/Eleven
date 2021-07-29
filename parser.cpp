#include "main.h"

namespace {
  Token* token;
  Token* csmtok;

  bool check() {
    return token->type != TOK_END;
  }

  void next() {
    token = token->next;
  }

  bool consume(char const* str) {
    if( token->str == str ) {
      csmtok = token;
      next();
      return true;
    }

    return false;
  }

  void expect(char const* str) {
    if( !consume(str) ) {
      error(token->pos, Utils::Format("expect '%s'", str));
    }
  }

  void expect_s(char const* str){
    if(token->str!=str){
      error(token->pos, Utils::Format("expect '%s'", str));
    }
  }
}

Node::Node(Node::Type type)
  :type(type), token(csmtok) { }

Node::Node(Node::Type type, Node* lhs, Node* rhs, Token* token)
  :type(type), lhs(lhs), rhs(rhs), token(token ? token : csmtok) { }

Node* expr();
Node* primary() {
  
  if(consume("(")){
    auto x = expr();
    expect(")");
    return x;
  }

  if(consume("[")){
    auto x = new Node(NODE_ARRAY);
    if(!consume("]")){
      do{x->list.emplace_back(expr());}
      while(consume(","));
      expect("]");
    }
    return x;
  }
  
  switch( token->type ) {
    case TOK_IMM: {
      auto x = new Node(NODE_VALUE);
      x->token = token;
      next();
      return x;
    }
    
    case TOK_IDENT: {
      auto x = new Node(NODE_VARIABLE);
      x->token = token;
      next();

      if( consume("(") ) {
        x->type = NODE_CALLFUNC;

        if(!consume(")")){
          do{ x->list.emplace_back(expr()); }
          while( consume(",") );
          expect(")");
        }
      }

      return x;
    }
  }
  
  error(token->pos, "syntax error");
}

Node* indexref() {
  auto x= primary();
  while(check()&&consume("[")){
    x=new Node(NODE_INDEXREF,x,expr());
    expect("]");
  }
  return x;
}

Node* memberaccess() {
  auto x = indexref();
  while(check()&&consume(".")){
    if(token->type!=TOK_IDENT)
      error(csmtok->pos,"syntax error");
    x=new Node(NODE_MEMBERACCESS,x,indexref());
  }
  return x;
}

Node* unary() {
  if(consume("-"))
    return new Node(NODE_SUB,new Node,memberaccess());
  
  if(consume("!"))
    return new Node(NODE_LOGNOT,new Node,memberaccess());

  if(consume("~"))
    return new Node(NODE_BITNOT,new Node,memberaccess());

  return memberaccess();
}

Node* mul() {
  auto x = unary();
  while( check() )
    if(consume("*")) x=new Node(NODE_MUL,x,unary());
    else if(consume("/")) x=new Node(NODE_DIV,x,unary());
    else if(consume("%")) x=new Node(NODE_MOD,x,unary());
    else break;
  return x;
}

Node* add() {
  auto x = mul();
  while( check() )
    if(consume("+")) x=new Node(NODE_ADD,x,mul());
    else if(consume("-")) x=new Node(NODE_SUB,x,mul());
    else break;
  return x;
}

Node* shift() {
  auto x=add();
  while(check()){
    if(consume("<<")) x=new Node(NODE_SHIFT,x,add());
    else if(consume(">>")) x=new Node(NODE_SHIFT,add(),x);
    else break;
  }
  return x;
}

Node* compare(){
  auto x=shift();
  while(check()){
    if(consume(">")) x=new Node(NODE_BIGGER,x,shift());
    else if(consume("<")) x=new Node(NODE_BIGGER,shift(),x);
    else if(consume(">=")) x=new Node(NODE_BIG_OR_EQ,x,shift());
    else if(consume("<=")) x=new Node(NODE_BIG_OR_EQ,shift(),x);
    else break;
  }
  return x;
}

Node* equal(){
  auto x=compare();
  while(check()){
    if(consume("==")) x=new Node(NODE_EQUAL,x,compare());
    else if(consume("!=")) x=new Node(NODE_NOT_EQUAL,x,compare());
    else break;
  }
  return x;
}

Node* bit_And() {
  auto x=equal();
  while(check()&&consume("&"))
    x=new Node(NODE_BITAND,x,equal());
  return x;
}

Node* bit_Xor(){
  auto x=bit_And();
  
  while(check()&&consume("^"))
    x=new Node(NODE_BITXOR,x,bit_And());

  return x;
}

Node* bit_Or(){
  auto x=bit_Xor();
  while(check()&&consume("|"))
    x=new Node(NODE_BITOR,x,bit_Xor());
  return x;
}

Node* log_And(){
  auto x=bit_Or();
  while(check()&&consume("&&"))
    x=new Node(NODE_LOGAND,x,bit_Or());
  return x;
}

Node* log_Or(){
  auto x=log_And();
  while(check()&&consume("||"))
    x=new Node(NODE_LOGOR,x,log_And());
  return x;
}

Node* assign() {
  auto x = log_Or();

  if(consume("=")){
    x=new Node(NODE_ASSIGN,x,assign());
  }

  return x;
}

Node* expr() {
  return assign();
}

Node* stmt() {
  
 if(consume("{")){
   auto node = new Node(NODE_SCOPE);
   auto closed=false;
   while(check()){
    if(consume("}")) { closed=true; break; }
    node->list.emplace_back(stmt());
   }
 if(!closed){
   error(node->token->pos,"not closed");
 }

   return node;
 }

  if(consume("if")){
    auto node = new Node(NODE_IF);

    node->lhs = expr();
    expect_s("{");
    node->rhs=stmt();
    if(consume("else")) node->list.emplace_back(stmt());

    return node;
  }

  if(consume("for")){
    auto node = new Node(NODE_FOR);
     for(i8 i=0;i<2;i++){
    node->list.emplace_back(expr());
    expect(";");
     }

    node->list.emplace_back(expr());
    expect_s("{");
    node->lhs=stmt();
    
    return node;
  }

  if(consume("foreach")){
    auto node = new Node(NODE_FOREACH);
    node->lhs = expr(); // iterator
    expect("in");
    node->rhs = expr(); // content
    expect_s("{");
    node->list.emplace_back(stmt());
    return node;
  }

  if(consume("while")){
    return new Node(NODE_WHILE,expr(),stmt(),csmtok);
  }

  if(consume("do")){
    expect_s("{");
    auto node = new Node(NODE_DOWHILE,stmt(),nullptr);
    expect("while");
    node->rhs = expr();
    return node;
  }

  if(consume("break")){
    auto x = new Node(NODE_BREAK);
    expect(";");
    return x;
  }

  if(consume("continue")){
    auto x = new Node(NODE_CONTINUE);
    expect(";");
    return x;
  }

  if(consume("return")){
    if(consume(";")) return new Node(NODE_RETURN);
    auto x=new Node(NODE_RETURN);
    x->lhs=expr();
    expect(";");
    return x;
  }

  if(consume("def")){
    if(token->type!=TOK_IDENT)
      error(token->pos,"expect identifier");
    
    auto node = new Node(NODE_FUNCTION);
    node->token = token;

    next();
    expect("(");

    if(!consume(")")){
      do {
        if(token->type!=TOK_IDENT)
          error(token->pos,"expect identifier");
        
        node->obj_list.emplace_back(token->obj);
        next();
      }while(consume(","));
      expect(")");
    }

    node->lhs = stmt();
    return node;
  }
  
  auto x=expr();
  expect(";");
  return x;
}

Node* parse(Token* tok) {
  token = tok;
  
  auto x = new Node(NODE_SCOPE);

  while( check() ) {
    x->list.emplace_back(stmt());
  }

  return x;
}