#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <optional>
#include <cstdlib>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "codegen/CodeGenerator.hpp"

namespace {

void printUsage(const char* executableName) {
	std::cout << "Usage: " << executableName << " [options] <source_file>" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  --help, -h             Show this help message and exit" << std::endl;
	std::cout << "  --verbose, -v          Enable verbose output during lexing/parsing" << std::endl;
	std::cout << "  --emit-obj <path>      Emit AOT object file (default: <source>.o)" << std::endl;
	std::cout << "  --emit-ir <path>       Emit LLVM IR (.ll) file" << std::endl;
	std::cout << "  --emit-exe <path>      Link object file into native executable" << std::endl;
}

std::string defaultObjectPathForSource(const std::string& sourcePath) {
	std::filesystem::path path(sourcePath);
	return (path.parent_path() / path.stem()).string() + ".o";
}

bool ensureApplicationEntrypoint(llvm::Module& module) {
	llvm::Function* appMain = module.getFunction("System.Application.main");
	if (!appMain) {
		return false;
	}

	if (module.getFunction("main")) {
		return true;
	}

	if (!appMain->getReturnType()->isVoidTy() || !appMain->arg_empty()) {
		return false;
	}

	llvm::LLVMContext& context = module.getContext();
	llvm::FunctionType* entryType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
	llvm::Function* entry = llvm::Function::Create(entryType, llvm::Function::ExternalLinkage, "main", module);

	llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", entry);
	llvm::IRBuilder<> builder(block);
	builder.CreateCall(appMain);
	builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));

	return !llvm::verifyFunction(*entry, &llvm::errs());
}

bool emitIRFile(const llvm::Module& module, const std::string& outputPath) {
	std::error_code errorCode;
	llvm::raw_fd_ostream outFile(outputPath, errorCode, llvm::sys::fs::OF_Text);
	if (errorCode) {
		std::cerr << "Error: Could not open IR output file '" << outputPath << "': " << errorCode.message() << std::endl;
		return false;
	}

	module.print(outFile, nullptr);
	outFile.flush();
	return true;
}

bool emitObjectFile(llvm::Module& module, const std::string& outputPath) {
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	const std::string targetTripleString = llvm::sys::getDefaultTargetTriple();
	llvm::Triple targetTriple(targetTripleString);
	module.setTargetTriple(targetTriple);

	std::string targetError;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTripleString, targetError);
	if (!target) {
		std::cerr << "Error: Failed to lookup target '" << targetTripleString << "': " << targetError << std::endl;
		return false;
	}

	llvm::TargetOptions options;
	auto relocationModel = std::optional<llvm::Reloc::Model>();
	std::unique_ptr<llvm::TargetMachine> targetMachine(
		target->createTargetMachine(targetTriple, "generic", "", options, relocationModel)
	);

	if (!targetMachine) {
		std::cerr << "Error: Failed to create LLVM target machine." << std::endl;
		return false;
	}

	module.setDataLayout(targetMachine->createDataLayout());

	std::error_code errorCode;
	llvm::raw_fd_ostream destination(outputPath, errorCode, llvm::sys::fs::OF_None);
	if (errorCode) {
		std::cerr << "Error: Could not open object output file '" << outputPath << "': " << errorCode.message() << std::endl;
		return false;
	}

	llvm::legacy::PassManager passManager;
	if (targetMachine->addPassesToEmitFile(passManager, destination, nullptr, llvm::CodeGenFileType::ObjectFile)) {
		std::cerr << "Error: LLVM target machine cannot emit object file for this target." << std::endl;
		return false;
	}

	passManager.run(module);
	destination.flush();
	return true;
}

bool linkExecutable(const std::string& objectPath, const std::string& executablePath) {
	std::string command = "cc -no-pie \"" + objectPath + "\" -o \"" + executablePath + "\"";
	const int result = std::system(command.c_str());
	if (result != 0) {
		std::cerr << "Error: Linker failed while creating executable. Command: " << command << std::endl;
		return false;
	}
	return true;
}

}

