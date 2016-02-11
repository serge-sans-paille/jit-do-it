#define DEBUG_TYPE "ReadAttributes"

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"

#ifndef NDEBUG
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Debug.h"
#endif

/* for stat support */
#include "llvm/ADT/Statistic.h"
STATISTIC(NumberFunctionsToJit, "The # of functions to jit");

#include <set>

using namespace llvm;

static const char JitAnnotation[] = "jitme";

class ReadAttributes : public ModulePass {
    private:
        using FunctionSet = std::set<Function*>;
        FunctionSet FunctionsToJit;

    public:
        static char ID;
        ReadAttributes() : ModulePass(ID) {}

        void print(llvm::raw_ostream &O, const Module *) const override {
          for(auto&& F : FunctionsToJit)
            O << F->getName() << "\n";
        }

        bool runOnModule(Module &mod) override {
            FunctionsToJit.clear();

            auto Annotations = mod.getNamedGlobal("llvm.global.annotations");
            if(!Annotations) {
                DEBUG(dbgs() << "No annotations found!\n");
                return false;
            }

            auto AnnotArray = cast<ConstantArray>(Annotations->getOperand(0));
            for(auto const& op: AnnotArray->operands()) {
                auto AnnotStruct = cast<ConstantStruct>(op);

                if (auto Fn = dyn_cast<Function>(AnnotStruct->getOperand(0)->getOperand(0))) {
                    auto Anno = cast<ConstantDataArray>(cast<GlobalVariable>(AnnotStruct->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
                    if(Anno == JitAnnotation) {
                        FunctionsToJit.insert(Fn);
                        ++NumberFunctionsToJit;
                        DEBUG(dbgs() << "Found 'jitme' annotation on function: " << Fn->getName() << "\n");
                    }
                }
            }
            return false;
        }

        void getAnalysisUsage(AnalysisUsage &Info) const {
            //Info.addRequired<DominatorTreeWrapperPass>();
            Info.setPreservesAll();
        }

        FunctionSet const& getFunctionsToJit() const {
            return FunctionsToJit;
        }
};

char ReadAttributes::ID = 0;
static RegisterPass<ReadAttributes> X("ReadAttributes", "Read & display compiler attributes",
        true, true);

