#include "llvm/Support/TargetSelect.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>

#include "common.h"

using namespace std;
using namespace llvm;
// check if Type "ty" is a pointer to a defined class
bool check_type(Type * ty, const map<string, bool> & clsn) {
    if (!ty->isPointerTy())
        return false;
    Type * typ = ty->getPointerElementType();
    if (!typ->isStructTy())
        return false;
    string name = typ->getStructName();
    auto search = clsn.find(name);
    return (search != clsn.end());
}
// main process
ExitOnError ExitOnErr;
int main(int argc, char** argv) {
    // prepare LLVM
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    ExitOnErr.setBanner(string(argv[0]) + "error: ");
    // handle input error
    if (argc <= 2) {
        errs() << "Usage: <" << argv[0] << "> target.bc config.ivt\n";
        return -1;
    }
    // open ivt specification
    ifstream fin(argv[2]);
    if (!fin.is_open()) {
        errs() << "open ivt configure file <" << argv[2] << ">\n";
        return -1;
    }
    // read the ivt specification
    vector<string> lines;
    map<string, bool> clsn;             // map of IR class symbol
    vector< pair<string, long> > vptr;  // map from IR vptr symbol to index
    map<string, bool> ctor;             // map of IR constructors symbol
    long id = 0;
    while (!fin.eof()) {
        string line;
        getline(fin, line);
        if (line.empty())
            continue;
        lines.push_back(line);
        if (line[0] == '#')
            continue;
        string svp = trim(line);
        clsn.insert(pair<string, bool>(name2clsn(line), true));
        if (svp == "")
            vptr.push_back(pair<string, long>(name2vptr(line), id * sizeof(long)));
        else
            vptr.push_back(pair<string, long>(svp, id * sizeof(long)));
        ctor.insert(pair<string, bool>(name2ctor(line) + "C2E", true));
        ctor.insert(pair<string, bool>(name2ctor(line) + "D2E", true));
        id++;
    }
    fin.close();
    // print ivt specification data for debuging
    errs() << "\nclass symbol table\n";
    for (pair<string, bool> i : clsn)
        errs() << i.first << "\n";
    errs() << "\nvptr symbol table\n";
    for (pair<string, long> i : vptr)
        errs() << i.second << ":" << i.first << "\n";
    errs() << "\nconstructor symbol table\n";
    for (pair<string, bool> i : ctor)
       errs() << i.first << "\n";
    // handle IR .bc file
    unique_ptr<MemoryBuffer> mb = ExitOnErr(errorOrToExpected(MemoryBuffer::getFile(argv[1])));
    LLVMContext context;
    unique_ptr<Module> m = ExitOnErr(parseBitcodeFile(*mb.get(), context));
    errs() << "\nprocessing...\n";
    vector< pair<Instruction *, Instruction *> > toDel;
    GlobalVariable * gvar_ivt = new GlobalVariable(*m, Type::getInt64Ty(context), false, GlobalValue::ExternalLinkage, 0, "g__ivt");
    for (Function & f : (*m))
        if (!f.isDeclaration())
            for (BasicBlock & bb : f)
                for (Instruction & i : bb) {
                    if (i.getOpcode() == Instruction::Load) {
                        // looking for dispatch
                        Value * v = i.getOperand(0);
                        if (!isa<BitCastInst>(v))
                            continue;
                        Value * vt = v;
                        bool found = false;
                        while (isa<BitCastInst>(vt)) {
                            BitCastInst * bt = cast<BitCastInst>(vt);
                            Type * ty = bt->getSrcTy();
                            if (check_type(ty, clsn)) {
                                found = true;
                                break;
                            }
                            vt = bt->getOperand(0);
                        }
                        if (!found)
                            continue;
                        // handle a special case (in constructors)
                        bool in_ctor = false;
                        for (pair<string, bool> vp : ctor)
                            if (f.getName().startswith(vp.first)) {
                                in_ctor = true;
                                break;
                            }
                        if (in_ctor)
                            continue;
                        errs() << "Find dispatch:" << i << "\n";
                        // dispatch found, make changes
                        BitCastInst * b = cast<BitCastInst>(v);
                        LoadInst * t0 = new LoadInst(gvar_ivt, "", b);
                        BitCastInst * t1 = new BitCastInst(b->getOperand(0), Type::getInt64PtrTy(context), "", b);
                        LoadInst * t2 = new LoadInst(t1, "", b);
                        BinaryOperator * t3 = BinaryOperator::Create(Instruction::Add, t0, t2, "", b);
                        CastInst * t4 = CastInst::CreateBitOrPointerCast(t3, b->getDestTy(), "");
                        LoadInst * t5 = new LoadInst(t4, i.getName());
                        toDel.push_back(pair<Instruction *, Instruction *>(b, t4));
                        toDel.push_back(pair<Instruction *, Instruction *>(&i, t5));
                    } else if (i.getOpcode() == Instruction::Store) {
                        // looking for object creation
                        string fullname;
                        raw_string_ostream str(fullname);
                        i.getOperand(0)->print(str);
                        long id = -1;
                        for (pair<string, long> & vp : vptr)
                            if (fullname.find(vp.first) != string::npos) {
                                vp.first = fullname; // record the fullname for future checking
                                id = vp.second;
                                break;
                            }
                        if (id < 0)
                            continue;
                        errs() << "Find object creation (" << id << "):" << i << "\n";
                        // object creation found, make changes
                        BitCastInst * t0 = new BitCastInst(i.getOperand(1), Type::getInt64PtrTy(context), "", &i);
                        ConstantInt * ci = ConstantInt::getSigned(Type::getInt64Ty(context), id);
                        StoreInst * t1 = new StoreInst(ci, t0, &i);
                        toDel.push_back(pair<Instruction *, Instruction *>(&i, t1));
                    }
                }
    // actually remove or replace instructions
    errs() << "\nhandling...\n";
    for (pair<Instruction *, Instruction *> i : toDel) {
        if (i.second == nullptr) {
            errs() << "Erasing " << *(i.first) << "\n";
            i.first->eraseFromParent();
        } else {
            errs() << "Replacing " << *(i.first) << " with " << *(i.second) << "\n";
            BasicBlock::iterator ii(i.first);
            ReplaceInstWithInst(i.first->getParent()->getInstList(), ii, i.second);
        }
    }
    // write back IR .bc file
    errs() << "\nverifying...\n";
    verifyModule(*m, &errs());
    WriteBitcodeToFile(m.get(), outs());
    // write IR vptr symbol back to ivt specification for future use
    errs() << "\nwriting vptr symbol table\n";
    for (pair<string, long> i : vptr)
      errs() << i.second << ":" << i.first << "\n";
    ofstream fout(argv[2]);
    id = 0;
    for (string line : lines) {
        fout << line;
        if (line[0] == '#') {
        } else {
            string svp = trim(line);
            if (svp == "" && vptr[id].first != name2vptr(line))
                fout << "@" << vptr[id].first;
            id++;
        }
        fout << endl;
    }
    fout.close();
    return 0;
}

