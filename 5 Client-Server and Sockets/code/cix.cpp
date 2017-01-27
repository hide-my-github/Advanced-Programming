//Authors: Nicholas N. Church, nnchurch@ucsc.edu
//Josh Shih, jtshih@ucsc.edu
//6/5/15
//CMPS109 Spring 2015
// $Id: cix.cpp,v 1.2 2015-05-12 18:59:40-07 - - $

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"get" , CIX_GET },
   {"put" , CIX_PUT },
   {"rm", CIX_RM }
};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = CIX_LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != CIX_LSOUT) {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.nbytes + 1];
      //cout << "buffer: " << buffer << endl;
      recv_packet (server, buffer, header.nbytes);
      //log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

void cix_rm(string filename, client_socket& server){
   cix_header header;
   header.command = CIX_RM;
   strcpy(header.filename, filename.c_str());
   log <<"sending header " << header << endl;
   send_packet(server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if(header.command != CIX_ACK) {
      log << "sent CIX_RM, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
   } else {
      log << header.filename << " removed." <<endl;
   }
}

void cix_get (string filename, client_socket& server) {
   cix_header header;
   header.command = CIX_GET;
   strcpy(header.filename, filename.c_str());
   send_packet (server, &header, sizeof header);
   log << "sending header " << header << endl;
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl; 
   char buffer[0x1000];
   if (header.command != CIX_FILE) {
      log << "sent CIX_GET, server did not return CIX_FILE" << endl;
      log << "server returned " << header << endl;
   }else {
      recv_packet (server, buffer, header.nbytes);
      //log << "received " << header.nbytes << " bytes" << endl;
      ofstream myfile (header.filename, ios::binary | ios::trunc);
      if (myfile.is_open()) {
         myfile.write(buffer, header.nbytes);
         myfile.close();
      } else log << "cix_get: unable to open local file." << endl;
   }
}

void cix_put (string filename, client_socket& server) {
   cix_header header;
   strcpy(header.filename, filename.c_str());
   header.command = CIX_PUT;
   ifstream infile(header.filename, ifstream::binary | ifstream::ate);
   if (!(infile.is_open())){
		log << (filename + " is a bad input file\n"); 
		return;
   }
   header.nbytes = infile.tellg();
   char memblock[header.nbytes];
   infile.seekg(0, ios::beg);
   infile.read(memblock, header.nbytes);
   infile.close();

   send_packet (server, &header, sizeof header);
   send_packet(server, memblock, header.nbytes);
   recv_packet (server, &header, sizeof header);
   if (header.command != CIX_ACK) {
      log << "sent CIX_PUT, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
   } 
}

void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host;
   in_port_t port;
   if(args.size() == 1){ //cix [port]
      host = get_cix_server_host(args, 3);
      port = get_cix_server_port (args, 0);
   } else {
      host = get_cix_server_host (args, 0);
      port = get_cix_server_port (args, 1);
   }
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line, command, filename;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         command = line.substr(0, line.find(" "));
         if (line.length() != command.length())
            filename = line.substr(command.length()+1, line.length());
         log << "command " << command << endl;
         const auto& itor = command_map.find (command);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
         switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_PUT:
               cix_put (filename, server);
               break;
            case CIX_GET:
               cix_get (filename, server);
               break;
         case CIX_RM:
            cix_rm(filename, server);
            break;
            default:
               log << command << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

