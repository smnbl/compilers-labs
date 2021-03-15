#ifndef OPTIMISE_X64_HPP
#define OPTIMISE_X64_HPP

#include "codegen-x64/module.hpp"

#include "llvm/Support/CommandLine.h"

namespace codegen_x64 {

enum class Optimisations {
    NopElimination, // Eliminate NOP instructions
    Assignment1,    // Your optimisation 1
    Assignment2,    // Your optimisation 2
    Assignment3,    // Your optimisation 3
};

class OptimiserX64 {
  public:
    void optimise(Module &module);

  private:
    bool optimiseNopElimination(Module &mod);
    bool optimise1(Module &mod);
    bool optimise2(Module &mod);
    bool optimise3(Module &mod);

    // Make an instruction a NOP.
    void makeNop(Instruction &ins);

    // ASSIGNMENT: Add any helper methods or fields you need here.
};
} // namespace codegen_x64

#endif /* end of include guard: OPTIMISE_X64_HPP */
