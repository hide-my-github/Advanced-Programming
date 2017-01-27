//Authors: Nicholas N. Church, nnchurch@ucsc.edu
//Josh Shih, jtshih@ucsc.edu
//6/5/15
//CMPS109 Spring 2015
// $Id: cixd.cpp,v 1.3 2015-05-12 19:06:46-07 - - $

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

void reply_ls (accepted_socket& client_sock, cix_header& header) {
   FILE* ls_pipe = popen ("ls -l", "r");
   if (ls_pipe == NULL) { 
      log << "ls -l: popen failed: " << strerror (errno) << endl;
      header.command = CIX_NAK;
      header.nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
   }
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   header.command = CIX_LSOUT;
   header.nbytes = ls_output.size();
   memset (header.filename, 0, FILENAME_SIZE);
   //log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   pclose(ls_pipe);
   //log << "sent " << ls_output.size() << " bytes" << endl;
}
void do_rm(accepted_socket& client_sock, cix_header& header) {
   
   string file = "unlink ";
   file += header.filename;
   //file += ")";
   char* blah = new char[file.size()];
   strcpy(blah, file.c_str());
    FILE* rm_pipe = popen (blah, "w");
   header.command = CIX_ACK;
   header.nbytes = 0; 
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header); 
   log << "sent 0 bytes" << endl;
   pclose(rm_pipe);
}

void do_get (accepted_socket& client_sock, cix_header& header) {
   string filename = header.filename;
   ifstream server_file (header.filename, ios::binary | ios::ate);
   if (!(server_file.is_open())){
	    header.nbytes = 0;
		header.command = CIX_NAK;
	    send_packet(client_sock, &header, sizeof header);
		log << header.filename << " is a bad input file\n"; 
		return;  
   }
   header.nbytes = server_file.tellg();
   char memblock[header.nbytes];
   server_file.seekg(0, ios::beg);
   if (server_file.is_open()) {
      server_file.read(memblock, header.nbytes);
      server_file.close();
   } else throw ("do_get: unable to open file" + filename);

   header.command = CIX_FILE;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, memblock, header.nbytes);
   log << "do_get::sent " << header.nbytes << " bytes" << endl;
}

void do_put(accepted_socket& client_sock, cix_header& header){
   ofstream myfile (header.filename, 
         ofstream::binary | ofstream::trunc);
   char buffer[0x1000];
   recv_packet(client_sock, buffer, header.nbytes);
   if (myfile.is_open()) {
      header.command = CIX_ACK;
      send_packet(client_sock, &header, sizeof header);
   } else {
      header.command = CIX_NAK;
      send_packet(client_sock, &header, sizeof header);
      log << "Unable to open file for output." << endl;
   }
   myfile.write(buffer, header.nbytes);
   myfile.close();
}


void run_server (accepted_socket& client_sock) {
   log.execname (log.execname() + "-server");
   log << "connected to " << to_string (client_sock) << endl;
   try {   
      for (;;) {
         cix_header header; 
         recv_packet (client_sock, &header, sizeof header);
         log << "received header " << header << endl;
         switch (header.command) {
            case CIX_LS: 
               reply_ls (client_sock, header);
               break;
         case CIX_PUT:
            do_put(client_sock, header);
            break;
         case CIX_GET:
            do_get(client_sock, header);
            break;
         case CIX_RM:
            do_rm(client_sock, header);
            break;
            default:
               log << "invalid header from client" << endl;
               log << "cix_nbytes = " << header.nbytes << endl;
               log << "cix_command = " << header.command << endl;
               log << "cix_filename = " << header.filename << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   throw cix_exit();
}

void fork_cixserver (server_socket& server, accepted_socket& accept) {
   pid_t pid = fork();
   if (pid == 0) { // child
      server.close();
      run_server (accept);
      throw cix_exit();
   }else {
      accept.close();
      if (pid < 0) {
         log << "fork failed: " << strerror (errno) << endl;
      }else {
         log << "forked cixserver pid " << pid << endl;
      }
   }
}


void reap_zombies() {
   for (;;) {
      int status;
      pid_t child = waitpid (-1, &status, WNOHANG);
      if (child <= 0) break;
      log << "child " << child
           << " exit " << (status >> 8)
           << " signal " << (status & 0x7F)
           << " core " << (status >> 7 & 1) << endl;
   }
}

void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   reap_zombies();
}

void signal_action (int signal, void (*handler) (int)) {
   struct sigaction action;
   action.sa_handler = handler;
   sigfillset (&action.sa_mask);
   action.sa_flags = 0;
   int rc = sigaction (signal, &action, nullptr);
   if (rc < 0) log << "sigaction " << strsignal (signal) << " failed: "
                   << strerror (errno) << endl;
}


int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   signal_action (SIGCHLD, signal_handler);
   in_port_t port = get_cix_server_port (args, 0);
   try {
      server_socket listener (port);
      for (;;) {
         log << to_string (hostinfo()) << " accepting port "
             << to_string (port) << endl;
         accepted_socket client_sock;
         for (;;) {
            try {
               listener.accept (client_sock);
               break;
            }catch (socket_sys_error& error) {
               switch (error.sys_errno) {
                  case EINTR:
                     log << "listener.accept caught "
                         << strerror (EINTR) << endl;
                     break;
                  default:
                     throw;
               }
            }
         }
         log << "accepted " << to_string (client_sock) << endl;
         try {
            fork_cixserver (listener, client_sock);
            reap_zombies();
         }catch (socket_error& error) {
            log << error.what() << endl;
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

