#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
#include <sstream>
#include <string>
using namespace std;
bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;
 
  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
   vector<string> res;
   res = split(s ,' ');
   if(res.size() == 1)
      return std::shared_ptr<QueryBase>(new WordQuery(s));
  else if (res.size() == 3){
    if(res.at(1)=="AND")
        return std::shared_ptr<QueryBase>(new AndQuery(res.at(0),res.at(2)));
    else if(res.at(1)=="OR")
        return std::shared_ptr<QueryBase>(new OrQuery(res.at(0),res.at(2)));
    else if(is_number(res.at(1)))
     return std::shared_ptr<QueryBase>(new NQuery(res.at(0),res.at(2) ,stoi(res.at(1))));
    else
      throw std::invalid_argument("Unrecognized search");
  }
   else if(res.at(0)=="NOT")
      return std::shared_ptr<QueryBase>(new NotQuery(res.at(1)));
 
    else {
      throw std::invalid_argument("Unrecognized search");
    }
}


////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const
{
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();
  
  for (size_t n = 0; n != sz; ++n)
  {
    if (beg==end || *beg != n)
		ret_lines->insert(n);
    else if (beg != end)
		++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
    
}

QueryResult AndQuery::eval (const TextQuery& text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = 
      std::make_shared<std::set<line_no>>(left_result.begin(), left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult NQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);

  auto ret_lines = std::make_shared<std::set<line_no>>();
      std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

    for (auto it = ret_lines->begin(); it != ret_lines->end();){
      istringstream res(left_result.get_file().get()->at(*it)); 
      int left , right , i = 0;
      do { 
        string word; 
        res >> word; 
        if(word.find(left_query) != string::npos)
          left = i;
        if(word.find(right_query) != string::npos)
          right = i;
        i++;
      } 
      while (res); 
      if(abs(left - right) - 1 > dist){
        ret_lines->erase(it);
        if(ret_lines->size() == 0)
          return QueryResult(rep(), ret_lines, left_result.get_file());
      }
      else{
        ++it;
      }
  }
  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////