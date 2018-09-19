#include <iostream>
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

DeclarationMatcher RecordMatcher = cxxRecordDecl( isExpansionInMainFile(), isDefinition() ).bind( "records" );
DeclarationMatcher NamespaceMatcher = namespaceDecl( isExpansionInMainFile(), unless( isAnonymous() ) ).bind( "namespaces" );

void AdjustSourceRangeToLeftBrace( clang::SourceRange& SR, clang::SourceLocation BraceLoc ) {
	if( BraceLoc < SR.getEnd() && !( BraceLoc < SR.getBegin() ) ) SR.setEnd( BraceLoc );
}

std::string GetSourceString( clang::SourceRange SR, clang::SourceManager& SM, clang::LangOptions LO ) {
	return Lexer::getSourceText(
		CharSourceRange::getCharRange(
			SR.getBegin(),
			Lexer::getLocForEndOfToken( SR.getEnd(), 0, SM, LO )
		),
		SM, LO, nullptr
	).str();
}

void PrintAttributes( Decl const* D, clang::SourceManager& SM, clang::LangOptions LO ) {
	clang::Decl::attr_range const Attributes = D->attrs();
	if( ( Attributes.end() - Attributes.begin() ) > 0 ) {
		std::cout << "[[";
		for( Attr const* Attribute : Attributes ) {
			std::cout << GetSourceString( Attribute->getRange(), SM, LO ) << ", ";
		}
		std::cout << "]]";
	}
}

void PrintComment( Decl const* D, clang::ASTContext& CT ) {
	if( clang::RawComment const* RC = CT.getRawCommentForDeclNoCache( D ) ) {
		std::cout << "//";
		std::string brief = RC->getBriefText( CT );
		std::cout << brief;
	}
}

class PrimaryVisitor : public ConstDeclVisitor<PrimaryVisitor> {
public:
	clang::ASTContext* context;
	clang::SourceManager* sourceManager;
	clang::LangOptions langOptions;

	void VisitCXXRecordDecl( CXXRecordDecl const* Record ) {
		if( Record->isAnonymousStructOrUnion() || !Record->isCompleteDefinition() ) return;
		std::cout << "CXXRecord >> " << Record->getNameAsString() << " ";
		PrintAttributes( Record, *sourceManager, langOptions );
		PrintComment( Record, *context );
		std::cout << std::endl;
	}

	void VisitNamespaceDecl( NamespaceDecl const* Namespace ) {
		if( Namespace->isAnonymousNamespace() ) return;
		std::cout << "Namespace >> " << Namespace->getNameAsString() << " ";
		PrintAttributes( Namespace, *sourceManager, langOptions );
		PrintComment( Namespace, *context );
		std::cout << std::endl;
	}
};

class NestedVisitor : public ConstDeclVisitor<NestedVisitor> {
public:
	clang::ASTContext* context;
	clang::SourceManager* sourceManager;
	clang::LangOptions langOptions;

	void VisitFieldDecl( FieldDecl const* Field ) {
		std::cout << "\tField >> " << Field->getType().getAsString() << " " << Field->getNameAsString() << " ";
		PrintAttributes( Field, *sourceManager, langOptions );
		PrintComment( Field, *context );
		std::cout << std::endl;
	}

	void VisitCXXMethodDecl( CXXMethodDecl const* Method ) {
		if( Method->isImplicit() || CXXConstructorDecl::classof( Method ) || CXXDestructorDecl::classof( Method ) || CXXConversionDecl::classof( Method ) ) return;
		std::cout << "\tMethod >> ";
		std::cout << Method->getReturnType().getAsString() << " " << Method->getNameAsString() << "( ";

		for( ParmVarDecl const* Parameter : Method->parameters() ) {
			std::cout << Parameter->getType().getAsString() << " " << Parameter->getNameAsString();
			PrintAttributes( Parameter, *sourceManager, langOptions );
			std::cout << ", ";
		}
		std::cout << ") ";

		PrintAttributes( Method, *sourceManager, langOptions );
		PrintComment( Method, *context );
		std::cout << std::endl;
	}
};

class Printer : public MatchFinder::MatchCallback {
public:
  virtual void run( MatchFinder::MatchResult const& Result ) {
		PrimaryVisitor PVisitor;
		PVisitor.context = Result.Context;
		PVisitor.sourceManager = Result.SourceManager;
		PVisitor.langOptions = LangOptions{};

		NestedVisitor NVisitor;
		NVisitor.context = Result.Context;
		NVisitor.sourceManager = Result.SourceManager;
		NVisitor.langOptions = LangOptions{};

	if( RecordDecl* Record = const_cast<RecordDecl*>( Result.Nodes.getNodeAs<clang::RecordDecl>( "records" ) ) ) {
			std::cout << "//==================================================================" << std::endl;
			PVisitor.Visit( Record );
			for( Decl* Sub : Record->decls() ) {
				NVisitor.Visit( Sub );
			}
		} else if( NamespaceDecl* Namespace = const_cast<NamespaceDecl*>( Result.Nodes.getNodeAs<clang::NamespaceDecl>( "namespaces" ) ) ) {
			std::cout << "//==================================================================" << std::endl;
			PVisitor.Visit( Namespace );
			//PrintDeclSource( Namespace, *Result.SourceManager );
			for( Decl* Sub : Namespace->decls() ) {
				NVisitor.Visit( Sub );
			}
		}
  }
};

class PrintIncludes : public PPCallbacks
{
public:
	virtual void InclusionDirective(
		SourceLocation hash_loc,
		const Token &include_token,
		StringRef file_name,
		bool is_angled,
		CharSourceRange filename_range,
		const FileEntry *file,
		StringRef search_path,
		StringRef relative_path,
		const clang::Module *imported
	) override {
		std::cout << "//==== New Include" << std::endl;
		std::cout << "#include " << file_name.str() << std::endl;
	}
};

class GeneratedIncludeAction : public PreprocessOnlyAction {
	void ExecuteAction() {
		getCompilerInstance().getPreprocessor().addPPCallbacks(
			std::make_unique<PrintIncludes>()
		);

		clang::PreprocessOnlyAction::ExecuteAction();
	}
};

int main(int argc, const char **argv) {
	std::cout << "I'm Alive!" << std::endl;
	// CommonOptionsParser constructor will parse arguments and create a
	// CompilationDatabase.  In case of error it will terminate the program.
	//@todo switch to static create method so that errors can be handled gracefully
	CommonOptionsParser OptionsParser( argc, argv, MyToolCategory );

	Printer Printer;
	MatchFinder Finder;
	Finder.addMatcher( RecordMatcher, &Printer );
	Finder.addMatcher( NamespaceMatcher, &Printer );

	static char Symbol = 0;
	std::cout << std::endl << CompilerInvocation::GetResourcesPath( "clang-tool", &Symbol ) << std::endl << std::endl;

	std::cout << "Source Files:\n";
	CompilationDatabase& Database = OptionsParser.getCompilations();
	const std::vector<std::string>& SourcePaths = OptionsParser.getSourcePathList();
	for( std::string const& S : SourcePaths ) {
		std::vector<CompileCommand> Commands = Database.getCompileCommands( S );
		for( CompileCommand const& Command : Commands ) {
			std::cout << "\t" << Command.Directory << "$> ";
			for( std::string const& C : Command.CommandLine ) {
				std::cout << C << " ";
			}
			std::cout << std::endl;
		}
	}

	ClangTool Tool( Database, SourcePaths );
	return Tool.run( newFrontendActionFactory<GeneratedIncludeAction>().get() );
}
