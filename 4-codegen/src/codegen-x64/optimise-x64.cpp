#include "codegen-x64/module.hpp"
#include "codegen-x64/optimise-x64.hpp"

#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <iterator>

#define DEBUG_TYPE "optimiser-x64"

// Command-line options to select which optimisation to apply.
llvm::cl::bits<codegen_x64::Optimisations> OptimisationsBits(
    llvm::cl::desc("Available peephole optimisations:"),
    llvm::cl::values(clEnumValN(codegen_x64::Optimisations::NopElimination,
                                "optimisation-nop-elimination",
                                "Eliminate NOP instructions"),
                     clEnumValN(codegen_x64::Optimisations::Assignment1,
                                "optimisation-1", "Your optimisation 1"),
                     clEnumValN(codegen_x64::Optimisations::Assignment2,
                                "optimisation-2", "Your optimisation 2"),
                     clEnumValN(codegen_x64::Optimisations::Assignment3,
                                "optimisation-3", "Your optimisation 3")));

// Statistic to see how many NOPs were removed.
STATISTIC(NumNopsEliminated,
          "The number of NOP instructions that were eliminated");

void codegen_x64::OptimiserX64::optimise(Module &module) {
    bool changed = true;

    while (changed) {
        changed = false;

        if (OptimisationsBits.isSet(Optimisations::NopElimination))
            changed |= optimiseNopElimination(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment1))
            changed |= optimise1(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment2))
            changed |= optimise2(module);

        if (OptimisationsBits.isSet(Optimisations::Assignment3))
            changed |= optimise3(module);
    }
}

bool codegen_x64::OptimiserX64::optimiseNopElimination(Module &mod) {
    bool changed = false;

    // Remove all "nop" instructions from all basic blocks.
    for (auto &bbl : mod.blocks) {
        auto &insns = bbl.instructions;
        auto it = std::remove_if(
            std::begin(insns), std::end(insns),
            [](const Instruction &ins) { return ins.opcode == "nop"; });

        // Update statistic
        NumNopsEliminated += std::distance(it, std::end(insns));

        // Check if anything was removed
        if (it != std::end(insns))
            changed = true;

        insns.erase(it, std::end(insns));
    }

    return changed;
}

bool codegen_x64::OptimiserX64::optimise1(Module &mod) {
    // ASSIGNMENT: Implement your first peephole optimisation here.
    // Return true only if you have changed the assembly.
    return false;
}

bool codegen_x64::OptimiserX64::optimise2(Module &mod) {
    // ASSIGNMENT: Implement your second peephole optimisation here.
    // Return true only if you have changed the assembly.
    return false;
}

bool codegen_x64::OptimiserX64::optimise3(Module &mod) {
    // ASSIGNMENT: Implement your third peephole optimisation here.
    // Return true only if you have changed the assembly.
    return false;
}

void codegen_x64::OptimiserX64::makeNop(Instruction &ins) {
    ins.opcode = "nop";
    ins.operands.clear();
}

