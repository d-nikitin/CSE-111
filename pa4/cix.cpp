// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $
//Dmitriy Nikitin PA4

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "fstream"
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"put", cix_command::PUT  },
   {"get", cix_command::GET  },
   {"rm", cix_command::RM    },

};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}
//helper function to copy string to char*
void copy_name(cix_header& header, string name){
   int i; 
   for(i = 0; i < name.length(); ++i)
      header.filename[i] = name[i];
   header.filename[i] = '\0';
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      size_t host_nbytes = ntohl (header.nbytes);
      auto buffer = make_unique<char[]> (host_nbytes + 1);
      recv_packet (server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_get(client_socket& server, string name){
   cix_header header;
   header.command = cix_command::GET;
   copy_name(header, name);
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if(header.command != cix_command::FILEOUT){
      outlog << "sent GET, server did not return FILEOUT" << endl;
      outlog << "server returned " << header << endl;
   }
   else{
      size_t bytes  = header.nbytes;
      char buffer[bytes+1];
      recv_packet(server,buffer,bytes);
      outlog << "received " << bytes << " bytes" << endl;
      buffer[bytes] = '\0';
      ofstream stream(header.filename, ofstream::binary);
      stream.write(buffer, bytes);
      stream.close();
   }
}

void cix_put(client_socket& server, string name){
   cix_header header;
   copy_name(header, name);
   ifstream stream (header.filename, ifstream::binary);
   if(stream){
      stream.seekg(0,stream.end);
      size_t bytes = stream.tellg();
      stream.seekg(0, stream.beg);
      char buffer[bytes];
      header.command = cix_command::PUT;
      header.nbytes = bytes;
      outlog << "sending header " << header << endl;
      send_packet (server, &header, sizeof header);
      send_packet(server, buffer, bytes);
      recv_packet (server, &header, sizeof header);
      outlog << "received header " << header << endl;
   }
   else 
      outlog << "file " << name << " not found" << endl;
 
   if(header.command != cix_command::ACK){
      outlog << "sent PUT, server did not return ACK" << endl;
      outlog << "server returned " << header << endl;
   }
   stream.close();
}

void cix_rm(client_socket& server, string name){
   cix_header header;
   copy_name(header, name);
   header.command = cix_command::RM;
   header.nbytes = 0;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if(header.command != cix_command::ACK){
      outlog << "Error: file " << name << " doesn't exsist" << endl;
   }
}


void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         string com;
         if(line.find(" ") != string::npos)
            com = line.substr(0,line.find_first_of(' ',0));
         else
            com = line;         
         outlog << "command " << com << endl;
         const auto& itor = command_map.find (com);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         string name = line.substr(line.find_first_of(' ' , 0)+1,
                                                       string::npos);
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::PUT:
               cix_put (server, name);
               break;
            case cix_command::GET:
               cix_get (server, name);
               break;
            case cix_command::RM:
               cix_rm (server, name);
               break;
            default:
               outlog << com << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}
