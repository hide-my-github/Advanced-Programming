// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

#include <iostream>
#include <stdexcept>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};

//inode methods*****************************************************

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case PLAIN_INODE:
           contents = make_shared<plain_file>();
           break;
      case DIR_INODE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}
/*
const map<string,inode_ptr>& inode::get_contents(){
	return directory_ptr_of(contents)->get_dirents();
}
*/
const file_base_ptr& inode::get_contents(){
	return contents;
}

size_t inode::get_size() {
	if (get_type() == DIR_INODE) {
		return (directory_ptr_of(get_contents()))->get_dirents().size();
	} else {
		return (plain_file_ptr_of(get_contents()))->size();
	}
}

inode_t inode::get_type(){
	return type;
}
//filebase mathods*****************************************************


plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
   plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
   if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
   return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}


size_t plain_file::size() const {
   size_t size {};
   for(auto& words : data){
	   size += words.size();
   }
   if(data.size()!=0){
	   size += data.size()-1 ; //takes into account spaces between strings!
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
	data = words;
}

const map<string,inode_ptr>& directory::get_dirents(){
	return dirents;
}

//directory Methods*****************************************************
directory::directory(){
	dirents.insert({".",nullptr}); 
	dirents.insert({"..",nullptr});
}

size_t directory::size() const {
   size_t size {dirents.size()};
   DEBUGF ('i', "size = " << size);
   return size;
}
void directory::setDot(inode_ptr ptr){
  dirents.at(".") = ptr; //result: map<".", ptr>
}

void directory::setDotDot(inode_ptr ptr){
  dirents.at("..") = ptr; //result: map<"..", ptr>
}

void directory::remove (const string& filename) {
	
	auto elem = dirents.find(filename);
    if(elem != dirents.end()){
		(*elem).second = nullptr;
		dirents.erase(elem);
    }else{
		throw yshell_exn("no such file / directory");
    }
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir(const string& dirname){
	inode_ptr newDir = make_shared<inode>(DIR_INODE);
	dirents.insert({dirname, newDir});
	return newDir;
}

inode_ptr directory::mkfile(const string& fileName){
   inode_ptr newFile = make_shared<inode>(PLAIN_INODE);
   dirents.insert({fileName, newFile});
   return newFile;
}
//InodeState Methods*****************************************************

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt << "\"");
   root = make_shared<inode>(DIR_INODE);
   setCwdTo(root);
   directory_ptr_of(root->contents)->setDot(root);
   directory_ptr_of(root->contents)->setDotDot(root);
}

void inode_state::setCwdTo(inode_ptr ptr){
	cwd = ptr;
}

void inode_state::setCwdToRoot(){
	cwd = root;
}

void inode_state::changePrompt(const wordvec& words){
	wordvec strVec = words;
	strVec.erase(strVec.begin());
	string hold = "";
	for(auto& str : strVec)
		hold += str+" ";
	prompt = hold;
}
	  
const string& inode_state::spitPrompt(){
	return prompt;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode_ptr inode_state::getCwd(){
	return cwd;
}

inode_ptr inode_state::getRoot(){
	return root;
}