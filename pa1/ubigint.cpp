// $Id: ubigint.cpp,v 1.8 2020-01-06 13:39:55-08 - - $
//Dmitriy Nikitin
//dnikitn@ucsc.edu
#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include<vector>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that) {
   while(that != 0){
         ubig_value.push_back(that%10 + '0');
         that /= 10;
   }
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (int i = that.size() - 1; i >= 0; i--) {
      unsigned char  digit = that.at(i);
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      else
         ubig_value.push_back(digit);
   }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint result = ubigint();
   int size;
   bool check = false;
   if(ubig_value.size() > that.ubig_value.size()) {
      size = that.ubig_value.size();
      check = true;//this is bigger than that
   }
   else 
      size = ubig_value.size();
   int carry = 0;
   int val = 0;
   int i = 0;
   int o1, o2;
   while(i < size){
      o1 = ubig_value.at(i) - '0';
      o2 = that.ubig_value.at(i) - '0';
      val = o1 + o2 + carry;
      carry = 0;
      if(val > 9) {
         carry = 1;
         val %= 10;
      }

      result.ubig_value.push_back(val + '0');
      i++;
   }
   if(check){
      while(i < ubig_value.size()){
         val = ubig_value.at(i) - '0';
         if(carry != 0) 
            carry = 1;
            val += carry;
         carry = 0;       
           if(val > 9){
              val %= 10;
              carry = 1;
         result.ubig_value.push_back(val + '0');
         }
         else
            result.ubig_value.push_back(val + '0');
         i++;
      }
   }
   else{//that has more numbers than this
      while(i < that.ubig_value.size()){
         val = that.ubig_value.at(i) - '0';
         if(carry != 0) 
            val += carry;
         carry = 0;       
           if(val > 9){
              val %= 10;
              carry = 1;
         result.ubig_value.push_back(val + '0');
         }
         else
            result.ubig_value.push_back(val + '0');
         i++;
      }
   }
   result.ubig_value.push_back(carry + '0');
   while (result.ubig_value.size() > 0 
         and (result.ubig_value.back() - '0') == 0){
      result.ubig_value.pop_back();
      }
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result = ubigint();
   result.ubig_value.resize(ubig_value.size());
   int size = that.ubig_value.size();
   int carry = 0;
   int val = 0;
   int i = 0;
   int o1, o2;
   while(i < size){
      o1 = ubig_value.at(i) - '0';
      o2 = that.ubig_value.at(i) - '0';
      val = o1 - o2 + carry;
      carry = 0;
      if(val < 0) {
         carry = -1;
         val += 10;
      }
      result.ubig_value.push_back(val + '0');
      i++;
   }
   //if this has more nums than that 
   while(i < ubig_value.size()){
      if(carry != 0){
         result.ubig_value.push_back(
            ((ubig_value.at(i) - '0')+carry) + '0');
         carry = 0;
         i++;
      }
      else{
         result.ubig_value.push_back(ubig_value.at(i));
         i++;
      }
   }
   while (result.ubig_value.size() > 0 
         and (result.ubig_value.back() - '0') == 0){
      result.ubig_value.pop_back();
      }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result = ubigint();
   int d = 0;
   int carry;
   int n = that.ubig_value.size();
   int m = ubig_value.size();
   result.ubig_value.resize(n+m);
   int o1, o2;
   for(int i = 0; i < m; i++){
      carry = 0;
      o1 = ubig_value.at(i) - '0';
      for(int j = 0; j < n; j++){
         o2 = that.ubig_value.at(j) - '0';
         if((result.ubig_value.at(i+j) - '0') < 0)
            d = o1*o2 + carry;
         else
            d = (result.ubig_value.at(i+j) -'0') + (o1*o2 + carry);
         result.ubig_value.at(i+j) = d%10 + '0';
         carry = d/10;
      }
      result.ubig_value.at(i+n) = carry + '0';
   }
    while (result.ubig_value.size() > 0 
         and (result.ubig_value.back() - '0') == 0) {
      result.ubig_value.pop_back();
   }
   return result;
}

void ubigint::multiply_by_2() {
   int carry = 0;
   int num;
   for(int i = 0; i < ubig_value.size(); i++){
      num  = ubig_value.at(i) -'0';
      num += carry;
      num *= 2;
      carry = 0;
      if(num>9) {
         carry = 1;
         num -= 10;
      }
      ubig_value.at(i) = num + '0';
   }
   if(carry > 0) 
      ubig_value.push_back(carry + '0');
   while (ubig_value.size() > 0 
      and (ubig_value.back() - '0') == 0){
      ubig_value.pop_back();
      }
}

void ubigint::divide_by_2() {
   //FIX
   std::vector<unsigned char> v;
   int carry = 0;
   int num;
   for(int i = ubig_value.size()-1; i >= 0; i--){
      //carry *= 10;
      num = (ubig_value.at(i) - '0') + carry;
      num /= 2;
      v.insert(v.begin(), num + '0');
      carry = num%2;
   }
   ubig_value.clear();
   ubig_value = v;
   while (ubig_value.size() > 0 
         and (ubig_value.back() - '0') == 0){
      ubig_value.pop_back();
      }
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient; 
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if(ubig_value.size() != that.ubig_value.size()){
      return false;
   }
   else{
      for(int i = 0; i < ubig_value.size(); i++)
         if(ubig_value[i] != that.ubig_value[i])
            return false;
   }
   return true;
   // does C++ have built in vector comparison?
}

bool ubigint::operator< (const ubigint& that) const {
   if(ubig_value.size() < that.ubig_value.size())
      return true;
   else if(ubig_value.size() > that.ubig_value.size())
      return false;
   else {//both are of the same size
      for(int i = ubig_value.size()-1 ; i >= 0; i--){
         if(ubig_value[i] < that.ubig_value[i]) return true;
         else if(ubig_value[i] > that.ubig_value[i]) return false;
      }
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   int count = 1;
   for(int i = that.ubig_value.size(); i >= 0; i--){
      if(count%70 == 0)
         out << "\\" << "\n";
      out << that.ubig_value[i];
      count ++;
   }
   return out;
}
