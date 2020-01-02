#include <unordered_set>
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace std;
using namespace llvm;

using dg = unordered_map<Instruction*, pair<unordered_set<Instruction*>, unordered_set<Instruction*>>>;

bool isIdentical(Value* v1, Value* v2) {
	return !(v1 - v2) && v1 != nullptr && v2 != nullptr;
}

CallInst* getCall(Instruction *I) {
	return dyn_cast<CallInst>(I);
}

bool callsName(CallInst* C, string name) {
	if (auto F = C->getCalledFunction())
		return F->getName() == name;
	return false;
}

Value* getArg(CallInst* C, unsigned int position) {
	auto arg = C->arg_begin() + position;
	if (arg < C->arg_end())
		return arg->get();
	return nullptr;
}

Value* getReturn(CallInst* C) {
	return dyn_cast<Value>(C);
}

void new2init(dg graph) {
	for (auto& [node, edges] : graph)
		if (auto newCall = getCall(node))
			if (callsName(newCall, "EVP_CIPHER_CTX_new"))
				for (auto edge : edges.second) {
					if (auto initCall = getCall(edge))
						if (callsName(initCall, "EVP_EncryptInit_ex"))
							if (isIdentical(getReturn(newCall), getArg(initCall, 0)))
								continue;
					errs() << "* new2init\n";
				}
}

string getString(Instruction* I) {
	if (auto C = getCall(I))
		if (auto F = C->getCalledFunction())
			return F->getName();
	return to_string((unsigned long) I);
}

void printGraph(dg graph) {
	for (auto& [node, edges] : graph) {
		errs() << "\t" << getString(node) << " -> ";
		for (auto edge : edges.second) {
			errs() << getString(edge) << " ";
		}
		errs() << "\n";
	}
}

void runAnalysis(unordered_map<Function*, dg> graphs) {
	for (auto& [F, graph] : graphs) {
		errs() << string(100, '-') << "\n" << F->getName() << "\n";
		new2init(graph);
		printGraph(graph);
	}
}

unordered_set<string> getAllowed() {
	unordered_set<string> allowed;
	allowed.insert("EVP_CIPHER_CTX_new");
	allowed.insert("EVP_CIPHER_CTX_free");
	allowed.insert("EVP_EncryptInit_ex");
	allowed.insert("EVP_EncryptUpdate");
	allowed.insert("EVP_EncryptFinal_ex");
	return allowed;
}
