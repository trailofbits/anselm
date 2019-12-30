#include <unordered_set>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace std;
using namespace llvm;

using dg = unordered_map<Instruction*, pair<unordered_set<Instruction*>, unordered_set<Instruction*>>>;

unordered_set<string> getAllowed() {
	unordered_set<string> allowed;
	allowed.insert("EVP_CIPHER_CTX_new");
	allowed.insert("EVP_CIPHER_CTX_free");
	allowed.insert("EVP_EncryptInit_ex");
	allowed.insert("EVP_EncryptUpdate");
	allowed.insert("EVP_EncryptFinal_ex");
	return allowed;
}

string represent(Instruction* I) {
	if (auto call = dyn_cast<CallInst>(I))
		if (auto func = call->getCalledFunction())
			return func->getName();
	return to_string((unsigned long) I);
}

void runAnalysis(unordered_map<Function*, dg> graphs) {
	for (auto& [F, graph] : graphs) {
		errs() << "\n" << F->getName() << "\n----------\n";
		for (auto& [node, edges] : graph) {
			errs() << represent(node) << " -> ";
			for (auto edge : edges.second) {
				errs() << represent(edge) << " ";
			}
			errs() << "\n";
		}
	}
}
