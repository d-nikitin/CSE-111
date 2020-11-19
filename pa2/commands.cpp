// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $
// Dmitriy Nikitin ID: 1649206 PA2

#include "commands.h"
#include "debug.h"
#include "file_sys.h"
#include "util.h"
#include<iomanip>

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"    , fn_rmr  },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}


//helper function returns true if the file exsits
bool check_file(inode_state& state, string fileName){
   inode_ptr CWD = state.get_cwd();
   auto check = CWD->get_contents()->get_dirents().find(fileName);
   if(check == CWD->get_contents()->get_dirents().end())
      return false;
   return true;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr curr_node = state.get_cwd();
   wordvec out;
   for(int i = 1; i < words.size(); i++){
      auto file = curr_node->get_contents()->
                              get_dirents().find(words.at(i));
      if(file == curr_node->get_contents()->get_dirents().end() )
         cerr << "cat: " << words.at(i) 
                     << ": No such file or directory"<< endl;
      else{
         out = file->second->get_contents()->readfile();
         cout << out <<endl;
      }
      out.clear();
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){
      state.set_cwd(state.get_root());
   }
   else if (words.at(1) == "/"){
      state.set_cwd(state.get_root());
   }
   else{
      auto curr_node = 
         state.get_cwd()->get_contents()->
                     get_dirents().find(words.at(1));
      if(curr_node != state.get_cwd()->get_contents()->
                                    get_dirents().end()){
         inode_ptr file = curr_node->second;
         if(file->get_file_type() == 'p')
            cerr << words << " is a plainfile" << endl;
         else
            state.set_cwd(file);
      }
      else
         cerr << "no such file exists" << endl;
   }
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}

//helper methods found on stack overflow to check if a string consists
//entirly of digits

