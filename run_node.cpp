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

std::list<Node*> scope_list;

Object::ObjPointer find_var(std::string const& name) {
  for(auto it=scope_list.rbegin(); it!=scope_list.rend(); it++ ) {
    for(u64 i=0;i<(*it)->obj_list.size();i++){
      if((*it)->obj_list[i].name==name)
        return{ *it, i };
    }
  }

  return { };
}

Node* get_cur_scope() {
  return * scope_list.rbegin();
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
    
    case NODE_ASSIGN: {
      if(node->lhs->type==NODE_VARIABLE){
        auto ptr = find_var(node->lhs->token->str);

        if(!ptr.scope){
          node->lhs->token->obj.obj_ptr = { get_cur_scope(), get_cur_scope()->obj_list.size() };
          get_cur_scope()->obj_list.emplace_back(node->lhs->token->obj);
        }
      }

      auto dest = run_node(node->lhs);
      auto src = run_node(node->rhs);

      *dest.obj_ptr = src;
      dest.obj_ptr->name = dest.name;

      return src;
    }

    case NODE_SCOPE: {
      scope_list.push_back(node);

      for(auto&&i:node->list)
        run_node(i);

      scope_list.pop_back();
      break;
    }

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