#include "problem.h"
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <stdlib.h>
#include <matheval.h>

using namespace std;

vector<int> createRangeVector(string s) {
  vector<int> tmpV;
  if(s.find(",") != -1 || s.find(":") != -1) {
    vector<string> split_str;
    boost::split(split_str, s, boost::is_any_of(","));
    for(int j=0; j<split_str.size(); j++) {
      vector<string> prange;
      boost::split(prange, split_str[j], boost::is_any_of(":"));
      if(prange.size() == 2) {
        for(int k=atoi(prange[0].c_str()); k<=atoi(prange[1].c_str()); k++)
          tmpV.push_back(k);
      }
    }
  } else {
    tmpV.push_back(atoi(s.c_str()));
  }
  return tmpV;
}

Problem parse(string filename) {
  bool codesegment = false;
  ifstream file;
  string word;
  file.open(filename.c_str());
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
    else if(codesegment) {
      code += " " + word;
    } else {
      if(word == "problem") {
        file >> name;
        Problem problem(name);
      }
      if(word == "result") {
        result = true;
        file >> name;
      }
      if(word == "codelet") {
        result = false;
        file >> name;
        output.clear();
        deps.clear();
        params.clear();
      }
      if(word == "parameter") {
        file >> foo;
        boost::split(sfoo, foo, boost::is_any_of("="));
        params.push_back(make_pair(sfoo[0], sfoo[1]));
      }
      if(word == "depends-on") {
        file >> foo;
        deps.push_back(foo);
        //boost::split(sfoo, foo, boost::is_any_of("[]"));
        //vector<vector<int> > v;
        //string basename = sfoo[0];
        //for(int i=1; i<sfoo.size(); i++) 
        //  if(sfoo[i].length()) v.push_back(createRangeVector(sfoo[i]));
        //int i=0;
        //vector<int> cp=getCartProd(i, v);
        //while(cp.size()) {
        //  ostringstream toadd;
        //  toadd << basename;
        //  for(int j=0; j<cp.size(); j++) toadd << "[" << cp[j] << "]";
        //  deps.push_back(toadd.str());
        //  cp = getCartProd(++i, v);
        //}
      }
      if(word == "output") {
        string o;
        file >> o;
        output.push_back(o);
      }
      if(word == "code") {
        file >> code;
        codesegment = true;
      }
      if(word == "data") {
        string d;
        file >> d;
        problem.addData(d);      
      }
    }
  }

  return problem;
}

int main(int argc, char** argv) {

  string descfile(argv[1]);
  Problem problem = parse(descfile);
  map<string,string> rcht;
  vector<vector<int> > resultVals;
  vector<string> resultKeys;
  codeStruct code;  

  //Create a vector of result keys
  vector<pair<string,string> > parameters = problem.getResult().getParameters();
  for(int i=0; i<parameters.size(); i++) {
    resultKeys.push_back(parameters[i].first);
    resultVals.push_back(createRangeVector(parameters[i].second));
  }

  KeyObject output = problem.getResult().getOutput()[0];

  srand48((unsigned long)time(0)); 
  unsigned long origIndex = lrand48() % sizeofCartProd(resultVals);
  for(unsigned long index = origIndex+1; index != origIndex; index++) {
    vector<int> cp = getCartProd(index, resultVals);
    //If a zero vector returned, index was out of range and must be set to 0
    if(cp.size() == 0) {
      index = 0;
      cp = getCartProd(index, resultVals);
    }

    //Zip together the key/value pairs in the cartesian tuple
    vector<pair<string, int> > zipKey;
    for(int i=0; i<cp.size(); i++) zipKey.push_back(make_pair(resultKeys[i], cp[i]));

    //Generate key to search for
    string key = output.toString(zipKey);

    cout << "Attempt to solve result key: " << key << endl;
    //Continuously attempt to solve until result point complete
    while(rcht.find(key) == rcht.end()) {
      KeyObject attempt(key);
      string attemptstring = key;
      bool solvesIt = false;
      while(!solvesIt) {
        bool codeletMatch = false;
        vector<Codelet> candidates = problem.getCodelets(attempt.toBlankString());
        vector<string> attemptDeps, attemptOuts;
        vector<string> missing;
        vector<double> values;
        for(int i =0; i<candidates.size() && !codeletMatch; i++) {
          attemptDeps.clear();
          attemptOuts.clear();
          if(candidates[i].matchesDep(attemptstring, attemptDeps, attemptOuts)) {
            codeletMatch = true;
            code = candidates[i].getCode();
            values.clear();
            missing.clear();
            //Check to see if all of the dependencies have been solved
            for(int j=0; j<attemptDeps.size(); j++) {
              if(rcht.find(attemptDeps[i]) != rcht.end()) {
                values.push_back(atof(rcht[attemptDeps[j]].c_str()));
              } else
                missing.push_back(attemptDeps[j]);
            }
          }
        }
        //If no codelet matches, check the data files for a key that does
        if(!codeletMatch) {
          cout << "CRAP! Couldn't find a match for " << attemptstring << endl;
          exit(1);
          //We need to go back to the result point now
          break;
        }
        //If dependencies were missing, attempt to solve one of those dependencies
        else if(missing.size()) {
          attemptstring = missing[rand() % missing.size()];
          KeyObject attempt(attemptstring);
        }
        //If there was a codelet match and were no missing dependencies, then it's time to solve 
        else {
          //Evaluate the code
          char **names = (char**)malloc(sizeof(char*)*code.inputs.size());
          for(int a=0; a<code.inputs.size(); a++) {
            names[a] = (char*)code.inputs[a].c_str();
          }
          void* eval = evaluator_create((char*)code.expression.c_str());
          double Cvals[values.size()];
          for(int v=0; v<values.size(); v++) Cvals[v] = values[v];
          double val = evaluator_evaluate(eval, code.inputs.size(), names, Cvals);

          //Store the returned value in the table
          ostringstream os;
          os << val;
          cout << attemptstring << " " << val << endl;
          rcht[attemptstring] = os.str();         

          //Cleanup
          evaluator_destroy(eval);
          free(names);
          solvesIt = true;
        }
      }
    }
  }

}
