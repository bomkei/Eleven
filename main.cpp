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

  auto token = tokenize();

  auto node = parse(token);

  auto obj = run_node(node);

  Program::Pop();
}