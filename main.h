#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <algorithm>
#include <cstring>
#include <cstdarg>
#include <list>
#include <map>

#define alert fprintf(stderr,"\t%s %d\n",__FILE__,__LINE__)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

enum {
  OBJ_INT,
  OBJ_CHAR,
  OBJ_DOUBLE,
  OBJ_STRING,
  OBJ_BOOL,
  OBJ_TUPLE,
  OBJ_ARRAY,
  OBJ_NONE,
};

enum {
  TOK_IMM,
  TOK_IDENT,
  TOK_RESERVED,
  TOK_END
};

enum {
  NODE_ADD,
  NODE_SUB,
  NODE_MUL,
  NODE_DIV,

  NODE_ASSIGN,

  NODE_SCOPE,

  NODE_IF,
  NODE_FOR,
  NODE_FOREACH,

  NODE_VALUE,
  NODE_VARIABLE
};

struct Node;
struct Object {
  using Type = u32;
  
  struct ObjPointer {
    Node* scope = nullptr;
    u64 index = 0;

    Object& operator * () const;
    Object* operator -> () const;
  };

  Type type = OBJ_NONE;
  ObjPointer obj_ptr;
  std::string name;

  i64 v_int = 0;
  char v_char = 0;
  double v_dbl = 0;
  bool v_bool = 0;
  std::string v_str;
  std::vector<Object> list;

  std::string ToString() const {
    switch( type ) {
      case OBJ_INT:
        return std::to_string(v_int);

      case OBJ_CHAR:
        return std::string(1, v_char);

      case OBJ_DOUBLE:
        return std::to_string(v_dbl);

      case OBJ_STRING:
        return v_str;

      case OBJ_BOOL:
        return v_bool ? "True" : "False";

      case OBJ_TUPLE:
      case OBJ_ARRAY: {
        std::string s;
        for(u64 i=0;i<list.size();i++){
          s+=list[i].ToString();
          if(i<list.size()-1) s+=", ";
        }
        return (type==OBJ_TUPLE?"(":"[")+s+(type==OBJ_TUPLE?")":"]");
      }

      case OBJ_NONE:
        return "None";
    }

    return "";
  }

  //bool Eval() const {
    

   // return false;
  //}
};

struct Token {
  using Type = u32;
  using Position = u64;

  Type type = TOK_IMM;
  Token* next = nullptr;
  std::string str;
  Position pos = 0;
  Object obj;

  Token(Type type = TOK_IMM)
    :type(type) { }

  Token(Type type, Token* back, Position pos)
    :type(type), pos(pos) {
    back->next = this;
  }
};

struct Node {
  using Type = u32;

  Type type = NODE_VALUE;
  Token* token = nullptr;
  Node* lhs = nullptr;
  Node* rhs = nullptr;
  std::vector<Node*> list;
  std::vector<Object> obj_list;

  Node(Type type = NODE_VALUE);
  Node(Type type, Node* lhs, Node* rhs, Token* token = nullptr);

};

struct Program {
  
  static inline std::list<Program*> _list;

  static Program* GetInstance() {
    return *_list.rbegin();
  }

  static Program* Push() {
    _list.push_front(new Program);
    return GetInstance();
  }

  static void Pop() {
    _list.pop_front();
  }

  std::string source;
  

};

namespace Utils {
  template <class T, class F, class... Args>
  i64 FindVector(std::vector<T>& vec, F compare, Args...args) {
    for( i64 i = 0; i < vec.size(); i++ ) {
      if( compare(vec[i], args...) )
        return i;
    }

    return -1;
  }

  template <class... Args>
  std::string Format(std::string const& fmt, Args...args) {
    static char buf[0x1000];
    sprintf(buf, fmt.c_str(), args...);
    return buf;
  }
  
  template <class T>
  T Replace(T str, T const& find, T const& repl) {
    if( str.empty() )
      return str;
    
    for( u64 i = 0; i < str.length() - find.length(); ) {
      if( str.substr(i, find.length()) == find ) {
        str.erase(i, find.length());
        str.insert(i, repl);
        i += repl.length();
      }
      else
        i++;
    }

    return str;
  }

  inline bool IsHexadecimal(char ch) {
    return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || (ch >= '0' && ch <= '9');
  }
}

inline std::ostream& operator << (std::ostream& ost, Object const& obj) {
  return ost << obj.ToString();
}

std::string node2str(Node*);

std::string readfile(char const*);

[[noreturn]]
void error(Token::Position pos, std::string const& msg);

Token* tokenize();

Node* parse(Token* token);

Object run_node(Node* node);

