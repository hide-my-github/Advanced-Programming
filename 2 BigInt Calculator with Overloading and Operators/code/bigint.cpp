// $Id: bigint.cpp,v 1.61 2014-06-26 17:06:06-07 - - $

#include <cstdlib>
#include <exception>
#include <limits>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;
using digit_t = unsigned char;
using bigvalue_t = vector<digit_t>;
#include "bigint.h"
#include "debug.h"

#define ZERO '0' 

bigint::bigint (const string& that) {
   auto itor = that.cbegin();
   if (itor != that.cend() and *itor == '_') {
      negative = true;
      ++itor;
   }
	  
   while (itor != that.cend()){
	   unsigned char u_itor = static_cast<unsigned char>(*itor);
	   big_value.insert( big_value.begin() , u_itor);
	   itor++;
   }
   
   DEBUGF ('~', this << " -> " << long_value)
}

bigint operator+ (const bigint& left, const bigint& right) {
	bigint sum;
	bigvalue_t sum_vec;
    
	if( left.is_neg() && !right.is_neg() ){
		sum_vec = sum.do_bigsub(right.get_big_value(), left.get_big_value());
		sum.set_big_value(sum_vec);
		if(!bigint::do_bigless(left.get_big_value(), right.get_big_value()))
			sum.toggle_neg();
	}
	else if( right.is_neg() && !left.is_neg() ){

		if(bigint::do_bigless(left.get_big_value(), right.get_big_value())){
			sum.toggle_neg();
			sum_vec = sum.do_bigsub(right.get_big_value(), left.get_big_value());
			sum.set_big_value(sum_vec);			
		}
		else{
			sum_vec = sum.do_bigsub(left.get_big_value(), right.get_big_value());
			sum.set_big_value(sum_vec);
		}
	}
	else if( right.is_neg() && left.is_neg() ){
		sum.toggle_neg();
		sum_vec = sum.do_bigadd(left.get_big_value(), right.get_big_value());
		sum.set_big_value(sum_vec);
	}
	else{
		sum_vec = sum.do_bigadd(left.get_big_value(), right.get_big_value());
		sum.set_big_value(sum_vec);
	}

	sum.set_big_value(sum_vec);
	//trim zeros.
	for(int i=sum_vec.size()-1; i>=0; i--){
	  if(sum_vec.at(i) == ZERO)
		  sum_vec.pop_back();
	  else
		  break;
	}
	sum.set_big_value(sum_vec);
	
   return sum;
}

bigint operator- (const bigint& left, const bigint& right) {
   bigint difference;
   bigvalue_t diff_vec;
   
   const bigint::bigvalue_t& left_vec = left.get_big_value();
   const bigint::bigvalue_t& right_vec = right.get_big_value();

   if(right.is_neg()){
       diff_vec = difference.do_bigadd(left_vec, right_vec);
       if(left.is_neg() and !bigint::do_bigless(left_vec, right_vec)){
           difference.toggle_neg();
       }
   } else if (!right.is_neg() and !left.is_neg() and bigint::do_bigless(left_vec, right_vec)){
      diff_vec = difference.do_bigsub(right_vec, left_vec);
      difference.toggle_neg();
   }
   else{
       diff_vec = difference.do_bigsub(left_vec, right_vec);
       if(!left.is_neg() and bigint::do_bigless(left_vec, right_vec)){
           difference.toggle_neg();
       }

    }
	
	
  difference.set_big_value(diff_vec);
  //trim zeros.
  for(int i=diff_vec.size()-1; i>=0; i--){
	  if(diff_vec.at(i) == ZERO)
		  diff_vec.pop_back();
	  else
		  break;
  }
  difference.set_big_value(diff_vec);
  return difference;
}


//what does this do? stack.pop is taken care of in main so two arguments should always be passed to do_arith
bigint operator+ (const bigint& right) {
	bigvalue_t sum_vec;
	bigvalue_t one_vec;
	one_vec.push_back(1+ZERO);
	
   const bigvalue_t& right_vec = right.get_big_value();
	
	bigint sum;
	sum_vec = sum.do_bigadd(right_vec, one_vec);
	sum.set_big_value(sum_vec);
    return sum;
}

bigint operator- (const bigint& right) {
	bigvalue_t diff_vec;
	bigvalue_t one_vec;
	one_vec.push_back(1+ZERO);
	
   const bigvalue_t& right_vec = right.get_big_value();
	
	bigint diff;
	diff_vec = diff.do_bigadd(right_vec, one_vec);
	diff.set_big_value(diff_vec);
    return diff;
}