bool isDigits(const string &s){
   return s.find_first_not_of("0123456789") == string::npos;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() > 1){
      if(isDigits(words.at(1))){
         exec::status(stoi(words.at(1)));
      }
      else 
         exec::status(127);
   }
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr curr_node;
   if(words.size() == 1){
      curr_node = state.get_cwd();
   }
   else if(words.at(1) == "/"){
      curr_node = state.get_root();
   }
   else if(check_file(state,words.at(1))){
      curr_node = state.get_cwd()->get_contents()->
               get_dirents().find(words.at(1))->second;
   }
   else{
      cerr << "error file/directory does not exsist" << endl;
      return;
   }
   if(curr_node->get_file_type() == 'p'){
      fn_cat(state,words);
      return;
   }
   else{
      string dot = ".";
      string dotdot = "..";
      map<string, inode_ptr> map;
      map = curr_node->get_contents()->get_dirents();
      try{
         if(words.at(1) == dot)
            cout << dot << ":" << endl;
         else if (words.at(1) == dotdot)
            cout << dotdot << ":" << endl;
         else
            cout << curr_node->get_path() << ":" << endl;
      }
      catch(const std::out_of_range& e){
         cout << curr_node->get_path() << ":" << endl;
      }
      std::map<string, inode_ptr>::iterator it = map.begin();
      while(it != map.end()){
         inode_ptr node = it->second;
         cout << setw(6) << node->get_inode_nr();
         cout << setw(6) << node->get_contents()->size();
         string print = it->first;
         if(node->get_file_type() == 'd' and 
                  print != dot and print != dotdot)
            cout << "  " << it->first << "/" << endl;
         else
            cout << "  " << it->first << endl;
         it++;
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string dot = ".";
   string dotdot = "..";
   inode_ptr backup = state.get_cwd();
   bool bugfix = false;
   for(int i = 1; i < words.size(); i++){
      if(words.size() > 1) {
         if(words.at(i) == "/"){
            state.set_cwd(state.get_root());
         }
         else if(words.at(i).find("/") != string::npos){
            string path = words.at(i);
            wordvec dirs;
            int last_cut = 0;
            for(int i = 0; i < path.size(); i++){
               if(path.at(i) == '/'){
                  string g  = path.substr(last_cut, i-last_cut);
                  last_cut = 0;
                  dirs.push_back(g);
                  last_cut += i + 1;
               }
               else if(i == path.size()-1){
                  dirs.push_back(path.substr(last_cut, i));
               }
            }
            for(int i = 0; i < dirs.size(); i++){
               auto curr_node =
                  state.get_cwd()->get_contents()->
                           get_dirents().find(dirs.at(i));
               if(curr_node != state.get_cwd()->get_contents()->
                                          get_dirents().end()){
                  state.set_cwd(curr_node->second);
                  if(i == dirs.size()-1){
                     wordvec wordbug;
                     wordbug.push_back("ls");
                     fn_ls(state,wordbug);
                  }
               }  
               else{
                  cerr << "error: " << dirs.at(i) 
                     << " dir does not exist" << endl;
                  break;
               }      
            } 
         }
         else {//there is word 
            auto curr_node = 
               state.get_cwd()->get_contents()->
                           get_dirents().find(words.at(1));
            if(curr_node != state.get_cwd()->get_contents()->
                                          get_dirents().end()){
                  fn_ls(state,words);
                  state.set_cwd(curr_node->second);
                  bugfix = true;
               }
               else{
                  cerr << "error: " << words.at(i) 
                     << " dir does not exist" << endl;
                  break;
               }     
         }
      }
      std::map<string, inode_ptr> map;
      map = state.get_cwd()->get_contents()->get_dirents();
      if(!bugfix) fn_ls(state,words);
      string name;
      for(auto p: map){
         name = p.first;
         if(name != dot and name != dotdot){
            inode_ptr node = p.second;
            if(node->get_file_type() == 'd'){
               wordvec words2;
               words2.push_back(name);
               state.set_cwd(node);
               fn_lsr(state,words2);
               state.set_cwd(backup);
               }
         }
         else 
            continue;
      }
   }
   if(words.size() == 1){ //we are recuring bois
      std::map<string, inode_ptr> map;
      map = state.get_cwd()->get_contents()->get_dirents();
      fn_ls(state,words);
      string name;
      for(auto p: map){
         name = p.first;
         if(name != dot and name != dotdot){
            inode_ptr node = p.second;
            if(node->get_file_type() == 'd'){
               wordvec words2;
               words2.push_back(name);
               state.set_cwd(node);
               fn_lsr(state,words2);
               state.set_cwd(backup);
               }
         }
         else 
            continue;
      }
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.at(1).find("/") != string::npos){
      bool broken = false;
      inode_ptr backup = state.get_cwd();
      string path = words.at(1);
      string filename = "";
      wordvec dirs;
      int last_cut = 0;
      for(int i = 0; i < path.size(); i++){
         if(path.at(i) == '/'){
            string g  = path.substr(last_cut, i-last_cut);
            last_cut = 0;
            dirs.push_back(g);
            last_cut += i + 1;
         }
         else if(i == path.size()-1){
            filename = path.substr(last_cut, i);
         }
      }
      for(int i = 0; i < dirs.size(); i++){
         auto curr_node =
            state.get_cwd()->get_contents()->
                     get_dirents().find(dirs.at(i));
         if(curr_node != state.get_cwd()->get_contents()->
                                    get_dirents().end()){
            state.set_cwd(curr_node->second);
         }
         else{
            cerr << "error: " << dirs.at(i) << " dir does not exist" 
                  << endl;
            broken = true;
            break;
         }      
      }
      if(broken != true){
         wordvec dat;
         for(int i = 2; i < words.size(); i++)
            dat.push_back(words.at(i));
         if(check_file(state,filename) == false){
            inode_ptr file = state.get_cwd()->get_contents()->
                                          mkfile(filename, state);
            file->get_contents()->set_data(dat);
            state.get_cwd()->get_contents()->get_dirents().
                                          insert({filename,file});
         }
         else 
            cerr << "error: " << filename 
                  << " already exists" << endl; 
      }
      state.set_cwd(backup);
   }
   else{
      string file_name = words.at(1); 
      wordvec dat;
      for(int i = 2; i < words.size(); i++)
         dat.push_back(words.at(i));
      if(check_file(state,file_name) == false){
         auto file = state.get_cwd()->
                  get_contents()->mkfile(file_name,state);
         file->get_contents()->set_data(dat);
         state.get_cwd()->get_contents()->
                     get_dirents().insert({file_name,file});
      }
      else{
         auto file = state.get_cwd()->get_contents()->
                           get_dirents().find(file_name);
         file->second->get_contents()->set_data(dat);
      }
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.at(1).find("/") != string::npos){
      bool broken = false;
      inode_ptr backup = state.get_cwd();
      string path = words.at(1);
      wordvec dirs;
      int last_cut = 0;
      for(int i = 0; i < path.size(); i++){
         if(path.at(i) == '/'){
            string g  = path.substr(last_cut, i-last_cut);
            last_cut = 0;
            dirs.push_back(g);
            last_cut += i + 1;
         }
         else if(i == path.size()-1){
            dirs.push_back(path.substr(last_cut, path.size()));
         }
      }
      for(int i = 0; i < dirs.size()-1; i++){
         auto curr_node =
            state.get_cwd()->get_contents()->
                     get_dirents().find(dirs.at(i));
         if(curr_node != state.get_cwd()->get_contents()->
                                    get_dirents().end()){
            state.set_cwd(curr_node->second);
         }
         else{
            cerr << "error: " << dirs.at(i) << " dir does not exist" 
                  << endl;
            broken = true;
            break;
         }      
      }
      if(broken != true){
         int d = dirs.size() - 1;
         inode_ptr dir = state.get_cwd()->get_contents()->
                                       mkdir(dirs.at(d), state);
         state.get_cwd()->get_contents()->get_dirents().
                                       insert({dirs.at(d),dir});
      }
      state.set_cwd(backup);
   }
   else{
      inode_ptr dir = state.get_cwd()->get_contents()->
                                    mkdir(words.at(1), state);
      state.get_cwd()->get_contents()->get_dirents().
                                    insert({words.at(1),dir});
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string p = "";
   for(int i = 1; i < words.size(); i++){
      if(i != words.size()-1)
         p += words.at(i) + " ";
      else
         p += words.at(i);
   }
   state.set_prompt(p);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string out = state.get_cwd()->get_path();
   cout << out << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   std::map<string,inode_ptr> m;
   m = state.get_cwd()->get_contents()->get_dirents();
   auto del = m.find(words.at(1));
   if(del != m.end()){
      // if plain fill set ptr to null and it will garbage collected
      // if dir clear all entries, then dots and then ptr from parent
      inode_ptr node = del->second;
      if(node->get_file_type() == 'p'){
         state.get_cwd()->get_contents()->
                  get_dirents().erase(words.at(1));
         node = NULL;
      }
      else
         cerr << "error: can not call rm on a directory" << endl;
   }
   else 
      cerr << "error: pathname does not exist" << endl;
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string dot = ".";
   string dotdot = "..";
   inode_ptr backup = state.get_cwd();
   std::map<string, inode_ptr> map;
   map = state.get_cwd()->get_contents()->get_dirents();
   auto del = map.find(words.at(1));  
   inode_ptr node = del->second;
   if(node->get_file_type() == 'd'){
      std::map<string, inode_ptr> dmap;
      dmap = node->get_contents()->get_dirents();
      for(auto p: dmap){
         string name = p.first;
         inode_ptr dnode = p.second;
         if(name != dot and name != dotdot and 
                     dnode->get_file_type() == 'd'){
            wordvec words2;
            words2.push_back("rmr");
            words2.push_back(name);
            state.set_cwd(dnode);
            fn_rmr(state,words2);
            state.set_cwd(backup);
         }
      }
      state.get_cwd()->get_contents()->get_dirents().
                                    erase(words.at(1));
   }
   else
      state.get_cwd()->get_contents()->get_dirents().
                                    erase(words.at(1));
}
