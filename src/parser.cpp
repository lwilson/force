#include "problem.h"
#include <fstream>

using namespace std;

Problem parse(string filename) {
  bool codesegment = false;
  ifstream file;
  string word;
  file.open(filename);
  string name;
  bool result;
  vector<pair<string,string> > params;
  vector<string> deps;
  vector<string> output;
  string code;
  Problem problem;
  string foo;
  vector<string> sfoo;

  while(!file.eof()) {
    file >> word;
    if(word == "end") {
      file >> word;
      if(word == "result" || word == "codelet") {
        problem.addCodelet(name, result, params, deps, output, code);
      } else {
        if(word == "code") codesegment = false;
      }
    }
    if(codesegment) {
      code += word;
    } else {
      if(word == "problem") {
      }
      if(word == "result" || word == "codelet") {
        result = (word == "result");
        file >> name;
      }
      if(word == "parameter") {
        file >> foo;
        boost::split(sfoo, foo, boost::is_any_of("="));
        params.push_back(make_pair(sfoo[0], sfoo[1]));
      }
      if(word == "depends-on") {
        file >> foo;
        boost::split(sfoo, foo, boost::is_any_of("[]"));
        string basename = sfoo[0];
        for(int i=1; i<sfoo.size(); i++) {
          if(sfoo[i].length()) {
            if(sfoo[i].find(",") != -1 || sfoo[i].find(":") != -1) {
              vector<string> sfoo2;
              boost::split(sfoo2, sfoo[i], boost::is_any_of(","));
              for(int j=0; j<sfoo2.size(); j++) {
                vector<string> prange;
                boost::split(prange, sfoo2[j], boost::is_any_of(":"));
                if(prange.size() == 2) {
                  
              }
            }
          }
        }  
    }
  }
}