long bigint::to_long() const {
/*
   if (*this <= bigint (numeric_limits<long>::min())
    or *this > bigint (numeric_limits<long>::max()))
               throw range_error ("bigint__to_long: out of range");
*/			   
	long value = 0;
	long i = 1;
	bigvalue_t the_big_vec = get_big_value();
	//cout << "the_big_vec: ";
   	//for(int i = the_big_vec.size()-1; i >= 0; i--)
	//	cout << the_big_vec.at(i);
	//cout << endl;
    vector<unsigned char>::iterator itor;
	for(itor = the_big_vec.begin(); itor != the_big_vec.end(); ++itor){
		//cout << "i == " << i << " value == " << value << "((*itor)*i) == " << ((*itor-ZERO)*i) << endl;
		value = value + ((*itor-ZERO)*i);
		//cout << "*itor == " << *itor << " i == " << i << " value == " << value << endl;
		i = 10*i;
		//cout << "now i == " << i << endl;
	}
	//cout << "value: " << value << endl;
   return value;
}
bool abs_less (const long& left, const long& right) {
   return left < right;
}

//
// Multiplication algorithm.
//
bigint operator* (const bigint& left, const bigint& right) {
   	bigint product;

    if(left.is_neg() xor right.is_neg())
   		product.toggle_neg();

   	if ((right.to_long() == 0) or (left.to_long() == 0)) {
   		bigint::bigvalue_t product_vec(1, (unsigned char)'0');
   		product.set_big_value(product_vec);
   		return product;
    }
    else {
    	bigint::bigvalue_t product_vec = product.do_bigmul(left, right);
    	product.set_big_value(product_vec);
    	return product;
    }
}

//
// Division algorithm.
//

void multiply_by_2 (bigint& b) {
	bigint obj;
   b.set_big_value(obj.do_bigadd(b.get_big_value(),b.get_big_value()));
   
	bigvalue_t b_vec =b.get_big_value();
	b.set_big_value(b_vec);
	//trim zeros.
	for(int i=b_vec.size()-1; i>=0; i--){
		if(b_vec.at(i) == ZERO)
			b_vec.pop_back();
		else
			break;
	}
	b.set_big_value(b_vec);
	
}

void divide_by_2 (bigint& b) { 
 // cout<<"BEGINNING OF divide_by_2: "<<b<<endl;
  for(auto itor = b.big_value.rbegin(); itor != b.big_value.rend() ;itor++){
		if((*itor-ZERO)%2==1){
		itor++;
		*itor = (*itor-ZERO)+10;
		*itor = *itor+ZERO;
		itor--;
	}
	  
	*itor = (*itor-ZERO)/2;
	*itor = (*itor+ZERO);
   }
   
	bigvalue_t b_vec =b.get_big_value();
	b.set_big_value(b_vec);
	//trim zeros.
	for(int i=b_vec.size()-1; i>=0; i--){
		if(b_vec.at(i) == ZERO)
			b_vec.pop_back();
		else
			break;
	}
	b.set_big_value(b_vec);
	
	//cout<<"END OF divide_by_2: "<<b<<endl;
}

bigvalue_t bigint::do_bigadd (const bigvalue_t& left, const bigvalue_t& right){
/*
	for(int i=left.size()-1; i>=0; i--)
		cout<<left.at(i);
	cout<<endl;
	for(int i=right.size()-1; i>=0; i--)
		cout<<right.at(i);
	cout<<endl;
	*/
	
	bigvalue_t sum;
	
	int carry = 0;
	int current_digit = 0;
	auto iter_left = left.begin();
	auto iter_right = right.begin();
	while(iter_left != left.end() and iter_right != right.end()){
		current_digit = (int)((*iter_left-ZERO) + (int)(*iter_right-ZERO) + carry);
        //cout << "current_digit: " << current_digit << endl;
		carry = (current_digit)/10;
        //cout << "carry: " << carry << endl;
		current_digit = current_digit%10;
        //cout << "current_digit%10: " << current_digit << endl;
        //cout << "pushing back current_digit: " << current_digit << endl;
		sum.push_back(current_digit+ZERO);
		iter_left++;
		iter_right++;
	}
	
	if(iter_left == left.end()){
        //cout << "iter_left == left.end()" << endl;
		while(iter_right != right.end()){
			current_digit = (int)(*iter_right-ZERO) + carry;
            //cout << "current_digit: " << current_digit << endl;
			carry = current_digit/10;
            //cout << "carry: " << carry << endl;
			current_digit = current_digit%10;
            //cout << "current_digit%10: " << current_digit << endl;
            //cout << "pushing back current_digit: " << current_digit << endl;
			sum.push_back(current_digit+ZERO);
			iter_right++;
		}
	}
	else{
        //cout << "iter_right == right.end()" << endl;
		while(iter_left != left.end()){
			current_digit = (int)(*iter_left-ZERO) + carry;
            //cout << "current_digit: " << current_digit << endl;
			carry = current_digit/10;
            //cout << "carry: " << carry << endl;
			current_digit = current_digit%10;
            //cout << "current_digit%10: " << current_digit << endl;
            //cout << "pushing back current_digit: " << current_digit << endl;
			sum.push_back(current_digit+ZERO);
			iter_left++;
		}
	}
	
	if(carry != 0)
		sum.push_back(carry+ZERO);
	
	return sum;
}

