#include "main.h"

std::string readfile(char const* path) {
  std::ifstream ifs(path);
  std::string ret, line;

  if( ifs.fail() ) {
    std::cout << "";
    exit(1);
  }
  
  while( std::getline(ifs, line) ) {
    ret += line + '\n';
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