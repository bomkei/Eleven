#include "main.h"

static auto p_tokens = {
  "...",
  ">>=",
  "<<=",
  "<=>",
  "->",
  "<-",
  ">=",
  "<=",
  ">>",
  "<<",
  "==",
  "!=",
  "+=",
  "-=",
  "*=",
  "/=",
  "%=",
  "&=",
  "|=",
  "[]",
  "!",
  "=",
  ".",
  ",",
  "?",
  ">",
  "<",
  "!",
  "%",
  "&",
  "^",
  "|",
  "(",
  ")",
  "[",
  "]",
  "{",
  "}",
  ";",
  ":",
  "+",
  "-",
  "*",
  "/",
  "@",
};

namespace {
  Token::Position position;

  std::string const& src() {
    return Program::GetInstance()->source;
  }
  
  bool check() {
    return position < src().length();
  }

  char peek() {
    return src()[position];
  }

  void next() {
    position++;
  }

  bool match(char const* str) {
    return position + strlen(str) <= src().length() && memcmp(src().c_str() + position, str, strlen(str)) == 0;
  }

  void pass_space() {
    while( check() && peek() <= ' ' )
      next();
  }
}

void error(Token::Position pos, std::string const& msg) {
  std::cout << "pos=" << pos << ": " << msg << std::endl;
  
  exit(1);
}

Token* tokenize() {
  Token top;
  Token* cur = &top;

  pass_space();
  while( check() ) {
    auto ch = peek();
    auto pos = position;

    cur = new Token(TOK_IMM, cur, pos);

    // hex
    if( match("0x") ) {
      cur->str += "0x";
      position += 2;

      while( check() && Utils::IsHexadecimal(ch = peek()) ) {
        cur->str += ch;
        next();
      }

      cur->obj.type = OBJ_INT;
      cur->obj.v_int = std::stoi(cur->str, nullptr, 16);
    }

    // int
    else if( isdigit(ch) ) {
      while( check() && isdigit(ch = peek()) ) {
        cur->str += ch;
        next();
      }

      cur->obj.type = OBJ_INT;
      cur->obj.v_int = std::stoi(cur->str);

      if( peek() == '.' ) {
        next();
        if(!isdigit(peek())){
          position--;
          continue;
        }
        cur->obj.type = OBJ_DOUBLE;
        cur->str += '.';

        while( check() && isdigit(peek()) ) {
          cur->str += ch;
          next();
        }

        cur->obj.v_dbl = std::stod(cur->str);
      }
    }

    // identifier
    else if( isalpha(ch) || ch == '_' ) {
      cur->type = TOK_IDENT;
      
      while( check() && (isalnum(ch = peek()) || ch == '_') ) {
        cur->str += ch;
        next();
      }

      cur->obj.name = cur->str;
    }

    // string
    else if( ch == '"' ) {
      next();
      cur->obj.type = OBJ_STRING;

      while( check() && (ch = peek()) != '"' ) {
        cur->str += ch;
        next();
      }

      next();
      cur->obj.v_str = cur->str;
    }

    // char
    else if( ch == '\'' ) {
      next();
      cur->obj.type = OBJ_CHAR;
      cur->obj.v_char = peek();
      next();
      if( peek() != '\'' ) error(cur->pos, "error");
      next();
    }

    // other
    else {
      for( auto&& i : p_tokens ) {
        if( match(i) ) {
          cur->type = TOK_RESERVED;
          cur->str = i;
          position += strlen(i);
          goto hitjmp;
        }
      }

      error(cur->pos, "unknown token");

    hitjmp:;
    }

    pass_space();
  }

  cur = new Token(TOK_END, cur, position);
  return top.next;
}