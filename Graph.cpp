#include <unordered_set>
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "Pattern.h"

using namespace std;
using namespace llvm;

using dg = unordered_map<Instruction*, pair<unordered_set<Instruction*>, unordered_set<Instruction*>>>;

namespace {
	struct GraphPass : public ModulePass {
		static char ID;
		GraphPass() : ModulePass(ID) { }

		// isRelevant checks whether an individual instruction should be
		// included in the final graph.
		bool isRelevant(Instruction &I, unordered_set<string> allowed) {
			auto B = I.getParent();

			// first instruction of a block that has no predecessors
			if (pred_begin(B) == pred_end(B) && I.isIdenticalTo(&B->front()))
				return true;

			// last instruction of a block that has no successors
			if (succ_begin(B) == succ_end(B) && I.isIdenticalTo(&B->back()))
				return true;

			// instruction is a call to a function
			if (auto call = dyn_cast<CallInst>(&I)) {
				if (auto func = call->getCalledFunction()) {
					// marked for analysis
					if (allowed.find(func->getName()) != allowed.end())
						return true;

					// defined in bitcode
					if (!func->isDeclaration())
						return true;
				}
			}
			return false;
		}

		// getGraph creates a directed graph for a function with nodes
		// representing relevant instructions. Edges represent possible
		// paths of execution.
		dg getGraph(Function &F, unordered_set<string> allowed) {
			dg graph;

			// populate graph
			for (auto& B : F) {
				// always include the first instruction of a block for
				// intermediate use
				auto node = &B.front();
				graph[node];

				// create a linked list of relevant instructions within a block
				for (auto& I : B) {
					if (isRelevant(I, allowed) && !I.isIdenticalTo(node)) {
						graph[node].second.emplace(&I);
						graph[&I].first.emplace(node);
						node = &I;
					}
				}

				// create edges between blocks' linked lists
				for (auto it = succ_begin(&B); it != succ_end(&B); ++it) {
					auto edge = &(*it)->front();
					graph[edge].first.emplace(node);
					graph[node].second.emplace(edge);
				}
			}

			// compress graph by removing irrelevant first instructions
			// while maintaining overall paths
			for (auto& B : F) {
				if (!isRelevant(B.front(), allowed)) {
					auto node = &B.front();
					auto& edges = graph.at(node);

					// remove node from all predecessors
					for (auto& edge : edges.first) {
						auto& succs = graph.at(edge).second;
						succs.erase(node);
						for (auto succ : edges.second)
							succs.emplace(succ);
					}

					// remove node from all successors
					for (auto& edge : edges.second) {
						auto& preds = graph.at(edge).first;
						preds.erase(node);
						for (auto pred : edges.first)
							preds.emplace(pred);
					}

					graph.erase(node);
				}
			}

			return graph;
		}

		// runOnModule collects all function graphs and runs analyses
		virtual bool runOnModule(Module &M) override {
			auto allowed = getAllowed();
			unordered_map<Function*, dg> graphs;

			// only graph functions defined in bitcode
			for (auto& F : M)
				if (!F.isDeclaration())
					graphs[&F] = getGraph(F, allowed);

			runAnalysis(graphs);
			return false;
		}
	};
}

char GraphPass::ID = 0;
static RegisterPass<GraphPass> X("graph", "GraphPass", true, true);
