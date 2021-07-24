#include "main.h"

std::string readfile(char const* path) {
  std::ifstream ifs(path);
  std::string ret, line;

  if( ifs.fail() ) {
    std::cout << "";
    exit(1);
  }

  u64 line_num = 1;

  while( std::getline(ifs, line) ) {
    int indent_count = 0;
    for( auto&& ch : line ) {
      if( ch == ' ' ) indent_count++;
      else break;
    }

    if( indent_count % 2 ) {
      printf("line %lld: invalid indent", line_num);
      exit(1);
    }

    line.erase(0, indent_count);
    line.insert(0, std::string(indent_count / 2, '\t'));
    
    for( auto it = line.begin() + indent_count / 2; it != line.end(); ) {
      if( *it == '"' || *it == '\'' ) {
        auto c = *it;
        for( ++it; *it != c; it++ );
        it++;
      }
      else if( *it == '\t' ) {
        *it = ' ';
      }
      else
        it++;
    }
    
    ret += line + '\n';
    line_num++;
  }

  return ret;
}

int main(int argc, char** argv) {

  auto prg = Program::Push();

  prg->source = readfile("test.txt");

  std::cout << prg->source;
  
alert;
  auto token = tokenize();

alert;
  auto node = parse(token);

alert;
  std::cout<< node2str(node)<< '\n';

alert;
  auto obj = run_node(node);

alert;
  std::cout << obj << '\n';

  Program::Pop();
}