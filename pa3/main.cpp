// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $
//Dmitriy Nikitin ID: 1649206 asg3

#include <cstdlib>
#include <exception>
#include <iostream>
#include<fstream>
#include <string>
#include <regex>
#include <unistd.h>
#include <cassert>

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};

   str_str_map map;
   string line;
   int count = 1;
   if(argc > 1){// file was specified
      for(int i = 1; i < argc; i++){
         std::fstream file;
         string file_name = argv[i];
         file.open(file_name);
         if(file.is_open()){
            while(getline(file,line)){
               if (cin.eof()) break;
               smatch result;
               if (regex_search (line, result, comment_regex)) {
                  cout << file_name << ": " << count << ": " 
                                             << line << endl;
                  count++;
                  continue;
               }
               else if (regex_search (line, result, key_value_regex)) {
                  cout << file_name << ": " << count << ": " 
                                             << line << endl;
                  if(result[1] != "" and result[2] == ""){
                     auto it = map.find(result[1]);
                     if(it != map.end()){
                       it = map.erase(it);
                     }
                     else{
                       cerr << result[1] << ": key not found" << endl; 
                     }
                   }
                  else if(result[1] != "" and result[2] != ""){
                     map.insert(str_str_pair(result[1],result[2]));
                     cout << str_str_pair(result[1],result[2]) << endl;
                  }
                  else if(result[1] == "" and result[2] == ""){
                     auto it = map.begin();
                     while(it != map.end()){
                        cout << *it << endl;
                        ++it;
                     }
                  }
                  else if(result[1] == "" and result[2] != ""){
                     auto it = map.begin();
                     while(it != map.end()){
                        if((*it).second == result[2])
                           cout << *it << endl;
                        ++it;
                     }
                  }
               }
               else if (regex_search (line, result, trimmed_regex)) {
                  cout << file_name << ": " << count << ": " 
                                                << line << endl;
                  auto it = map.find(result[1]);
                  if(it != map.end())
                     cout << *it << endl;
                  else
                     cerr << result[1] << ": key not found" << endl;
               }
               else {
                  assert (false and "This can not happen.");
               }
               count++;
            }
         }
         else{
            cerr << "keyvalue: " << file_name 
                     << ": No such file or directory" << endl;
         }
         file.close();
      }
   }
   else{// we read from the console
      while (getline (cin, line)) {
         if (cin.eof()) break;
         smatch result;
         if (regex_search (line, result, comment_regex)) {
            cout << "-: " << count << ": " << line << endl;
            continue;
         }
         else if (regex_search (line, result, key_value_regex)) {
            cout << "-: " << count << ": " << line << endl;
            if(result[1] != "" and result[2] == ""){
               auto it = map.find(result[1]);
               if(it != map.end()){
                 it = map.erase(it);
               }
               else{
                 cerr << result[1] << ": key not found" << endl; 
               }
             }
            else if(result[1] != "" and result[2] != ""){
               map.insert(str_str_pair(result[1],result[2]));
               cout << str_str_pair(result[1],result[2]) << endl;
            }
            else if(result[1] == "" and result[2] == ""){
               auto it = map.begin();
               while(it != map.end()){
                  cout << *it << endl;
                  ++it;
               }
            }
            else if(result[1] == "" and result[2] != ""){
               auto it = map.begin();
               while(it != map.end()){
                  if((*it).second == result[2])
                     cout << *it << endl;
                  ++it;
               }
            }
         }
         else if (regex_search (line, result, trimmed_regex)) {
            cout << "-: " << count << ": " << line << endl;
            auto it = map.find(result[1]);
            if(it != map.end())
               cout << *it << endl;
            else
               cerr << result[1] << ": key not found" << endl;
         }
         else {
            assert (false and "This can not happen.");
         }
         count++;
      }
   }
   map.~listmap();
   return EXIT_SUCCESS;
}
