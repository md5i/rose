// C++ code calling an Jovial frontend function.

// sage3basic.h must be the first file included for the ROSE build system to work properly
//
#include "sage3basic.h"

#include "rose_config.h"

#include "jovial_support.h"
#include "preprocess.h"
#include "ATermToSageJovialTraversal.h"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>

#define ATERM_TRAVERSAL_ONLY 0
#define DEBUG_EXPERIMENTAL_JOVIAL 0
#define DEBUG_PREPROCESS_OUTPUT 0
#define OUTPUT_WHOLE_GRAPH_AST 0
#define OUTPUT_DOT_FILE_AST 0

#if OUTPUT_WHOLE_GRAPH_AST
#  include "wholeAST_API.h"
#endif

int jovial_main(int argc, char** argv, SgSourceFile* sg_source_file)
   {
     int status;
     std::string parse_table;
     std::string preprocessor;

     assert(sg_source_file != nullptr);

     std::string stratego_bin_path = STRATEGO_BIN_PATH;
     assert (stratego_bin_path.empty() == false);

  // Step 1 - Parse the input file
  // ------

  // The filename is obtained from the source-file object
     std::string filenameWithPath = sg_source_file->getFileName();
     std::string filenameWithoutPath = Rose::StringUtility::stripPathFromFileName(filenameWithPath);

  // Setup for preprocessing
     std::string preprocess_path = "src/frontend/Experimental_Jovial_ROSE_Connection";
     preprocessor = findRoseSupportPathFromBuild(preprocess_path, "bin") + "/jovial_preprocess";

     std::string commandString = preprocessor;
     commandString += " -i " + filenameWithPath;
     commandString += " | "  + stratego_bin_path + "/sglri";

  // Add path to the parse table (located in the source tree)
     std::string parse_table_path = "src/3rdPartyLibraries/experimental-jovial-parser/share/rose";
     parse_table = findRoseSupportPathFromSource(parse_table_path, "share/rose") + "/Jovial.tbl";
     commandString += " -p " + parse_table;

  // Add source code location information to output
     commandString += " --preserve-locations";

  // Output the transformed aterm file
     commandString += " -o " + filenameWithoutPath + ".aterm";

  // Make system call to run parser and output ATerm parse-tree file
     status = system(commandString.c_str());
     if (status != 0) {
       std::cerr << "\nFAILED: in jovial_main(), unable to parse file " << filenameWithoutPath;
       return status;
     }

  // Step 2 - Traverse the ATerm parse tree and convert into Sage nodes
  // ------

  // Initialize the ATerm library
     ATinitialize(argc, argv);

     std::string aterm_filename = filenameWithoutPath + ".aterm";

#if DEBUG_EXPERIMENTAL_JOVIAL
     std::cout << "PARSER command: " << commandString << "\n";
     std::cout << "OPENING ATerm parse-tree file " << aterm_filename << "\n";
#endif

  // Read the ATerm file that was created by the parser
     FILE * file = fopen(aterm_filename.c_str(), "r");
     if (file == nullptr) {
       std::cerr << "\nFAILED: in jovial_main(), unable to open file " << aterm_filename;
       std::cerr << "\n\n";
       return 1;
     }

     ATerm module_term = ATreadFromTextFile(file);
     fclose(file);

#if DEBUG_EXPERIMENTAL_JOVIAL
     std::cout << "SUCCESSFULLY read ATerm parse-tree file " << "\n";
#endif

  // Get the token stream for access to comments
     std::ifstream in_stream{filenameWithPath};
     std::ofstream out_stream{"/dev/null"};
     std::ostringstream oss;
     status = Jovial::preprocess(in_stream, out_stream, oss);
     if (status) {
       std::cerr << "\nFAILED: in jovial_main(), unable to read tokens for file "
                 << filenameWithPath << "\n\n";
       return status;
     }
     std::istringstream tokens{oss.str()};

  // Initialize the global scope and put it on the SageInterface scope stack
  // for usage by the sage tree builder during the ATerm traversal.
     Rose::builder::initialize_global_scope(sg_source_file);

     ATermSupport::ATermToSageJovialTraversal* aterm_traversal;
     aterm_traversal = new ATermSupport::ATermToSageJovialTraversal(sg_source_file, tokens);

     if (aterm_traversal->traverse_Module(module_term) != ATtrue) {
       std::cerr << "\nFAILED: in jovial_main(), unable to traverse ATerm file "
                 << aterm_filename << "\n\n";
       return 1;
     }

#if DEBUG_EXPERIMENTAL_JOVIAL
     std::cout << "SUCCESSFULLY traversed Jovial parse-tree" << "\n\n";
#endif
#if ATERM_TRAVERSAL_ONLY
     return 0;
#endif

#if OUTPUT_DOT_FILE_AST
  // Generate dot file for Sage nodes.
     generateDOT(SageBuilder::getGlobalScopeFromScopeStack(), filenameWithoutPath);
#endif

#if OUTPUT_WHOLE_GRAPH_AST
     std::vector<std::string> argList;
     argList.push_back("-DSKIP_ROSE_BUILTIN_DECLARATIONS");
     CustomMemoryPoolDOTGeneration::s_Filter_Flags* filter_flags = new CustomMemoryPoolDOTGeneration::s_Filter_Flags(argList);
     generateWholeGraphOfAST(filenameWithoutPath+"_WholeAST", filter_flags);
#endif

     if (aterm_traversal)  delete aterm_traversal;

#if DEBUG_EXPERIMENTAL_JOVIAL
     std::cout << "\nSUCCESSFULLY completed untyped node conversions (returning to caller)" << "\n\n";
#endif

     return 0;
   }
