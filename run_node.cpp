#include "main.h"

std::list<Node*> scope_list;

namespace{
  bool* loop_breaked;
  bool* loop_continued;
}

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

void AssignObject(Object& dest, Object& src) {
  *dest.obj_ptr = src;
  dest.obj_ptr->name = dest.name;
  dest.obj_ptr->obj_ptr = dest.obj_ptr;
}

void push_scope(Node* node) {
  scope_list.push_back(node);
}

void pop_scope() {
  scope_list.pop_back();
}

Node* get_cur_scope() {
  return * scope_list.rbegin();
}

Object::ObjPointer find_var(std::string const& name) {
  for(auto it=scope_list.rbegin(); it!=scope_list.rend(); it++ ) {
    for(u64 i=0;i<(*it)->obj_list.size();i++){
      if((*it)->obj_list[i].name==name)
        return{ *it, i };
    }
  }

  return { };
}

void make_var(Node* node) {
  if(node->type!=NODE_VARIABLE)
    return;

  if(!find_var(node->token->str).scope){
    node->token->obj.obj_ptr = { get_cur_scope(), get_cur_scope()->obj_list.size() };
    get_cur_scope()->obj_list.emplace_back(node->token->obj);
  }
}

Object run_node(Node* node) {
  if( !node )
    return { };
  
  switch(node->type){
    case NODE_VALUE:
      return node->token->obj;
    
    case NODE_VARIABLE: {
      auto find = find_var(node->token->str);

      if(!find.scope)
        error(node->token->pos,"cannot use variable before assignment");

      return *find;
    }

    case NODE_CALLFUNC: {
      auto const& name = node->token->str;
      std::vector<Object> args;

      for(auto&&i:node->list)
        args.emplace_back(run_node(i));
      
      if(name=="print"){
        for(auto&&i:args) std::cout << i;
        std::cout << std::endl;
      }
      
      break;
    }
    
    case NODE_ARRAY: {
      Object obj;
      obj.type = OBJ_ARRAY;

      for(auto&&i:node->list)
        obj.list.emplace_back(run_node(i));
      
      return obj;
    }

    case NODE_INDEXREF: {
      auto obj = run_node(node->lhs);
      auto index = run_node(node->rhs);

      if(obj.type!=OBJ_ARRAY)
        error(node->token->pos,"object is not an array");
      
      if(index.type!=OBJ_INT)
        error(node->token->pos,"index is must be an integer");
      
      if(index.v_int<0||index.v_int>=obj.list.size())
        error(node->token->pos,"index out of range");
      
      return obj.list[index.v_int];
    }

    case NODE_MEMBERACCESS: {
      auto obj = run_node(node->lhs);
      
      std::list<Node*> index_list;

      Node* nd=node->rhs;
      while(1){
        if(nd->type==NODE_INDEXREF) {
          index_list.push_front(nd);
          nd=nd->lhs;
        }
        else
          break;
      }

      auto const& name = nd->token->str;
      auto is_func = nd->type==NODE_CALLFUNC;
      std::vector<Object> args;

      if(is_func){
        for(auto&&i:nd->list)
          args.emplace_back(run_node(i));
      }

      if(name=="reverse" && is_func){
        if(obj.type==OBJ_ARRAY)
          Utils::Reverse(obj.list);
        else if(obj.type==OBJ_STRING)
          Utils::Reverse(obj.v_str);
        else
          error(nd->token->pos,"object is not array or string");
      }
      else if(name=="append"&&is_func){
        if(obj.type!=OBJ_ARRAY)
          error(nd->token->pos,"object is not array");
        if(args.size()!=1)
          error(nd->token->pos,"invalid arguments");
        
        obj.list.emplace_back(args[0]);
        if(obj.obj_ptr.scope) AssignObject(*obj.obj_ptr, obj);
      }
      else if(name=="length"&&!is_func){
        if(obj.type==OBJ_ARRAY)
          obj.v_int=obj.list.size();
        else if(obj.type==OBJ_STRING)
          obj.v_int=obj.v_str.length();
        else
          error(nd->token->pos,"object is not array or string");
        
        obj.type=OBJ_INT;
      }
      else if(name=="sqrt"&&!is_func){
        if(obj.type==OBJ_INT)
          obj.v_dbl=std::sqrt(obj.v_int);
        else if(obj.type==OBJ_DOUBLE)
          obj.v_dbl=std::sqrt(obj.v_dbl);
        else
          error(nd->token->pos,"object is not numeric");
        
        obj.type=OBJ_DOUBLE;
      }
      else
        error(node->token->pos,"object is not have member '"+name+"'");

      for(auto it=index_list.begin();it!=index_list.end();it++) {
        if(obj.type!=OBJ_ARRAY)
          error((*it)->token->pos,"object is not array");
        
        auto index = run_node((*it)->rhs);
        
        if(index.type!=OBJ_INT)
          error((*it)->rhs->token->pos,"index is must be an integer");
      
        if(index.v_int<0||index.v_int>=obj.list.size())
          error((*it)->rhs->token->pos,"index out of range");

        obj = obj.list[index.v_int];
      }

      return obj;

   // errjmp:;
    //  error(node->token->pos,"object is not have member '"+name+"'");
    }

    case NODE_ASSIGN: {
      make_var(node->lhs);

      auto dest = run_node(node->lhs);
      auto src = run_node(node->rhs);

      if(!dest.obj_ptr.scope)
        error(node->token->pos,"left side is rvalue");

      AssignObject(dest, src);

      return src;
    }

    case NODE_SCOPE: {
      push_scope(node);

      for(auto&&i:node->list)
        run_node(i);

      pop_scope();
      break;
    }

    case NODE_IF: {
      auto cond = run_node(node->lhs);
      if(cond.type!=OBJ_BOOL) error(node->token->pos,"condition must be boolean");
      
      if(cond.v_bool)
        run_node(node->rhs);
      else if(node->list.size())
        run_node(node->list[0]);
      
      break;
    }

    case NODE_FOR: {
      run_node(node->list[0]);

      auto p1=loop_breaked;
      auto p2=loop_continued;

      auto lb=false;
      auto lc=false;
      loop_breaked=&lb;
      loop_continued=&lc;

      while(1){
        auto cond = run_node(node->list[1]);
        if(cond.type!=OBJ_BOOL) error(node->token->pos,"condition is must be boolean");
        if(!cond.v_bool) break;

        run_node(node->lhs);
        lb=lc=0;
        run_node(node->list[2]);
        if(lb)break;
      }

      loop_breaked=p1;
      loop_continued=p2;

      break;
    }

    case NODE_FOREACH: {
      push_scope(node);

      make_var(node->lhs);
      
      auto iterator = run_node(node->lhs);
      
      auto p1=loop_breaked;
      auto p2=loop_continued;

      auto lb=false;
      auto lc=false;
      loop_breaked=&lb;
      loop_continued=&lc;

      if(!iterator.obj_ptr.scope)
        error(node->token->pos,"iterator is rvalue");
  
      u64 index = 0;
      while(1) {
        auto content = run_node(node->rhs);

        if(content.type!=OBJ_ARRAY)
          error(node->rhs->token->pos,"content is not an array");

        AssignObject(iterator, content.list[index]);
        index++;

        *loop_breaked = false;
        *loop_continued = false;
        
        run_node(node->list[0]);

        if(index>=content.list.size() || *loop_breaked) break;
      }

      loop_breaked=p1;
      loop_continued=p2;

      pop_scope();
      break;
    }

    case NODE_BREAK: {
      if(!loop_breaked)
        error(node->token->pos,"here is not inner of loop statement");
      
      *loop_breaked=true;
      break;
    }

    case NODE_CONTINUE: {
      if(!loop_continued)
        error(node->token->pos,"here is not inner of loop statement");
      
      *loop_continued=true;
      break;
    }

    case NODE_RETURN: {

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