bigvalue_t bigint::do_bigsub(const bigvalue_t& left, const bigvalue_t& right){
  bigvalue_t difference;
	
	int current_digit = 0;
  int borrow = 0;
    
	auto iter_left = left.begin();
	auto iter_right = right.begin();

	while(iter_right != right.end()){
		current_digit = (int)(*iter_left-ZERO) - (int)(*iter_right-ZERO) - borrow;
    if (current_digit < 0){
      borrow = 1;
      current_digit += 10;
    }
    else{
      borrow = 0;
    }
    difference.push_back(current_digit+ZERO);
    iter_left++;
    iter_right++;
	}
  
  while(iter_left != left.end()){
	current_digit = (*iter_left - ZERO) - borrow;
    difference.push_back(current_digit+ZERO);
    iter_left++;
  }
	return difference;
}

bigvalue_t bigint::do_bigmul(const bigint& left, const bigint& right){
	
	//initialize a vector containing one '0' 
	/*
   	bigvalue_t product_vec(1, (unsigned char)'0');
   	const bigvalue_t& left_vec = left.get_big_value();

    cout << "left: ";
    for(int i = left_vec.size() - 1; i >= 0; i--)
		cout << left_vec.at(i);
	cout << endl;
	*/
/*
	long multiplier = right.to_long();
	//cout << "multiplier: " << multiplier << endl; 

   	while (multiplier > 0){
 		product_vec = do_bigadd(left_vec, product_vec);
		--multiplier;
   	}

   	return product_vec;
*/
	
	bigint sum_of_products;
	bigint hold;
	
	bigvalue_t single_mult;
	
	bigvalue_t product_vec;
	
	auto iter_left = left.get_big_value().begin();
	auto iter_right = right.get_big_value().begin();
	auto iter_left_end = left.get_big_value().end();
	auto iter_right_end = right.get_big_value().end();
	int carry = 0;
	int push_forward = 0;
	for(auto itr_right = iter_right; itr_right != iter_right_end; itr_right++){
		for(int i=0; i<push_forward; i++){
				single_mult.push_back(0+ZERO);
			}
		for(auto itr_left = iter_left; itr_left != iter_left_end; itr_left++){
			
			if(*itr_right == 0+ZERO){
				single_mult.push_back(0+ZERO);
				break;
			}
			else{
				long ins_this = ((*itr_left-ZERO)*(*itr_right-ZERO))+ carry;
				//cout<<": "<<ins_this<<endl;
				if(ins_this%10 > 0){
					carry = ins_this/10;
					ins_this = ins_this%10;
				}
				else
					carry = 0;
				single_mult.push_back(ins_this+ZERO);
				/*
					cout << "single_mult: ";
					for(int i = single_mult.size() - 1; i >= 0; i--)
						cout << single_mult.at(i);
					cout << endl;
					*/
			}
			
		}
		single_mult.push_back(carry+ZERO);
		push_forward++;

		carry = 0;
		hold.set_big_value(single_mult);
		sum_of_products = sum_of_products + hold;
		single_mult.clear();
	}
	
	product_vec = sum_of_products.get_big_value();
	
	return product_vec;
}

bool bigint::do_bigless(const bigvalue_t& left, const bigvalue_t& right){
	if(left.size() < right.size())
		return true;
	else if(left.size() > right.size() )
		return false;
	else{
		auto iter_left = left.end();
		iter_left--;
		auto iter_right = right.end();
		iter_right--;
		for(int i = left.size()-1; i > -1; --i){
			if(*iter_left < *iter_right) {
				return true;
			} else if (*iter_left > *iter_right) {
				return false;
			}
			--iter_left;
			--iter_right;
		}
		return false;
	}
}

void bigint::set_big_value(const bigvalue_t& new_bigvalue){
	this->big_value = new_bigvalue;
}

const bigvalue_t& bigint::get_big_value() const{
	return big_value;
}

bool bigint::is_neg() const{
	return this->negative;
}

void bigint::toggle_neg(){
	this->negative = !this->is_neg();
}

