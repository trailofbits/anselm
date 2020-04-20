#include "Context.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <stack>
#include <unordered_set>
using namespace std;

Context::Context(string filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		errs() << "Pattern file must be specified with anselm-pattern\n";
		exit(1);
	}

	ostringstream pattern;
	unordered_map<string, int> vars;

	string line;
	string token;
	bool negate = false;
	while (getline(file, line)) {
		istringstream stream(line);

		stream >> token;
		if (token == "!") {
			pattern << "(?!";
			stream >> token;
			negate = true;
		}

		pattern << "(?:[^:;]+(?::[0-9]+)+;)*";
		names.insert(token);

		pattern << token;
		while (stream >> token) {
			pattern << ":";
			if (token == "_") {
				pattern << "[0-9]+";
			} else {
				if (vars.find(token) == vars.end()) {
					vars[token] = vars.size() + 1;
					pattern << "([0-9]+)";
				} else {
					pattern << "\\" << to_string(vars[token]);
				}
			}
		}
		pattern << ";";

		if (negate) {
			pattern << ")";
			negate = false;
		}
	}

	regex = std::regex(pattern.str());
}

void Context::pass(Function &F) {
	errs() << F.getName() << "\n";
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
			inspect(path);
		} else {
			for (Node succ : succs) {
				paths.push(path);
				paths.top().push_back(succ);
			}
		}
	}
}

void Context::inspect(Path path) {
	// build string to match against regex
	ostringstream stream;
	for (Node node : path)
		if (auto call = dyn_cast<CallInst>(node))
			if (auto func = call->getCalledFunction())
				if (names.find(func->getName()) != names.end()) {
					// function name
					stream << func->getName().data();

					// return value
					stream << ":" << (unsigned long) call;

					// arguments
					for (auto &arg : call->args())
						stream << ":" << (unsigned long) arg.get();
					stream << ";";
				}

	// search for matches and print result
	errs() << "\t";
	if (regex_search(stream.str(), regex)) {
		errs() << "FAIL ";
	} else {
		errs() << "PASS ";
	}
	errs() << stream.str() << "\n";
}

bool Context::isRelevant(Instruction &I) {
	auto B = I.getParent();

	// first instruction of a block that has no predecessors
	if (pred_begin(B) == pred_end(B) && I.isIdenticalTo(&B->front()))
		return true;

	// last instruction of a block that has no successors
	if (succ_begin(B) == succ_end(B) && I.isIdenticalTo(&B->back()))
		return true;

	// instruction is an allowed call
	if (auto call = dyn_cast<CallInst>(&I))
		if (auto func = call->getCalledFunction())
			if (names.find(func->getName()) != names.end())
				return true;

	return false;
}
