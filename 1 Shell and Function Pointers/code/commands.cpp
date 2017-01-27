// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

#include "commands.h"
#include "debug.h"
#include <iomanip>

commands::commands(): map ({
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
   {"rmr"   , fn_rmr   },
}){}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}


void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if(words.size() == 1){
	   cout<<"ERROR: cat: missing filename"<<endl;
	   return;
   }

   inode_ptr currPtr = state.getCwd();
   if(words.size() > 2){
		for(int i=1; i<(int)words.size(); i++){
			if( directory_ptr_of(currPtr->get_contents())->get_dirents().count(words.at(i)) == 0){
				cout<<"cat: "<<words.at(1)<<": No such file or directory"<<endl;
				return;
			}
			auto file = directory_ptr_of(currPtr->get_contents())->get_dirents().find(words.at(i));
			if(file->second->get_type() != DIR_INODE){
				wordvec text = plain_file_ptr_of(file->second->get_contents())->readfile();
				cout<<text<<endl;
			}
			else{
			   cout<<words.at(i)<<" is not a file"<<endl;
			   return;
			}
		}
	}
	else{
		if( directory_ptr_of(currPtr->get_contents())->get_dirents().count(words.at(1)) == 0){
			cout<<"cat: "<<words.at(1)<<": No such file or directory"<<endl;
			return;
		}
		auto file = directory_ptr_of(currPtr->get_contents())->get_dirents().find(words.at(1));
		if(file->second->get_type() != DIR_INODE){
			wordvec text = plain_file_ptr_of(file->second->get_contents())->readfile();
			cout<<text<<endl;
		}
		else{
		   cout<<words.at(1)<<" is not a file"<<endl;
		   return;
		}		
	}


}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   

	inode_ptr currPtr = state.getCwd();
	if(words.size() == 1 || words.at(1) == "/"){
		state.setCwdToRoot();
	}
	else{
		vector<string> path = split (words.at(1), "/"); //words == "/foo/bar/baz/" then path.at(0) == foo, etc.
		if(words.at(1).at(0) == '/'){ //search from root
			for(auto& itor: path){
				//cout<<": "+itor<<endl;
				if(directory_ptr_of(currPtr->get_contents())->get_dirents().count(itor) == 0) //directory to go to is NOT found
				{
					cout<<"ERROR: directory path not found"<<endl;
					return;
				}
				currPtr = directory_ptr_of(currPtr->get_contents())->get_dirents().find(itor)->second;
				state.setCwdTo(currPtr);
			}
		}
		else{ //search in cwd
			for(auto& itor: path){
				//cout<<": "+itor<<endl;
				if(directory_ptr_of(currPtr->get_contents())->get_dirents().count(itor) == 0) //directory to go to is NOT found
				{
					cout<<"ERROR: directory path not found"<<endl;
					return;
				}
				currPtr = directory_ptr_of(currPtr->get_contents())->get_dirents().find(itor)->second;
				state.setCwdTo(currPtr);
			}
		}
	}
}

void fn_echo (inode_state& state, const wordvec& words){ //TESTED
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   for(int i = 1; i!= (int)(words.size()); i++){
	   cout<<words.at(i)+" ";
   }
   cout<<endl;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size() == 1){ //127
	    exit_status::set(127);
		throw ysh_exit_exn();
   }
	else{
		int value = atoi(words.at(1).c_str());
		exit_status::set(value);
		throw ysh_exit_exn();
	}

}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   	if (words.size() > 2) {
	   cout << "fn_ls: Too many operands" << endl << "Usage: ls [pathname]\n" << endl;   
	} else if (words.size() == 1) {
		print_path_info(state, (getPathName(state.getCwd()) == getPathName(state.getRoot())) ? "" : getPathName(state.getCwd()));
	} else { //we have a directory, check if valid directory name
		if(isValidPath(state, words.at(1))) {
			print_path_info(state, words.at(1)); //passes the whole pathname /foo/bar/baz
		} else {
		   cout << "Invalid pathname: " << words.at(1) << endl;
		}
	}
}

void fn_lsr (inode_state& state, const wordvec& words){
    DEBUGF ('c', state);
    DEBUGF ('c', words);
    //re-make words.at(1) into a wordvec called path with first element "" and second element the path name of cwd.
	inode_ptr hold = state.getCwd();
	if(words.size() == 2)
	{
		//traverse down the path
		if(isValidPath(state, words.at(1)))
			state.setCwdTo(get_path_node(state, words.at(1)));
	}
	else if (words.size() > 2){
		for(auto& elem: words){
			if(elem != "lsr"){
				wordvec path{"lsr", elem};
				fn_lsr(state, path);
			}
		}
		return;
	}
	wordvec here{""};
    fn_ls(state, here); //show current ls, here.size() == 1
	inode_ptr currPtr = state.getCwd();
	for(auto& elem : directory_ptr_of(currPtr->get_contents())->get_dirents())
	{
		if(elem.first != "."){
			if(elem.first != ".."){
				if(elem.second->get_type() == DIR_INODE)
				{
					wordvec path{elem.first};
					state.setCwdTo(elem.second);
					fn_lsr(state, path);
				}
			}
		}
	}
	
    state.setCwdTo(hold);
}


