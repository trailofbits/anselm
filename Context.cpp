#include "Context.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

#include <stack>
#include <string>
using namespace std;

string toString(Instruction* I) {
	if (auto C = dyn_cast<CallInst>(I))
		if (auto F = C->getCalledFunction())
			return F->getName();
	return to_string((unsigned long) I);
}

Context::Context() {
	names.insert("EVP_CIPHER_CTX_new");
	names.insert("EVP_CIPHER_CTX_free");
	names.insert("EVP_EncryptInit_ex");
	names.insert("EVP_EncryptUpdate");
	names.insert("EVP_EncryptFinal_ex");
}

void Context::pass(Function &F) {
	errs() << string(100, '-') << "\n" << F.getName() << "\n";
	traverse(F);
}

void Context::traverse(Function &F) {
	DirectedGraph graph;

	// populate graph
	for (auto &B : F) {
		// include the first instruction of a block
		Node node = &B.front();
		graph[node];

		// linked list of relevant instructions within a block
		for (auto &I : B) {
			if (isRelevant(I) && !I.isIdenticalTo(node)) {
				graph[node].second.emplace(&I);
				graph[&I].first.emplace(node);
				node = &I;
			}
		}

		// create edges between linked lists
		for (auto it = succ_begin(&B); it != succ_end(&B); ++it) {
			Node edge = &(*it)->front();
			graph[edge].first.emplace(node);
			graph[node].second.emplace(edge);
		}
	}

	// compress graph
	for (auto &B : F) {
		// remove irrelevant first instruction of blocks
		if (!isRelevant(B.front())) {
			Node node = &B.front();
			Edges &preds = graph.at(node).first;
			Edges &succs = graph.at(node).second;

			// remove node from all predecessors
			for (Node pred : preds) {
				Edges &edges = graph.at(pred).second;
				edges.erase(node);
				for (Node succ : succs)
					edges.emplace(succ);
			}

			// remove node from all successors
			for (Node succ : succs) {
				Edges &edges = graph.at(succ).first;
				edges.erase(node);
				for (Node pred : preds)
					edges.emplace(pred);
			}

			graph.erase(node);
		}
	}

	Path start;
	start.push_back(&F.getEntryBlock().front());
	stack<Path> paths;
	paths.push(start);
	while (!paths.empty()) {
		Path path = paths.top();
		paths.pop();
		Node back = path.back();
		Edges &succs = graph.at(back).second;
		if (succs.empty()) {
			for (Node node : path) {
				errs() << toString(node) << "; ";
			}
			errs() << "\n";
		} else {
			for (Node succ : succs) {
				paths.push(path);
				paths.top().push_back(succ);
			}
		}
	}
}

// Checks whether an instruction should be included in the final graph.
bool Context::isRelevant(Instruction &I) {
	auto B = I.getParent();

	// first instruction of a block that has no predecessors
	if (pred_begin(B) == pred_end(B) && I.isIdenticalTo(&B->front()))
		return true;

	// last instruction of a block that has no successors
	if (succ_begin(B) == succ_end(B) && I.isIdenticalTo(&B->back()))
		return true;

	// instruction is an allowed call
	if (auto call = dyn_cast<CallInst>(&I)) {
		if (auto func = call->getCalledFunction()) {
			if (names.find(func->getName()) != names.end())
				return true;
		}
	}
	return false;
}
