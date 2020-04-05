#include "Context.h"

#include "llvm/Pass.h"
using namespace llvm;

namespace {
	struct Anselm : public FunctionPass {
		static char ID;
		Anselm() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &F) override {
			Context context;
			context.pass(F);
			return false;
		}
	};
}

char Anselm::ID = 0;
static RegisterPass<Anselm> X("anselm", "Anselm", true, true);
