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
#include <cmath>

#define __DEBUG__ 1

#if __DEBUG__
  #define alert fprintf(stderr,"\t%s %d\n",__FILE__,__LINE__)
#else
  #define alert 0
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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

  template <class T>
  void Reverse(T& arr) {
    if(arr.size()<=2)
      return;
    
    u64 max = arr.size() / 2 + arr.size() % 1;
    
    for(u64 i = 0; i < max; i++ ) {
      std::swap(arr[i], arr[arr.size() - 1 - i]);
    }
  }

  template <class T, class F>
  std::vector<T> CreateVector(std::vector<T> const& vec1, std::vector<T> const& vec2, F fun){
    std::vector<T> ret;

    if( vec1.size() != vec2.size() )
      throw 0;

    for( u64 i = 0; i < vec1.size(); i++ ) {
      ret.emplace_back(fun(vec1[i], vec2[i]));
    }

    return ret;
  }

  inline bool IsHexadecimal(char ch) {
    return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || (ch >= '0' && ch <= '9');
  }
}

enum {
  OBJ_INT,
  OBJ_CHAR,
  OBJ_DOUBLE,
  OBJ_STRING,
  OBJ_BOOL,
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
  NODE_MOD,
  NODE_SHIFT,
  NODE_BIGGER,
  NODE_BIG_OR_EQ,
  NODE_EQUAL,
  NODE_NOT_EQUAL,
  NODE_BITAND,
  NODE_BITXOR,
  NODE_BITOR,
  NODE_LOGAND,
  NODE_LOGOR,

  NODE_LOGNOT,
  NODE_BITNOT,
  
  NODE_ASSIGN,

  NODE_SCOPE,

  NODE_IF,
  NODE_FOR,
  NODE_FOREACH,
  NODE_WHILE,
  NODE_DOWHILE,
  NODE_SWITCH,
  NODE_LOOP,

  NODE_FUNCTION,

  NODE_BREAK,
  NODE_CONTINUE,
  NODE_RETURN,

  NODE_INDEXREF,
  NODE_MEMBERACCESS,

  NODE_ARRAY,
  NODE_VALUE,
  NODE_VARIABLE,
  NODE_CALLFUNC,
};

struct Node;
struct Object {
  using Type = u32;
  
  struct ObjPointer {
    Node* scope = nullptr;
    u64 index = 0;

    Object& operator * () const;
    Object* operator -> () const;

  #if __DEBUG__
    std::string ToString() const {
      return Utils::Format("\tObjPointer %p: scope=%p index=%d", this, scope, index);
    }
  #endif
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

      case OBJ_ARRAY: {
        std::string s;
        for(u64 i=0;i<list.size();i++){
          s+=list[i].ToString();
          if(i<list.size()-1) s+=", ";
        }
        return "[" + s + "]";
      }

      case OBJ_NONE:
        return "None";
    }

    return "";
  }

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
  std::vector<Node*> functions;
  std::vector<Object> obj_list;

  Node(Type type = NODE_VALUE);
  Node(Type type, Node* lhs, Node* rhs, Token* token = nullptr);

};

struct Program {
  
  static inline std::list<Program*> _list;
  std::string source;

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

  

};

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

