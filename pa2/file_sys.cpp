// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $
// Dmitriy Nikitin ID: 1649206 PA2

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
  this->root = make_shared<inode>(file_type::DIRECTORY_TYPE) ;
  this->root->set_path("/");
  this->root->inode_nr = 1;
  this->cwd = this->root;
  this->root->get_contents()->get_dirents().insert({".",root});
  this->root->get_contents()->get_dirents().insert({"..",root});

  DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           file_t = 'p';
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           file_t = 'd';
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
  throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
  throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
  throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&, inode_state& state) {
  throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&, inode_state& state) {
  throw file_error ("is a " + error_file_type());
}
map<string, inode_ptr>& base_file::get_dirents(){
  throw file_error ("is a " + error_file_type());
}
void base_file::set_data(wordvec& that){
  throw file_error ("is a " + error_file_type());
}
void base_file::set_name(string n){
  throw file_error ("is a " + error_file_type());
}
bool base_file::is_Empty(){
  throw file_error ("is a " + error_file_type());
}
string base_file::get_name(){
  throw file_error ("is a " + error_file_type());
}

//plain files

size_t plain_file::size() const {
  size_t size = 0;
  for(int i = 0; i < data.size(); i++)
    size += data.at(i).size();
  size += data.size() -1;
  DEBUGF ('i', "size = " << size);
  return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', this->data);
   return this->data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   this->data.clear();
   this->data = words;
}

//Directories 

size_t directory::size() const {
   size_t size = dirents.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   this->get_dirents().erase(filename);
}

inode_ptr directory::mkdir 
    (const string& dirname, inode_state& state) {
  DEBUGF ('i', dirname);
  inode_ptr out = make_shared<inode>(file_type::DIRECTORY_TYPE);
  out->set_parent(state.get_cwd());
  out->get_contents()->set_name(dirname);
  string curr_path = "";
  if(state.get_cwd() != state.get_root())
     curr_path = (state.get_cwd()->get_path() + "/" + dirname);
  else
     curr_path = ("/" + dirname);
  out->set_path(curr_path);
  out->get_contents()->get_dirents().insert
                                ({".",out});
  out->get_contents()->get_dirents().insert 
                                ({"..",out->get_parent()});
  return out;
}

inode_ptr directory::mkfile 
    (const string& filename, inode_state& state) {
   DEBUGF ('i', filename);
   inode_ptr file = make_shared<inode>(file_type::PLAIN_TYPE);
   file->get_contents()->set_name(filename);
   file->set_parent(state.get_cwd());
   string curr_path = (state.get_cwd()->get_path() + "/" + filename);
   file->set_path(curr_path);
   return file;
}

bool directory::is_Empty(){
  if(this->size() == 2) return true;
  return false;
}
