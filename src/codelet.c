#include <string.h>
#include <stdlib.h>
#include "codelet.h"
#include "ae.h"

char *getVariableBasename(const char *varStr) {
  char **split_str;
  int n;
  //Split the input string into its base and parameters
  split_str = split(varStr, "[]", &n);
  
  return split_str[0];
}

char **getVariableParameters(const char *varStr, int *numparams) {
  char **split_str;
  int i,j,n,l;

  //Split the input string into its base and parameters
  split_str = split(varStr, "[]", &n);

  char **parameters = NULL;
  j=0;
  for(i=1; i<n; i++) {
    l = strlen(split_str[i]);
    if(l > 0) {
      parameters = (char**)realloc(parameters, sizeof(char*)*(j+1));
      parameters[j] = (char*)malloc(sizeof(char)*(l+1));
      strcpy(parameters[j++], split_str[i]);
    }
  }

  *numparams = j;
  return parameters;
}

char *getBlankVariable(const char *varStr) {
  int n;
  char *s = getVariableBasename(varStr);
  char **params = getVariableParameters(varStr, &n);
  s = (char*)realloc(s, sizeof(char)*(strlen(s)+(n*2)+1));

  for(int i=0; i<n; i++) strcat(s, "[]");

  return s;
}

char *makeVariable(lua_State *L, const char *varStr, const kvlist_type pairs) {
  double value;
  char sTmp[256];
  int i,k,n;
  char *p;

  char *s = getVariableBasename(varStr);
  char **parameters = getVariableParameters(varStr, &n);

  for(k=0; k<n; k++) {
    p = (char*)malloc(sizeof(char)*(strlen(parameters[k])+1));
    strcpy(p, parameters[k]);
    for(i=0; i<pairs.size; i++)
      p = replace_all(p, pairs.keys[i], pairs.vals[i]);
    value = ae_eval(L, p);
    sprintf(sTmp, "[%ld]", (int)value);
    s = (char*)realloc(s, sizeof(char)*(strlen(s) + strlen(sTmp) + 1));
    strcat(s, sTmp);
  }
  return s;
}

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