void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   	if(words.size() < 2){
		cout<<"Usage: make [pathname] [words...]\n";
		return; 
	}
	wordvec text;
	inode_ptr currPtr = {nullptr};
	for(auto iter=words.begin()+2; iter!=words.end(); iter++){
		text.push_back(*iter);
	}
	wordvec pathVector = split (words.at(1), "/"); //check to see if theres a specific path
	string fileName = pathVector.back();
	pathVector.pop_back(); //pops the last entry which is the text filename we are creating
	if(pathVector.size() == 0){ //no path, create in cwd
		currPtr = state.getCwd();
	}
	else //there is a path that we must take and create at
	{
		string pathString = words.at(1);
		pathString.erase( pathString.size()-fileName.size() ,fileName.size() );
		if(isValidPath(state, pathString)) 
			currPtr = get_path_node(state, pathString); 
		else{
			cout<<"ERROR: no existing path!"<<endl;
			return;
		}
	}
	for(auto& elem: directory_ptr_of(currPtr->get_contents())->get_dirents())
	{
		if(elem.first == fileName)
		{
			cout<<"ERROR: pathname already exists"<<endl;
			return;
		}
	}
	inode_ptr newFile = directory_ptr_of(currPtr->get_contents())->mkfile(fileName);
	plain_file_ptr_of(newFile->get_contents())->writefile(text);
}

void fn_mkdir (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);

   	if(words.size() < 2){
		cout<<"Usage: make [pathname] [words...]\n";
		return; 
	}
	inode_ptr currPtr = {nullptr};
	wordvec pathVector = split (words.at(1), "/"); //check to see if theres a specific path
	string fileName = pathVector.back();
	pathVector.pop_back(); //pops the last entry which is the text filename we are creating
	if(pathVector.size() == 0){ //no path, create in cwd
		currPtr = state.getCwd();
	}
	else //there is a path that we must take and create at
	{
		string pathString = words.at(1);
		pathString.erase( pathString.size()-fileName.size() ,fileName.size() );
		if(isValidPath(state, pathString)) 
			currPtr = get_path_node(state, pathString); 
		else{
			cout<<"ERROR: no existing path!"<<endl;
			return;
		}
	}
	for(auto& elem: directory_ptr_of(currPtr->get_contents())->get_dirents())
	{
		if(elem.first == words.at(1))
		{
			cout<<"ERROR: directory already exists"<<endl;
			return;
		}
	}

	inode_ptr newDir = directory_ptr_of(currPtr->get_contents())->mkdir(fileName);
	directory_ptr_of(newDir->get_contents())->setDot(newDir);
	directory_ptr_of(newDir->get_contents())->setDotDot(currPtr);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.changePrompt(words);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout<<getPathName(state.getCwd())<<endl;
}

