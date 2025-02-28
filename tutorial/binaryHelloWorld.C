//! [rose include]
#include <rose.h>
//! [rose include]

//! [other includes]
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>
#include <AsmUnparser.h>
//! [other includes]

int
main(int argc, char *argv[]) {
    //! [commandline]
    ROSE_INITIALIZE;                                    // see Rose::initialize
    std::string purpose = "disassembles a binary specimen";
    std::string description =
        "This tool disassembles the specified file and presents the results "
        "as a pseudo assembly listing, a listing intended for human consumption "
        "rather than assembling.  This implementation serves as the \"Hello, "
        "World!\" example for binary analysis, so let's keep it simple!";
    //! [commandline]

    //! [disassembly]
    Rose::BinaryAnalysis::Partitioner2::Engine engine;
    SgAsmBlock *gblock = engine.frontend(argc, argv, purpose, description);
    //! [disassembly]

    //! [unparsing]
    Rose::BinaryAnalysis::AsmUnparser unparser;
    unparser.unparse(std::cout, gblock);
    //! [unparsing]
}
