// $Id: bigint.cpp,v 1.2 2020-01-06 13:39:55-08 - - $
//Dmitriy Nikitin
//dnikitn@ucsc.edu
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include<vector>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): ubig_value (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << ubig_value)
}

bigint::bigint (const ubigint& ubig_value_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   ubig_value = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {ubig_value, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
    bigint result = bigint();
  if(is_negative == true && that.is_negative == false){
    if(ubig_value < that.ubig_value)
      result.ubig_value = that.ubig_value - ubig_value;
    else{
      result.ubig_value = ubig_value - that.ubig_value;
      result.is_negative = true; 
    }
  }
  else if(is_negative == false && that.is_negative == true){
    if(ubig_value < that.ubig_value){
      result = that.ubig_value - ubig_value;
      result.is_negative = true;
    }
    else
      result.ubig_value = ubig_value - that.ubig_value;
  }
  else{//both neg or both pos
    result.ubig_value = ubig_value + that.ubig_value;
    result.is_negative = that.is_negative;
  }
  return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint result = bigint();
   if(is_negative == true && that.is_negative == false){
    result.ubig_value = ubig_value + that.ubig_value;
    result.is_negative = true;
  }
  else if(is_negative == false && that.is_negative == true){
    result.ubig_value = ubig_value + that.ubig_value;
  }
  else if(is_negative == true && that.ubig_value == true){
    result.ubig_value = that.ubig_value - ubig_value;
  }
  else{//both pos
    if(ubig_value > that.ubig_value)
      result.ubig_value = ubig_value - that.ubig_value;
    else{
      result.ubig_value = that.ubig_value - ubig_value;
      result.is_negative = true;
    }
  }
   return result;
}


bigint bigint::operator* (const bigint& that) const {
    bigint result = bigint();
    result.ubig_value = ubig_value * that.ubig_value;
    if(is_negative == true && that.is_negative == false)
      result.is_negative = true;
    else if(is_negative == false && that.is_negative == true)
      result.is_negative = true;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result = bigint();
   result.ubig_value = ubig_value / that.ubig_value;
   if(is_negative == true && that.is_negative == false)
      result.is_negative = true;
    else if(is_negative == false && that.is_negative == true)
      result.is_negative = true;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = bigint();
   result.ubig_value = ubig_value % that.ubig_value;
   return result;
}

bool bigint::operator== (const bigint& that) const {
  if(is_negative == that.is_negative and 
                ubig_value == that.ubig_value)
    return true;
  else 
    return false;
}

bool bigint::operator< (const bigint& that) const {
  if(is_negative != that.is_negative) return is_negative;
  return (ubig_value < that.ubig_value);
}

ostream& operator<< (ostream& out, const bigint& that) {
  return out << (that.is_negative ? "-" : "") 
             << that.ubig_value;         
}
