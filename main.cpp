#include <iostream>

#include "llvm/ADT/Optional.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/KaleidoscopeJIT.h"

static llvm::LLVMContext s_Context;
static llvm::IRBuilder<> s_Builder(s_Context);
static std::unique_ptr<llvm::Module> s_Module;

int main(int argc, char** argv)
{
	if(argc <= 1)
	{
		std::cerr << "Usage : asdf <asdf-code>\n";
		return 1;
	}

	std::string code = argv[1];

	std::cout << "[asdf JIT compiler]\n";
	std::cout << "Note that log below is not actually happening here; it's status of code generating for those behaviors.\n";

	std::cout << "Making module...\n";
	s_Module = llvm::make_unique<llvm::Module>("top", s_Context);
	
	std::cout << "Defining main entry func...\n";
	llvm::Function* main_func = llvm::Function::Create
		(
			llvm::FunctionType::get(llvm::Type::getInt32Ty(s_Context), false),
			llvm::Function::ExternalLinkage, 
			"main",
			s_Module.get()
		);

	std::cout << "Setting insert point...\n";

	auto main_block = llvm::BasicBlock::Create(s_Context, "", main_func);
	s_Builder.SetInsertPoint(main_block);

	constexpr std::size_t arr_size = 3000;

	std::cout << "Declaring data and pointer...\n";
	llvm::Value* data = s_Builder.CreateAlloca(s_Builder.getInt8PtrTy(), nullptr, "data");
	llvm::Value* ptr = s_Builder.CreateAlloca(s_Builder.getInt8PtrTy(), nullptr, "ptr");
	llvm::Value* flag = s_Builder.CreateAlloca(s_Builder.getInt1Ty(), s_Builder.getInt1(true), "flag");

	std::cout << "Getting standard calloc function...\n";
	llvm::Function* calloc_func = llvm::cast<llvm::Function>
		(
		 s_Module->getOrInsertFunction("calloc", s_Builder.getInt8PtrTy(), s_Builder.getInt64Ty(), s_Builder.getInt64Ty(), nullptr)
		);

	std::cout << "Getting standard putchar function...\n";
	llvm::Function* putchar_func = llvm::cast<llvm::Function>
		(
		 s_Module->getOrInsertFunction("putchar", s_Builder.getInt32Ty(), s_Builder.getInt32Ty(), nullptr)
		);
	std::cout << "Getting standard getchar function...\n";
	llvm::Function* getchar_func = llvm::cast<llvm::Function>
		(
		 s_Module->getOrInsertFunction("getchar", s_Builder.getInt32Ty(), nullptr)
		);

	std::cout << "Allocating data pointer...(with calloc)\n";
	llvm::Value* data_ptr = s_Builder.CreateCall(calloc_func, {s_Builder.getInt64(arr_size), s_Builder.getInt64(1)});

	std::cout << "Storing to data and pointer...\n";
	s_Builder.CreateStore(data_ptr, data);
	s_Builder.CreateStore(data_ptr, ptr);

//	std::size_t cond_num = 0;
	for(char ch : code)
	{
//		std::string cond_num_str = std::to_string(cond_num);
		switch(ch)
		{
			case 'a':
				s_Builder.CreateStore(s_Builder.CreateXor(s_Builder.getTrue(), s_Builder.CreateLoad(flag)), flag);
				break;
			case 's':
				{
				llvm::BasicBlock* cond = llvm::BasicBlock::Create(s_Context, "cond", main_func);
				llvm::BasicBlock* cond_true = llvm::BasicBlock::Create(s_Context, "cond_true", main_func);
				llvm::BasicBlock* cond_false = llvm::BasicBlock::Create(s_Context, "cond_false", main_func);
				llvm::BasicBlock* cond_end = llvm::BasicBlock::Create(s_Context, "cond_end", main_func);
				llvm::Value* tmp_ptr = s_Builder.CreateLoad(ptr);

				s_Builder.CreateBr(cond);
				s_Builder.SetInsertPoint(cond);
				llvm::Value* flag_cmp = s_Builder.CreateICmpEQ(s_Builder.CreateLoad(flag), s_Builder.getInt1(true), "branch");
				s_Builder.CreateCondBr(flag_cmp, cond_true, cond_false);
				s_Builder.SetInsertPoint(cond_true);
				s_Builder.CreateStore(s_Builder.CreateInBoundsGEP(s_Builder.getInt8Ty(), s_Builder.CreateLoad(ptr), s_Builder.getInt32(1)), ptr);
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_false);
				s_Builder.CreateStore(s_Builder.CreateInBoundsGEP(s_Builder.getInt8Ty(), s_Builder.CreateLoad(ptr), s_Builder.getInt32(-1)), ptr);
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_end);
//				cond_num++;
				}
				break;
			case 'd':
				{
				llvm::BasicBlock* cond = llvm::BasicBlock::Create(s_Context, "cond", main_func);
				llvm::BasicBlock* cond_true = llvm::BasicBlock::Create(s_Context, "cond_true", main_func);
				llvm::BasicBlock* cond_false = llvm::BasicBlock::Create(s_Context, "cond_false", main_func);
				llvm::BasicBlock* cond_end = llvm::BasicBlock::Create(s_Context, "cond_end", main_func);
				llvm::Value* tmp_ptr = s_Builder.CreateLoad(ptr);

				s_Builder.CreateBr(cond);
				s_Builder.SetInsertPoint(cond);
				llvm::Value* flag_cmp = s_Builder.CreateICmpEQ(s_Builder.CreateLoad(flag), s_Builder.getInt1(true), "branch");
				s_Builder.CreateCondBr(flag_cmp, cond_true, cond_false);
				s_Builder.SetInsertPoint(cond_true);
				s_Builder.CreateStore(s_Builder.CreateAdd(s_Builder.CreateLoad(tmp_ptr), s_Builder.getInt8(1)), tmp_ptr);
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_false);
				s_Builder.CreateStore(s_Builder.CreateAdd(s_Builder.CreateLoad(tmp_ptr), s_Builder.getInt8(-1)), tmp_ptr);
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_end);
//				cond_num++;
				}
				break;
			case 'f':
				{
				llvm::BasicBlock* cond = llvm::BasicBlock::Create(s_Context, "cond", main_func);
				llvm::BasicBlock* cond_true = llvm::BasicBlock::Create(s_Context, "cond_true", main_func);
				llvm::BasicBlock* cond_false = llvm::BasicBlock::Create(s_Context, "cond_false", main_func);
				llvm::BasicBlock* cond_end = llvm::BasicBlock::Create(s_Context, "cond_end", main_func);
				llvm::Value* tmp_ptr = s_Builder.CreateLoad(ptr);

				s_Builder.CreateBr(cond);
				s_Builder.SetInsertPoint(cond);
				llvm::Value* flag_cmp = s_Builder.CreateICmpEQ(s_Builder.CreateLoad(flag), s_Builder.getInt1(true), "branch");
				s_Builder.CreateCondBr(flag_cmp, cond_true, cond_false);
				s_Builder.SetInsertPoint(cond_true);
				s_Builder.CreateCall(putchar_func, s_Builder.CreateSExt(s_Builder.CreateLoad(s_Builder.CreateLoad(ptr)), s_Builder.getInt32Ty()));
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_false);
				s_Builder.CreateStore(s_Builder.CreateTrunc(s_Builder.CreateCall(getchar_func), s_Builder.getInt8Ty()), s_Builder.CreateLoad(ptr));
				s_Builder.CreateBr(cond_end);
				s_Builder.SetInsertPoint(cond_end);
