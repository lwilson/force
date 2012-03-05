use namespace std;

class ForceProblem {
  class Codelet {
    bool result;
    string name;
    map<string, string> params;
    vector<string> deps;
    vector<string> output;
    string code;

    public:
      void makeResult() {
        result = True;
      }
      void setName(string N) {
        name = N;
      }
      void addParameter(string p, string val) {
        params[p] = val;
      }
      void addDepend(string key) {
        deps.push_back(key);
      }
      void addOutput(string key) {
        output.push_back(key);
      }
      void addCode(string definition) {
        code = definition;
      }
      bool matchesDep(vector<string> &reqdeps, vector<string> &outputs) {
        
      }
  }
}
