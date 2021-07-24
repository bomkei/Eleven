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
      return x;
    }
  }
  
  error(token->pos, "syntax error");
}

Node* mul() {
  auto x = primary();
  while( check() )
    if(consume("*")) x=new Node(NODE_MUL,x,primary());
    else if(consume("/")) x=new Node(NODE_DIV,x,primary());
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

Node* expr() {
  return add();
}

Node* stmt() {
  
  
}

Node* parse(Token* tok) {
  token = tok;
  
  return expr();
}