int main (int argc, char *argv[])
{
	if (argc <= 1) {
		printUsage(argv[0]);
		return 0;
	}

	bool verbose = false;
	std::string sourceFile;
	std::string emitObjectPath;
	std::string emitIRPath;
	std::string emitExecutablePath;

	for (int i = 1; i < argc; ++i) {
		std::string argument = argv[i];

		if (argument == "--help" || argument == "-h") {
			printUsage(argv[0]);
			return 0;
		}

		if (argument == "--verbose" || argument == "-v") {
			verbose = true;
			continue;
		}

		if (argument == "--emit-obj") {
			if (i + 1 >= argc) {
				std::cerr << "Error: Missing path after --emit-obj." << std::endl;
				return 1;
			}
			emitObjectPath = argv[++i];
			continue;
		}

		if (argument == "--emit-ir") {
			if (i + 1 >= argc) {
				std::cerr << "Error: Missing path after --emit-ir." << std::endl;
				return 1;
			}
			emitIRPath = argv[++i];
			continue;
		}

		if (argument == "--emit-exe") {
			if (i + 1 >= argc) {
				std::cerr << "Error: Missing path after --emit-exe." << std::endl;
				return 1;
			}
			emitExecutablePath = argv[++i];
			continue;
		}

		if (!argument.empty() && argument[0] == '-') {
			std::cerr << "Error: Unknown option '" << argument << "'." << std::endl;
			return 1;
		}

		if (!sourceFile.empty()) {
			std::cerr << "Error: Multiple source files provided. Only one is currently supported." << std::endl;
			return 1;
		}

		sourceFile = argument;
	}

	if (sourceFile.empty()) {
		std::cerr << "Error: Missing source file argument." << std::endl;
		printUsage(argv[0]);
		return 1;
	}

	if (emitObjectPath.empty()) {
		emitObjectPath = defaultObjectPathForSource(sourceFile);
	}

	std::ifstream file(sourceFile);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << sourceFile << std::endl;
		return 1;
	}

	std::string text((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());

	Lexer lexer = Lexer(text);
	if (verbose)
	{
		std::cout << "Tokens:" << std::endl;
		while (true) {
			int16_t token = lexer.getNextToken();
			if (token == tok_eof) {
				break;
			}
			std::cout << "\t" << static_cast<Token>(token) << std::endl;
		}

		lexer = Lexer(text);
	}

	Parser parser = Parser(lexer, verbose);
	CodeGenerator generator;
	if (!parser.parse(generator)) {
		std::cerr << "Error: Compilation failed before AOT emission." << std::endl;
		return 1;
	}

	llvm::Module* module = generator.getModule();
	if (!module) {
		std::cerr << "Error: Internal compiler error: code generator produced no module." << std::endl;
		return 1;
	}

	if (!emitIRPath.empty() && !emitIRFile(*module, emitIRPath)) {
		return 1;
	}

	if (!emitExecutablePath.empty()) {
		if (!ensureApplicationEntrypoint(*module)) {
			std::cerr << "Error: Unable to synthesize native entrypoint from System.Application.main." << std::endl;
			return 1;
		}
	}

	if (!emitObjectFile(*module, emitObjectPath)) {
		return 1;
	}

	if (!emitExecutablePath.empty() && !linkExecutable(emitObjectPath, emitExecutablePath)) {
		return 1;
	}

	if (verbose) {
		std::cout << "AOT object emitted: " << emitObjectPath << std::endl;
		if (!emitIRPath.empty()) {
			std::cout << "LLVM IR emitted: " << emitIRPath << std::endl;
		}
		if (!emitExecutablePath.empty()) {
			std::cout << "Executable emitted: " << emitExecutablePath << std::endl;
		}
	}

	return 0;
}
