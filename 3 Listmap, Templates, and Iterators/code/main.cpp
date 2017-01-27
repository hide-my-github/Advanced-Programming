// $Id: main.cpp,v 1.8 2015-04-28 19:23:13-07 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fstream>

using namespace std;

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
            traceflags::setflags (optarg);
            break;
        default:
            complain() << "-" << (char) optopt << ": invalid option"
            << endl;
            break;
        }
    }
}

void trim (string str) {
    if (str.size() > 0) {
        size_t first = str.find_first_not_of(' ');
        size_t last = str.find_last_not_of(' ');
        str = str.substr(first, (last-first+1));
    }
}

void parse_input (str_str_map& map, string instring) {
    //find first occurence of '='
    size_t first = instring.find_first_of('=');
    //'#' is the first character
    if (instring.find_first_of('#') == 0 or instring.length() == 0) {
        //do nothing, this is a comment or blank line
		//cout<<"COMMENT DETECTED"<<endl;
    }
    //'=' is not found in the string so only a key has been passed in
    else if (first == string::npos) {
        //search through str_str_map map for the key instring
        auto itor = map.find(instring); 
        if (itor == map.end()) {
            cout << instring << ": key not found\n";
        }
    }
    //'=' is the last character in the input string
    else if (instring.length() > 1 and first == instring.length()-1) { // possibly off by 1, add one to length
        instring.pop_back(); //get rid of the '='
        auto itor = map.find(instring);
        if (itor != map.end()) {
            cout << "attempting to delete the found key/value pair\n";
            map.erase(itor);
        } else {
            cout << "key not found\n";
        }
        
    }
    //there is also a value in the passed in string i.e. key=value
    else if (first != 0 and first != instring.length()-1) {
        string key = instring.substr(0, first);
        string value = 
            instring.substr(key.length()+1, instring.length()-1);
        map.insert({key, value});
        cout<<key<<" = "<<value<<endl;
        //cout<<"out of else if"<<endl;
    }
    //the '=' is the only character in the string
    else if (first == 0 and instring.length() == 1) {
        //cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
        //print out all of the values
        for (auto itor = map.begin(); itor != map.end(); ++itor) {
            cout << itor->first << " = " << itor->second << endl;
        }
    }
    //the '=' is the first character and there is a value as well
    else if (first == 0 and instring.length() != 1) {
        string value = instring.substr(1, instring.length());
        for (auto itor = map.begin(); itor != map.end(); ++itor) {
            //cout << endl << "itor->second: " << itor->second << endl;
            if (itor->second == value)
                cout <<itor->first <<" = "<<value<< endl;
        }
    }
    else {
        cout << "unknown input format\n";
    }
}

int main (int argc, char** argv) {
    sys_info::set_execname (argv[0]);
    scan_options (argc, argv);

    str_str_map test;
    
    string input_string{};

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            //open infile in read-only mode.
            int command_number = 1;
            ifstream infile;
            infile.open(argv[i]);
            if (infile.is_open()) {
                //if the file is non-null, do stuff.
                while(getline(infile, input_string)) {
                    //trim off trailing whitespace
                    trim(input_string);
                    cout << argv[i]<<": "
                        << command_number << ": " 
                        << input_string << endl;
                    ++command_number;
                    parse_input(test, input_string);
                }
            } else {
                cout << argv[i] << ":: bad input file" << endl;
            }
            infile.close();
        }
    } else {
        //no input files so use cin
        while(cin >> input_string) {
            trim(input_string);
            parse_input(test, input_string);
        }
    }
    return 0;
}

