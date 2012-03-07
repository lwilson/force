#ifndef _PROBLEM_H
#define _PROBLEM_H

#include <string>
#include <vector>
#include <map>

using namespace std;

class KeyObject {
  string basename;
  vector<string> parameters;

  public:
    KeyObject(string key);
    vector<string> getParameters();
    string toString();
    string toString(vector<pair<string,string> > pairs);
};

class Codelet {
  bool result;
  string name;
  vector<pair<string, string> > params;
  vector<KeyObject> deps;
  vector<KeyObject> output;
  string code;

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
    bool matchesDep(string dependency, vector<string> &reqdeps, vector<string> &outputs);

    bool isResult();
    string getName();
    string getCode();
    vector<KeyObject> getOutput();
    vector<KeyObject> getDependencies();
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
};

#endif
