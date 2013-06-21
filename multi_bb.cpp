#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/Target/TargetSelect.h"
#include <iostream>
using namespace llvm;

/*
** Author: Richard Lee
** Note: this program is only tested with LLVM 2.9.
** LLVM is in development, and the API changes a lot over versions.
** If you compile it with another version, there might be compling errors.
*/
int main(){
	// Module Construction
	LLVMContext & context = llvm::getGlobalContext();
    Module* module = new Module("test", context);

    //Function Construction
    Constant* c = module->getOrInsertFunction("foo",
    /*ret type*/                           Type::getVoidTy(context),
    /*args*/                               Type::getVoidTy(context),
    /*varargs terminated with null*/       NULL);
    Function* foo = cast<Function>(c);
    foo->setCallingConv(CallingConv::C);

    //Basic Block Construction.
    BasicBlock* entryBlock = BasicBlock::Create(context, "entry", foo);

    IRBuilder<> builder(entryBlock);

    //Create three constant integer x, y, z.
    Value *x = ConstantInt::get(Type::getInt32Ty(context), 3);
    Value *y = ConstantInt::get(Type::getInt32Ty(context), 2);
    Value *z = ConstantInt::get(Type::getInt32Ty(context), 1);
    
    //addr = &value
    /* we will check the value of 'value' and see
    ** whether the function we construct is running correctly.
    */
    long value = 10;
    Value * addr = builder.CreateIntToPtr(
        ConstantInt::get(Type::getInt64Ty(context), (uint64_t)&value),
        Type::getInt64PtrTy(context),
        "addr"
    );

    // mem = [addr]
    Value* mem = builder.CreateLoad(addr, "mem");
    // tmp = 3*mem
    Value* tmp = builder.CreateBinOp(Instruction::Mul,
                                     x, mem, "tmp");
    // tmp2 = tmp+2
    Value* tmp2 = builder.CreateBinOp(Instruction::Add,
                                      tmp, y, "tmp2");
    // tmp3 = tmp2-1
    Value* tmp3 = builder.CreateBinOp(Instruction::Sub,
                                      tmp2, z, "tmp3");
    // [addr] = mem
    builder.CreateStore(tmp3, addr); 
    // goto exit
    BasicBlock* exitBlock = BasicBlock::Create(context, "exit", foo);
    builder.CreateBr(exitBlock);

    // Start to add instruction for 'exit' block
    builder.SetInsertPoint(exitBlock);
    // ret
    builder.CreateRetVoid();

    //Show the IR
    std::cout << "--------------- IR ---------------------" << std::endl;
    module->dump();
    std::cout << "--------------- End of IR --------------" << std::endl;

    //Create a ExecutionEngine of JIT kind.
    InitializeNativeTarget();
    ExecutionEngine *ee = EngineBuilder(module).setEngineKind(EngineKind::JIT)
        .setOptLevel(CodeGenOpt::None).create();

    //JIT the function
    void * fooAddr = ee->getPointerToFunction(foo);
    std::cout <<"address of function 'foo': " << std::hex << fooAddr << std::endl;

    void * entryAddr= ee->getPointerToBasicBlock(entryBlock);
    std::cout <<"address of block 'entry': " << std::hex << entryAddr << std::endl;

    void * exitAddr= ee->getPointerToBasicBlock(exitBlock);
    std::cout <<"address of block 'exit': " << std::hex << exitAddr << std::endl;

    //Run the function
    std::cout << std::dec << "Before calling foo: value = " << value <<  std::endl;
    typedef void (*FuncType)(void);
    FuncType fooFunc = (FuncType)fooAddr;
    fooFunc();
    std::cout << "After calling foo: value = " << value <<  std::endl;
    return 0;
}