void fn_rm (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	inode_ptr currPtr = state.getCwd();
	if(words.size() == 1) //TESTED
	{
		cout<<"ERROR: rm: missing operand"<<endl;
		return;
	}
	if(words.at(1) == "." || words.at(1)=="..") //TESTED
	{
		cout<<"ERROR: Cannot remove . or .."<<endl;
		return;
	}
	if(words.at(1) == "/") //TESTED
	{
		cout<<"ERROR: Cannot remove /"<<endl;
		return;
	}
	for(auto& elem: directory_ptr_of(currPtr->get_contents())->get_dirents())
	{
		if(elem.first == words.at(1))
		{
			if(elem.second->get_type() == DIR_INODE){
				if(directory_ptr_of(elem.second->get_contents())->get_dirents().size() > 2)
				{
					cout<<"ERROR: this directory is not empty!"<<endl;
					return;
				}
			}
			directory_ptr_of(currPtr->get_contents())->remove(elem.first);
			return;
	   }
	}
	cout<<"ERROR: filename not found, didn't remove anything!"<<endl;
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if(words.size()==1)
   {
	   cout<<"Usage: rmr [pathname] \n";
	   return;
   }
	inode_ptr save = state.getCwd(); 
	if(words.size() == 2)
	{
		//traverse down the path
		if(isValidPath(state, getPathName(state.getCwd())+"/"+words.at(1))){
			state.setCwdTo(get_path_node(state, words.at(1)));
		}
	}
	else if(words.size() > 2)
	{
		cout<<"ERROR"<<endl;
		return;
	}
	inode_ptr currPtr = state.getCwd();
	for(auto& iter: directory_ptr_of( currPtr->get_contents() )->get_dirents() ){
		if(iter.first != "." and iter.first != ".."){
			if(iter.second->get_type() == DIR_INODE and iter.second->get_size() > 2){
				wordvec blah{"rmr" , iter.first};
				fn_rmr(state,  blah);
			}
			else{
				wordvec blah{"rmr" , iter.first};
				fn_rm(state,  blah);
			}
		}
	}
	state.setCwdTo(save);
	fn_rm(state, words);
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

string getPathName(inode_ptr ptr){
	  int currInodeNr = ptr->get_inode_nr(); //initialize a variable to hold my current inode number
      vector<string> pathVector; //initialize a vector of type string
      while(currInodeNr != 1){ //while currently NOT at root
         ptr = directory_ptr_of(ptr->get_contents())->get_dirents().at(".."); //shifts ptr to point at its parent
         for(auto& elem: directory_ptr_of(ptr->get_contents())->get_dirents()){ //elem is the MAP of parent of where we are
            if(elem.second->get_inode_nr() == currInodeNr && elem.first != "."){ //in elem (a map), the inode_nr of a pointer to a directory is the same as mine
               pathVector.insert(pathVector.begin(),elem.first); //insert the name correlating to that matching pointer
            }
         }
         currInodeNr = ptr->get_inode_nr(); //after iterating through the parent's map, set my current inode number to that of its parent
      }
      string pathString = ""; //initialize a string to pass back to the calling function
      for(auto& str : pathVector){ //while pathVector is NOT empty, iterate through it
        pathString += "/"+str; //insert the strings at each location followed by a "/"
      }
	  if(pathString == "")
		  pathString += "/";
      return pathString; //return the completed path string

}

bool isValidPath(inode_state& state, string pathname) {
    wordvec pathVector = split (pathname, "/");
    inode_ptr current_inode = state.getRoot();
	bool found = false;
	if((int)pathVector.size() == 0)
		return true;
    for (int depth = 0; depth < (int)pathVector.size(); ++depth) {
        string current_dir = pathVector.at(depth);
        for (auto& elem: directory_ptr_of(current_inode->get_contents())->get_dirents()){
            if (elem.first.compare(current_dir) == 0)
                found = true;
        }
		if(found == false)
			return found;
    } 
	return found;
}

void print_path_info (inode_state& state, string pathname) {
    //cout << "print_path_info: running, pathname is: "<<pathname<<endl;
    inode_ptr current_inode = state.getRoot();
	wordvec pathVector = split(pathname, "/");
    if (pathname.empty()) { //if there is no pathname (user simply called ls by itself)
     //   cout << "[empty pathname]" << endl;
        current_inode = state.getCwd(); //we set the current_inode equal to cwd.
    } else{ //pathname is something some path
        current_inode = get_path_node(state, pathname);
    }
    //now print out the numbers and sizes and names for the directory
    if (pathname.empty())
        cout << "/";
    else if (pathname == "." or pathname == "..")
        cout << "/";
    else
        cout << pathname;
    cout  << ":" << endl;
    for (auto& elem: directory_ptr_of(current_inode->get_contents())->get_dirents()){
        current_inode = elem.second;
        cout<<right<<setw(8)<<current_inode->get_inode_nr()
            <<right<<setw(8)<<current_inode->get_size()<<" "
            <<left<<elem.first;
		if(elem.second->get_type() == DIR_INODE and elem.first != "." and elem.first != "..")
			cout<<"/";
		cout<<endl;
    }
}

inode_ptr get_path_node (inode_state& state, string pathname) {
	inode_ptr traversal_node = {nullptr};
	if(pathname.at(0) == '/')
		traversal_node = state.getRoot();
	else
		traversal_node = state.getCwd();
	
	wordvec path_vector = split (pathname, "/");
    for (int depth = 0; depth < (int)path_vector.size(); ++depth) {
        traversal_node = directory_ptr_of(traversal_node->get_contents())->get_dirents().at(path_vector.at(depth));
    }
    return traversal_node;
}

void recursive_ls (inode_ptr path_node, inode_state& state, const wordvec& path) {
    wordvec new_path = path;
    int entry_num = 0;
    for (auto current : directory_ptr_of(path_node->get_contents())->get_dirents()) {
        cout << "recursive_ls entry_num: " << entry_num << endl;
        if (entry_num < 2) {
            ++entry_num;
        } else if (current.second->get_type() == DIR_INODE) {
            string next_target = get_inode_name(current.second);
            new_path.push_back(next_target);
            recursive_ls(current.second, state, new_path);
            fn_ls(state, new_path);
            new_path.pop_back();
        }
    }
}

string get_inode_name (inode_ptr ptr) {
    inode_ptr parent = directory_ptr_of(ptr->get_contents())->get_dirents().at("..");
    for (auto& parent_dirents : directory_ptr_of(parent->get_contents())->get_dirents()) {
        if (parent_dirents.second == ptr) {
            if (parent_dirents.first == ".")
                return "/";
            return parent_dirents.first;
        }
    } return "Error: get_inode_name: ptr not found (this shouldn't be possible)";
}