void bigint::kill_zeros(){
	
	bigvalue_t hold = big_value;
	
	for(auto itr = hold.begin(); itr != hold.end(); itr++)
		cout<<"ITR: "<<*itr<<endl;
	cout<<"size: "<<hold.size()<<endl;
	auto iter = hold.end();
	iter--;
	for(int i=hold.size()-1; i != -1; i--)
	{
		cout<<"ITER: "<<*iter<<endl;
		if(*iter == ZERO){
			cout<<"POPPING: "<<hold.at(i)<<endl;
			hold.pop_back();
		}
		else
			break;
		iter--;
	}
	for(auto itr = hold.begin(); itr != hold.end(); itr++)
		cout<<"ITR: "<<*itr<<endl;
	
	this->set_big_value(hold);
}

bigint& bigint::operator= (const bigint& that){
      if(this == &that) 
		  return *this;
      if(that.is_neg())
		  this->toggle_neg();
      this->set_big_value(that.big_value);
      return *this;
}

bigint::quot_rem divide (const bigint& left, const bigint& right) {
	bigint zero;
	bigvalue_t temp;
	temp.push_back(0+ZERO);
	zero.set_big_value(temp);
	
	if (right == zero) throw domain_error ("divide by 0");
	if (right == zero) throw domain_error ("bigint::divide");
	
	bigvalue_t hold;
	hold.push_back(1+ZERO);
	bigint power_of_2;
	power_of_2.set_big_value(hold);
	
	bigint one = power_of_2;
	
	bigint divisor = right;
	bigint quotient;
	bigint rem = left; 
	while (divisor < rem ){
		multiply_by_2 (divisor);
		multiply_by_2 (power_of_2);
	}
	while (zero < power_of_2) { 
		if (not (rem < divisor)) {
			rem = rem - divisor;
			quotient = quotient + power_of_2;
		}
		divide_by_2 (divisor);
		if(power_of_2 == one)
			power_of_2 = zero;
		else
			divide_by_2 (power_of_2);
	}
	
	return {quotient, rem};
	
}

bigint operator/ (const bigint& left, const bigint& right) {
   return divide(left, right).first;
}

bigint operator% (const bigint& left, const bigint& right) {
   return divide(left, right).second;
}

bool operator== (const bigint& left, const bigint& right) {
   if(left.get_big_value().size() == right.get_big_value().size()){
   		for(int i = 0; i < (int)(left.get_big_value().size()); ++i){
   			if(!(left.get_big_value().at(i) == right.get_big_value().at(i)))
   				return false;
   		}
   		return true;
   }
   else {
   	return false;
   }
}

bool operator< (const bigint& left, const bigint& right) {
	const bigvalue_t& left_vec = left.get_big_value();
	const bigvalue_t& right_vec = right.get_big_value();
    bool result = bigint::do_bigless(left_vec, right_vec);
    return result;
}

ostream& operator<< (ostream& out, const bigint& that) {
   bigvalue_t big_vec = that.get_big_value();
   if(that.is_neg())
	   cout<<"-";
   
   for(int i = big_vec.size()-1; i >= 0; i--){
   	out << big_vec.at(i);
   }
   return out;
}

bigint pow (const bigint& base, const bigint& exponent) {
	/*
   DEBUGF ('^', "base = " << base << ", exponent = " << exponent);
   if (base == 0) return 0;
   bigint base_copy = base;
   long expt = exponent.to_long();
   bigint result = 1;
   if (expt < 0) {
      base_copy = 1 / base_copy;
      expt = - expt;
   }
   while (expt > 0) {
      if (expt & 1) { //odd
         result = result * base_copy;
         --expt;
      }else { //even
         base_copy = base_copy * base_copy;
         expt /= 2;
      }
   }
   DEBUGF ('^', "result = " << result);
   */

   bigint result;

   long long_exponent = exponent.to_long();
   //cout << "long_exponent: " << long_exponent << endl;
    bigvalue_t intermediate_vec(1, (unsigned char)'1');

    if(long_exponent < 0){
   	 	result.set_big_value(intermediate_vec);
    }
    else if (long_exponent == 0){
   	 	result.set_big_value(intermediate_vec);
    }
    else {
    	bigint base_copy = base;
    	base_copy.set_big_value(intermediate_vec);
   		for(long index = 0; index < long_exponent; ++index){
   			//cout << "pow: index = " << index << endl;
   			intermediate_vec = result.do_bigmul(base_copy, base);
   			base_copy.set_big_value(intermediate_vec);
   		}
   		//free(&base_copy);
	}
	result.set_big_value(intermediate_vec);
   	return result;
}
