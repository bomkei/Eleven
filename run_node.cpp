#include "main.h"

Object& Object::ObjPointer::operator * () const {
  return scope->obj_list[index];
}

Object* Object::ObjPointer::operator -> () const {
  return &scope->obj_list[index];
}

void AdjustObjectType(Object& L, Object& R) {
  if((L.type==OBJ_ARRAY)!=(R.type==OBJ_ARRAY))
    return;
  

}

Object run_node(Node* node) {
  if( !node )
    return { };
  
  switch(node->type){
    case NODE_VALUE:
      return node->token->obj;
    
    case NODE_VARIABLE:
      if( !node->token->obj.obj_ptr.scope )
        error(node->token->pos, "cannot use variable before assignment");
      
      return *node->token->obj.obj_ptr;
    
    default: {
      auto lhs = run_node(node->lhs);
      auto rhs = run_node(node->rhs);
      
      if(lhs.type!=rhs.type)
        error(node->token->pos,"type mismatch");
      
      switch( node->type ) {
        case NODE_ADD:
          switch(lhs.type){
            case OBJ_INT: lhs.v_int+=rhs.v_int; break;
            case OBJ_CHAR: lhs.v_char += rhs.v_char; break;
            case OBJ_DOUBLE: lhs.v_dbl += rhs.v_dbl; break;
            case OBJ_STRING: lhs.v_str += rhs.v_str; break;
          }
          break;
        
        case NODE_SUB:
          switch(lhs.type){
            case OBJ_INT: lhs.v_int-=rhs.v_int; break;
            case OBJ_CHAR: lhs.v_char -= rhs.v_char; break;
            case OBJ_DOUBLE: lhs.v_dbl -= rhs.v_dbl; break;
          }
          break;
        
        case NODE_MUL:
          switch(lhs.type){
            case OBJ_INT: lhs.v_int*=rhs.v_int; break;
            case OBJ_CHAR: lhs.v_char *= rhs.v_char; break;
            case OBJ_DOUBLE: lhs.v_dbl *= rhs.v_dbl; break;
          }
          break;
        
        case NODE_DIV:
          switch(lhs.type){
            case OBJ_INT: lhs.v_int/=rhs.v_int; break;
            case OBJ_CHAR: lhs.v_char /= rhs.v_char; break;
            case OBJ_DOUBLE: lhs.v_dbl /= rhs.v_dbl; break;
          }
          break;
        

      }

      
      return lhs;
    }
  }
  
  return { };
}