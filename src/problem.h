#ifndef _PROBLEM_H
#define _PROBLEM_H

#include "strfuncs.h"

typedef struct {
  char *name;
  codelet_type *codelets;
  codelet_type result;
  stringlist_type data;  
} problem_type;

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
