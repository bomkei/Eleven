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
  :type(type) { }

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
    x=new Node(NODE_MEMBERACCESS,x,indexref());
  }
  return x;
}

Node* unary() {
  if(consume("-"))
    return new Node(NODE_SUB,new Node,memberaccess());
  
  return memberaccess();
}

Node* mul() {
  auto x = unary();
  while( check() )
    if(consume("*")) x=new Node(NODE_MUL,x,unary());
    else if(consume("/")) x=new Node(NODE_DIV,x,unary());
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

Node* assign() {
  auto x = add();

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