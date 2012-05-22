#include "problem.h"
#include <fstream>
#include <sstream>
#include <stack>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <stdlib.h>
#include "ae.h"
#include <unordered_map>
#include <omp.h>

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
  unordered_map<string, double> rcht;
  vector<vector<int> > resultVals;
  vector<string> resultKeys;

  //Create a vector of result keys
  vector<pair<string,string> > parameters = problem.getResult().getParameters();
  for(int i=0; i<parameters.size(); i++) {
    resultKeys.push_back(parameters[i].first);
    resultVals.push_back(createRangeVector(parameters[i].second));
  }

  string output = problem.getResult().getOutput()[0];

#pragma omp parallel
{
  stack<string> depStack;
  string attempt;
  char vs[256];
  codeStruct code;
  srand48((unsigned long)time(NULL) | omp_get_thread_num()); 
  unsigned long origIndex = lrand48() % sizeofCartProd(resultVals);
  unsigned long index = origIndex;
 
  lua_State *L = ae_open();

  do {
    vector<int> cp = getCartProd(index, resultVals);
    //If a zero vector returned, index was out of range and must be set to 0
    if(cp.size() == 0) {
      index = 0;
      cp = getCartProd(index, resultVals);
    }

    //Zip together the key/value pairs in the cartesian tuple
    vector<pair<string, string> > zipKey;
    for(int i=0; i<cp.size(); i++) {
      sprintf(vs, "%ld", cp[i]);
      string s(vs);
      zipKey.push_back(make_pair(resultKeys[i], s));
    }

    //Generate key to search for
    string key = makeVariable(L, output, zipKey);

    //Continuously attempt to solve until result point complete
    while(rcht.find(key) == rcht.end()) {
      if(depStack.size() > 0) {
        attempt = depStack.top();
        depStack.pop();
      } else {
        attempt = key;
      }
      bool solvesIt = false;
      while(!solvesIt) {
        bool codeletMatch = false;
        vector<Codelet> candidates = problem.getCodelets(getBlankVariable(attempt));
        vector<string> attemptDeps, attemptOuts;
        vector<string> missing;
        map<string, double> values;
        for(int i =0; i<candidates.size() && !codeletMatch; i++) {
          attemptDeps.clear();
          attemptOuts.clear();
          if(candidates[i].matchesDep(L, attempt, attemptDeps, attemptOuts)) {
            codeletMatch = true;
            code = candidates[i].getCode();
            values.clear();
            missing.clear();
            //Check to see if all of the dependencies have been solved
            for(int j=0; j<attemptDeps.size(); j++) {
              if(rcht.find(attemptDeps[j]) == rcht.end())
                missing.push_back(attemptDeps[j]);
              else
                values[attemptDeps[j]] = rcht[attemptDeps[j]];
            }
          }
        }
        //If no codelet matches, check the data files for a key that does
        if(!codeletMatch) {
          cout << "CRAP! Couldn't find a match for " << attempt << endl;
          vector<string> data = problem.getData();
          for(int d=0; d<data.size(); d++) {
            //Check the filename -- if URL, use CURL, otherwise fopen
            if(strstr(data[d].c_str(), "http://")) {
              //URL - go to web and retrieve the file
            } else {
	      //Local file, just grep out the key we need
              string cmd = "cat " + data[d] + " | grep '" + attempt +"'";
	      FILE *fp = popen(cmd.c_str(), "r");
              double value;
              if(fscanf(fp, "%*s %f", &value)) {
                #pragma omp critical
                rcht[attempt] = value;
                pclose(fp);
                break;
              }
              pclose(fp);
            }
          }
          exit(1);
          //We need to go back to the result point now
          break;
        }
        //If dependencies were missing, attempt to solve one of those dependencies
        else if(missing.size()) {
          depStack.push(attempt);
          attempt = missing[rand() % missing.size()];
        }
        //If there was a codelet match and were no missing dependencies, then it's time to solve 
        else {
          //Evaluate the code
          string expr = code.expression;
          for(int n=0; n<code.inputs.size(); n++) {
            sprintf(vs, "%lf", values[attemptDeps[n]]);
            string os(vs);
            boost::replace_all(expr, code.inputs[n], os);
          }
          double value = ae_eval(L, (char*)expr.c_str());

          //Store the returned value in the table
          #pragma omp critical
          rcht[attempt] = value;
          cout << attempt << " " << value << endl;

          solvesIt = true;
        }
      }
    }
    index++;
  } while(index != origIndex);

  cout << "No more work, cleaning up" << endl;
  ae_close(L);
}
  return 0;
}
