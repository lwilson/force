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

string KeyObject::getBasename() { return basename; }

vector<string> KeyObject::getParameters() { return parameters; }

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

void Codelet::addParameter(string p, string val) { params[p] = val; }

void Codelet::addParameter(pair<string, string> p) { params[p.first] = p.second; }

void Codelet::addDepend(string key) { deps.push_back(KeyObject(key)); }

void Codelet::addOutput(string key) { output.push_back(KeyObject(key)); }

void Codelet::addCode(string definition) { code = definition; }

bool Codelet::matchesDep(string dependency, vector<string> &reqdeps, vector<string> &outputs) {
  int codeletMatch = -1;
  KeyObject depKey(dependency);
  for(int i=0; i<output.size(); i++) {
    if((depKey.getBasename() == output[i].getBasename()) && 
       (depKey.getParameters().size() == output[i].getParameters().size())) {
      bool match=true;
      //Basenames match and parameter lists are same length, this codelet is a potential fit
      for(int j=0; j<output[i].getParameters().size(); j++) {
        vector<string> ranges;
        //Check if output tag is in the codelet parameter list, if so, grab the parameter range
        if(params.find(output[i].getParameters()[j]) != params.end()) 
          boost::split(ranges, params[output[i].getParameters()[j]], boost::is_any_of(","));
        else 
          ranges.push_back(output[i].getParameters()[j]);
        //Determine if dependency parameter matches codelet parameter range
        for(int k=0; k<ranges.size(); k++) {
          vector<string> foo;
          boost::split(foo, ranges[k], boost::is_any_of(":"));
          if(foo.size() == 1) {
            if(depKey.getParameters()[j] != foo[0]) match = false;
          } else {
            if((atoi(depKey.getParameters()[j].c_str()) < atoi(foo[0].c_str())) || 
               (atoi(depKey.getParameters()[j].c_str()) > atoi(foo[1].c_str())))match = false; 
          }
          //This codelet does not match, we can move on to the next output
          if(!match) break;
        }
      }
      if(match) {
        codeletMatch = i;
        break;
      }
    }
  }
  if(codeletMatch > -1) {
    reqdeps.clear();
    outputs.clear();
    vector<pair<string,string> > kvset;
    for(int i=0; i<output[codeletMatch].getParameters().size(); i++)
      kvset.push_back(make_pair(output[codeletMatch].getParameters()[i], depKey.getParameters()[i]));
    for(int i=0; i<deps.size(); i++)
      reqdeps.push_back(deps[i].toString(kvset));
    for(int i=0; i<output.size(); i++)
      outputs.push_back(output[i].toString(kvset));
    return true;
  }
  return false;
}

bool Codelet::isResult() { return result; }

string Codelet::getName() { return name; }

string Codelet::getCode() { return code; }

vector<KeyObject> Codelet::getOutput() { return output; }

vector<KeyObject> Codelet::getDependencies() { return deps; }

vector<pair<string,string> > Codelet::getParameters() { 
  vector<pair<string,string> > v(params.begin(), params.end());
  return v;
}

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

