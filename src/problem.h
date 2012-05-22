#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <string>
#include <vector>
#include <map>
#include "ae.h"

using namespace std;

typedef struct {
  vector<string> inputs;
  string expression;
} codeStruct;

template <class T>
vector<T> getCartProd(int index, vector<vector<T> > myVecs) {
  vector<T> result;
  int i;

  result.resize(myVecs.size());

  for(i=result.size()-1; i>0; i--) {
    result[i] = myVecs[i][index % myVecs[i].size()];
    index /= myVecs[i].size();
  }

  if(index >= myVecs[0].size()) {
    result.clear();
  } else {
    result[0] = myVecs[0][index % myVecs[0].size()];
  }

  return result;
}

template <class T>
unsigned long sizeofCartProd(vector<vector<T> > myVecs) {
  unsigned long size = 1;
  if(!myVecs.size()) return 0;
  for(int i=0; i<myVecs.size(); i++) size *= myVecs[i].size();
  return size;
}

string makeVariable(lua_State *L, string varStr, vector<pair<string, string> > pairs);
string getBlankVariable(string varStr);

class Codelet {
  bool result;
  string name;
  map<string, string> params;
  vector<string> deps;
  vector<string> output;
  codeStruct code;

  private:
    vector<string> getReqDeps(vector<pair<string, string> > dependency); 
    vector<string> getOutputs(vector<pair<string, string> > dependency); 

  public:
    void makeResult();
    void setName(string N);
    void addParameter(string p, string val);
    void addParameter(pair<string, string> p);
    void addDepend(string key);
    void addOutput(string key);
    void addCode(string definition);
    bool matchesDep(lua_State *L, string dependency, vector<string> &reqdeps, vector<string> &outputs);

    bool isResult();
    string getName();
    codeStruct getCode();
    vector<string> getOutput();
    vector<string> getDependencies();
    vector<pair<string, string> > getParameters();
};

class Problem {
  string name;
  map<string, vector<Codelet> > codelets;
  Codelet result;
  vector<string> data;

  public:
    Problem();

    Problem(string n);
    void addCodelet(string name, bool res, vector<pair<string, string> > params, vector<string> d, vector<string> o, string code);
    void addData(string url);

    int numCodelets();
    int numData();
    int numEntries();

    vector<Codelet> getCodelets(string s);
    Codelet getResult();
    vector<string> getData();
};

#endif