//				cond_num++;
				}
				break;
		}
	}

	std::cout << "Getting standard free function...\n";
	llvm::Function* free_func = llvm::cast<llvm::Function>(
      	s_Module->getOrInsertFunction("free", s_Builder.getVoidTy(), s_Builder.getInt8PtrTy(),
        nullptr));

	std::cout << "Free-ing allocated data...\n";
  	s_Builder.CreateCall(free_func, {s_Builder.CreateLoad(data)});

	std::cout << "Returning zero from main entry function...\n";
  	s_Builder.CreateRet(s_Builder.getInt32(0));

		
    auto FPM = llvm::make_unique<llvm::legacy::FunctionPassManager>(s_Module.get());
	FPM->add(llvm::createInstructionCombiningPass());
	FPM->add(llvm::createReassociatePass());
//	FPM->add(llvm::createGVNPass());
	FPM->add(llvm::createCFGSimplificationPass());
	FPM->doInitialization();

	FPM->run(*main_func);

	std::cout << "Writing LLVM IR to output.ll...\n";
	std::error_code err_code; llvm::raw_fd_ostream output("output.ll", err_code, llvm::sys::fs::F_None); 
	s_Module->print(output, nullptr);

	std::cout << "JIT\n";
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	llvm::orc::KaleidoscopeJIT jit;
	s_Module->setDataLayout(jit.getTargetMachine().createDataLayout());
		
	auto module_handle = jit.addModule(std::move(s_Module));
	auto main_symbol = jit.findSymbol("main");
	auto main_sym_address = (int(*)(void))main_symbol.getAddress();
	main_sym_address();

	std::cout << "Finished\n";
	return 0;
}
