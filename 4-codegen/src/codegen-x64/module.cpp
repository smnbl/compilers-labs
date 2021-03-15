#include "codegen-x64/module.hpp"

codegen_x64::Instruction::Instruction(const std::string &opcode,
                                      const std::vector<std::string> &operands,
                                      const std::string &comment)
    : opcode(opcode), operands(operands), comment(comment) {}

codegen_x64::BasicBlock::BasicBlock(const std::string &name,
                                    const std::string &comment, bool is_global)
    : name(name), comment(comment), is_global(is_global) {}

llvm::formatted_raw_ostream &
codegen_x64::operator<<(llvm::formatted_raw_ostream &os,
                        const codegen_x64::Instruction &ins) {
    os.PadToColumn(4);
    os << ins.opcode;
    os.PadToColumn(16);

    bool first = true;

    for (const auto &operand : ins.operands) {
        if (!first) {
            os << ", ";
        }
        first = false;

        os << operand;
    }

    if (!ins.comment.empty()) {
        os.PadToColumn(40);
        os << "# " << ins.comment;
    }

    os << "\n";

    return os;
}

llvm::formatted_raw_ostream &
codegen_x64::operator<<(llvm::formatted_raw_ostream &os,
                        const codegen_x64::BasicBlock &bbl) {
    os << "\n";
    if (bbl.is_global)
        os << ".global " << bbl.name << "\n";

    os << bbl.name << ":";

    if (!bbl.comment.empty()) {
        os.PadToColumn(40);
        os << "# " << bbl.comment;
    }

    os << "\n";

    for (const auto &ins : bbl.instructions) {
        os << ins;
    }

    return os;
}

llvm::formatted_raw_ostream &
codegen_x64::operator<<(llvm::formatted_raw_ostream &os,
                        const codegen_x64::Module &mod) {
    for (const auto &bbl : mod.blocks) {
        os << bbl;
    }

    return os;
}

codegen_x64::Module &
codegen_x64::operator<<(codegen_x64::Module &mod,
                        const codegen_x64::BasicBlock &bbl) {
    mod.blocks.emplace_back(bbl);
    return mod;
}

codegen_x64::Module &
codegen_x64::operator<<(codegen_x64::Module &mod,
                        const codegen_x64::Instruction &ins) {
    mod.blocks.back().instructions.emplace_back(ins);
    return mod;
}
