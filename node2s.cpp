#include "main.h"

std::string node2str(Node* node) {
  if(!node)
    return"";

  switch(node->type){
    case NODE_VALUE:
      return node->token->obj.ToString();

    case NODE_VARIABLE:
      return node->token->str;
    
    default: {
      auto lhs = node2str(node->lhs);
      auto rhs = node2str(node->rhs);
      char const* kigou;

      switch(node->type){
        case NODE_ADD:kigou="+";break;
        case NODE_SUB:kigou="-";break;
        case NODE_MUL:kigou="*";break;
        case NODE_DIV:kigou="/";break;
      }

      return lhs + kigou + rhs;
    }
  }

  return "";
}