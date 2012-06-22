#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "problem.h"
#include "ae.h"

string getVariableBasename(string varStr) {
  vector<string> split_str;
  //Split the input string into its base and parameters
  boost::split(split_str, varStr, boost::is_any_of("[]"));
  
  return split_str[0];
}

vector<string> getVariableParameters(string varStr) {
  vector<string> split_str;
  //Split the input string into its base and parameters
  boost::split(split_str, varStr, boost::is_any_of("[]"));

  vector<string> parameters;
  for(int i=1; i<split_str.size(); i++) {
    if(split_str[i].length()) parameters.push_back(split_str[i]);
  }

  return parameters;
}

string getBlankVariable(string varStr) {
  string s = getVariableBasename(varStr);
  vector<string> params = getVariableParameters(varStr);
  for(int i=0; i<params.size(); i++) s += "[]";

  return s;
}

string makeVariable(lua_State *L, string varStr, vector<pair<string, string> > pairs) {
  double value;
  char sTmp[256];
  string s;
  vector<string> parameters = getVariableParameters(varStr);

  s = getVariableBasename(varStr);

  for(int k=0; k<parameters.size(); k++) {
    string p = parameters[k];
    for(int i=0; i<pairs.size(); i++)
      boost::replace_all(p, pairs[i].first, pairs[i].second);
    char* p2 = (char*)p.c_str();
    value = ae_eval(L, p2);
    sprintf(sTmp, "[%ld]", (int)value);
    s.append(sTmp);
  }
  return s;
}

vector<string> Codelet::getReqDeps(vector<pair<string, string> > dependency) {
}

vector<string> Codelet::getOutputs(vector<pair<string, string> > dependency) {
}

void Codelet::makeResult() { result = true; }

void Codelet::setName(string N) { name = N; }

void Codelet::addParameter(string p, string val) { params[p] = val; }

void Codelet::addParameter(pair<string, string> p) { params[p.first] = p.second; }

void Codelet::addDepend(string key) { deps.push_back(key); }

void Codelet::addOutput(string key) { output.push_back(key); }

void Codelet::addCode(string definition) {
  vector<string> tmpParams;
  vector<string> splitDef;

  //Split definition into two parts: parameters and expression
  boost::split(splitDef, definition, boost::is_any_of(":"));

  if(splitDef.size() == 2) {
    //Split the parameter list into individual variable names
    boost::split(tmpParams, splitDef[0], boost::is_any_of("(,)"));
    for(int i=0; i<tmpParams.size(); i++) {
      if(tmpParams[i].length()) code.inputs.push_back(tmpParams[i]);
    }

    //Store the expression
    code.expression = splitDef[1];
  } else code.expression = splitDef[0];
}

bool Codelet::matchesDep(lua_State *L, string dependency, vector<string> &reqdeps, vector<string> &outputs) {
  int codeletMatch = -1;
  string depBase = getVariableBasename(dependency);
  vector<string> depParams = getVariableParameters(dependency);
  for(int i=0; i<output.size(); i++) {
    string outBase = getVariableBasename(output[i]);
    vector<string> outParams = getVariableParameters(output[i]);
    if((depBase == outBase) && (depParams.size() == outParams.size())) {
      bool match = true;
      //Basenames match and parameter lists are same length, this codelet is a potential fit
      for(int j=0; j<outParams.size(); j++) {
        vector<string> ranges;
        //Check if output tag is in the codelet parameter list, if so, grab the parameter range
        if(params.find(outParams[j]) != params.end())
          boost::split(ranges, params[outParams[j]], boost::is_any_of(","));
        else 
          ranges.push_back(outParams[j]);
        vector<vector<string> > prange;
        bool parammatch = false;
        //Determine if dependency parameter matches codelet parameter range
        for(int k=0; k<ranges.size(); k++) {
          vector<string> foo;
          boost::split(foo, ranges[k], boost::is_any_of(":"));
          if(foo.size() == 1) {
            if(depParams[j] == foo[0]) parammatch = true;
          } else {
            if((atoi(depParams[j].c_str()) >= atoi(foo[0].c_str())) && 
               (atoi(depParams[j].c_str()) <= atoi(foo[1].c_str()))) parammatch = true; 
          }
        }
        //This codelet does not match, we can move on to the next output
        if(!parammatch) {
          match = false;
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
    string outBase = getVariableBasename(output[codeletMatch]);
    vector<string> outParams = getVariableParameters(output[codeletMatch]);

    for(int i=0; i<outParams.size(); i++)
      kvset.push_back(make_pair(outParams[i], depParams[i]));
    for(int i=0; i<deps.size(); i++)
      reqdeps.push_back(makeVariable(L, deps[i], kvset));
    for(int i=0; i<output.size(); i++)
      outputs.push_back(makeVariable(L, output[i], kvset));
    return true;
  }
  return false;
}

bool Codelet::isResult() { return result; }

string Codelet::getName() { return name; }

codeStruct Codelet::getCode() { return code; }

vector<string> Codelet::getOutput() { return output; }

vector<string> Codelet::getDependencies() { return deps; }

vector<pair<string,string> > Codelet::getParameters() { 
  vector<pair<string,string> > v(params.begin(), params.end());
  return v;
}

Problem::Problem() {}

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
    for(i=0; i<o.size(); i++) {
      codelets[getBlankVariable(o[i])].push_back(myCodelet);
    }
  }
}

void Problem::addData(string url) {
  data.push_back(url);  
}

vector<Codelet> Problem::getCodelets(string s) {
  return codelets[s];
}

Codelet Problem::getResult() { return result; }

vector<string> Problem::getData() { return data; }
