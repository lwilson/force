#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "problem.h"
#include <matheval.h>

KeyObject::KeyObject(string key) {
  vector<string> split_str;
  //Split the input string into its base and parameters
  boost::split(split_str, key, boost::is_any_of("[]"));
  basename = split_str[0];
  for(int i=1; i<split_str.size(); i++) {
    if(split_str[i].length()) parameters.push_back(split_str[i]);
  }
}

vector<string> KeyObject::getParameters() {
  return parameters;
}

string KeyObject::toString() {
  string s = basename;
  for(int i=0; i<parameters.size(); i++)
    s += "[" + parameters[i] + "]";
  return s;
}

string KeyObject::toString(vector<pair<string,string> > pairs) {
  void* eval;
  double value;
  ostringstream s;
  s << basename;
  for(int k=0; k<parameters.size(); k++) {
    string p = parameters[k];
    for(int i=0; i<pairs.size(); i++)
      boost::replace_all(p, pairs[i].first, pairs[i].second);
    char* p2 = (char*)p.c_str();
    eval = evaluator_create(p2);
    value = evaluator_evaluate_x(eval,0);
    s << "[" << value << "]";
    evaluator_destroy(eval);
  }
  return s.str();
}

vector<string> Codelet::getReqDeps(vector<pair<string, string> > dependency) {
}

vector<string> Codelet::getOutputs(vector<pair<string, string> > dependency) {
}

void Codelet::makeResult() { result = true; }

void Codelet::setName(string N) { name = N; }

void Codelet::addParameter(string p, string val) { params.push_back(make_pair(p,val)); }
void Codelet::addParameter(pair<string, string> p) { params.push_back(p); }

void Codelet::addDepend(string key) { deps.push_back(KeyObject(key)); }

void Codelet::addOutput(string key) { output.push_back(KeyObject(key)); }

void Codelet::addCode(string definition) { code = definition; }

bool Codelet::matchesDep(string dependency, vector<string> &reqdeps, vector<string> &outputs) {
}

bool Codelet::isResult() { return result; }

string Codelet::getName() { return name; }

string Codelet::getCode() { return code; }

vector<KeyObject> Codelet::getOutput() { return output; }

vector<KeyObject> Codelet::getDependencies() { return deps; }

vector<pair<string,string> > Codelet::getParameters() { return params; }

Problem::Problem(string n) {
  name = n;
}

void Problem::addCodelet(string name, bool res, vector<pair<string, string> > params, vector<string> d, vector<string> o, string code) {
  Codelet myCodelet;
  int i;
  myCodelet.setName(name);
  if(res) myCodelet.makeResult();
  for(i=0; i<params.size(); i++) myCodelet.addParameter(params[i]);
  for(i=0; i<d.size(); i++) myCodelet.addDepend(d[i]);
  for(i=0; i<o.size(); i++) myCodelet.addOutput(o[i]);
  myCodelet.addCode(code);
  
  if(res) {
    result = myCodelet;
  } else {
    string clean;
    vector<string> osplit;
    for(i=0; i<o.size(); i++) {
      boost::split(osplit, o[i], boost::is_any_of("[]"));
      clean = osplit[0];
      for(int j=1; j<osplit.size(); j++) if(osplit[j].length()) clean += "[]";
      codelets[clean].push_back(myCodelet);
    }
  }
}

