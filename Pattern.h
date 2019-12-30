#include <unordered_set>

using namespace std;
using namespace llvm;

using dg = unordered_map<Instruction*, pair<unordered_set<Instruction*>, unordered_set<Instruction*>>>;

unordered_set<string> getAllowed();
void runAnalysis(unordered_map<Function*, dg> graphs);
