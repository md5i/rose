#include <rose.h>                                       // must be first
#ifdef ROSE_ENABLE_LIBRARY_IDENTIFICATION

static const char *gPurpose = "insert into fast library identification and recognition database";
static const char *gDescription =
    "Inserts function information into a database for fast library identification and recognition (FLIR). "
    "If a ROSE Binary Analysis (*.rba) file is specified as the source, then that specimen's functions are hashed "
    "and added to the specified database. If a FLIR database URL is specified (see \"Databases\"), then function information "
    "is copied from the source database to the destination database. In either case, only functions that satisfy the "
    "function selection criteria are inserted.";

#include <Rose/BinaryAnalysis/LibraryIdentification.h>
#include <Rose/BinaryAnalysis/Partitioner2/Engine.h>
#include <Rose/CommandLine.h>
#include <Rose/Diagnostics.h>

#include <batSupport.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

using namespace Sawyer::Message::Common;
using namespace Rose;
using namespace Rose::BinaryAnalysis;
namespace P2 = Rose::BinaryAnalysis::Partitioner2;
using Flir = Rose::BinaryAnalysis::LibraryIdentification;

static Sawyer::Message::Facility mlog;

struct Settings {
    SerialIo::Format stateFormat = SerialIo::BINARY;    // format for the RBA file
    std::string connectUrl;                             // name of existing database
    std::string createUrl;                              // name of database to (re)create
    std::string libraryName;                            // name to use for library in the database
    std::string libraryVersion;                         // version string to give to the library in the database
    std::string libraryArchitecture;                    // string to use as the architecture in the database
    std::string libraryHash;                            // string to store as the library hash (ID) in the database
    Flir::Settings flir;                                // settings for fast library identification and recognition
};

static std::string
parseCommandLine(int argc, char *argv[], Settings &settings) {
    using namespace Sawyer::CommandLine;

    // Generic switches
    SwitchGroup generic = Rose::CommandLine::genericSwitches();
    generic.insert(Bat::stateFileFormatSwitch(settings.stateFormat));

    // Tool switches
    SwitchGroup tool("Tool specific switches");
    tool.name("tool");

    tool.insert(Switch("database", 'D')
                .argument("url", anyParser(settings.connectUrl))
                .doc("Name of an existing datatabase into which new library functions are added. See the \"Databases\" section for "
                     "information about how to specify database drivers and names."));
    tool.insert(Switch("create", 'C')
                .argument("url", anyParser(settings.createUrl))
                .doc("Name of a database to create, or an existing database whose tables will be recreated. See the \"Databases\" "
                     " section for information about how to specify database drivers and names."));

    tool.insert(Switch("library-name")
                .longName("lname")
                .argument("string", anyParser(settings.libraryName))
                .doc("Name to give to the library in the database. This is usually the base name of the shared library file "
                     "without the \".so\" or \".lib\" extension. The default is the empty string."));

    tool.insert(Switch("library-version")
                .longName("lvers")
                .argument("string", anyParser(settings.libraryVersion))
                .doc("String that will be used as the version for the library when added to the database. The default is the "
                     "empty string."));

    tool.insert(Switch("library-architecture")
                .longName("larch")
                .argument("string", anyParser(settings.libraryArchitecture))
                .doc("String that will be used as the architecture name for the library when added to the database. The default "
                     "is the empty string."));

    tool.insert(Switch("library-hash")
                .longName("lhash")
                .argument("string", anyParser(settings.libraryHash))
                .doc("Hash value identifying the library. The default is to hash the library's virtual memory with SHA-256, but "
                     "it is also common to specify a hash of the library file itself (often as an MD5). The only requirement for "
                     "the hash is that it uniquely identifies the library and that the same hash is used consistently for that "
                     "library."));

    // Parsing
    Parser parser = Rose::CommandLine::createEmptyParser(gPurpose, gDescription);
    parser.errorStream(mlog[FATAL]);
    parser.with(generic);
    parser.with(Flir::commandLineSwitches(settings.flir));
    parser.with(tool);
    parser.doc("Synopsis", "@prop{programName} [@v{switches}] [@v{BAT-input}|@v{source_datbase}]");
    parser.doc("Databases", Sawyer::Database::Connection::uriDocString());

    std::vector<std::string> input = parser.parse(argc, argv).apply().unreachedArgs();
    if (input.size() > 1) {
        mlog[FATAL] <<"incorrect usage; see --help\n";
        exit(1);
    }
    if (settings.connectUrl.empty() && settings.createUrl.empty()) {
        mlog[FATAL] <<"a database name must be specified with --database/-D or --create/-C\n";
        exit(1);
    }
    if (!settings.connectUrl.empty() && !settings.createUrl.empty()) {
        mlog[FATAL] <<"the @s{database} and @s{create} switches are mutually exclusive\n";
        exit(1);
    }
    return input.empty() ? "-" : input[0];
}

static std::string
hashLibrary(const MemoryMap::Ptr &map) {
    ASSERT_not_null(map);
    Combinatorics::HasherSha256Builtin hasher;
    map->hash(hasher);
    return hasher.toString();
}

static void
copyFromRba(const Settings &settings, Flir &dst, const std::string &rbaName) {
    P2::Engine engine;
    P2::Partitioner partitioner = engine.loadPartitioner(rbaName, settings.stateFormat);

    std::string libraryHash = settings.libraryHash;
    if (libraryHash.empty()) {
        libraryHash = hashLibrary(partitioner.memoryMap());
        mlog[INFO] <<"library hash is " <<libraryHash <<"\n";
    }

    std::string libraryArchitecture = settings.libraryArchitecture;
    if (libraryArchitecture.empty()) {
        libraryArchitecture = partitioner.instructionProvider().disassembler()->name();
        mlog[INFO] <<"library architecture is " <<libraryArchitecture <<"\n";
    }

    // Create the analysis object and connect to the database
    auto lib = Flir::Library::instance(libraryHash, settings.libraryName, settings.libraryVersion,
                                       libraryArchitecture);
    size_t nInserted = dst.insertLibrary(lib, partitioner);
    mlog[INFO] <<"inserted/updated " <<StringUtility::plural(nInserted, "functions") <<"\n";
}

static void
copyFromDatabase(const Settings &settings, Flir &dst, const std::string &dbUrl) {
    Flir src;
    src.settings(settings.flir);
    src.connect(dbUrl);

    size_t nInserted = 0;
    std::set<std::string> libraryHashes;
    std::vector<Flir::Function::Ptr> srcFunctions = src.functions();
    Sawyer::ProgressBar<size_t> progress(srcFunctions.size(), mlog[MARCH], "functions");
    for (const auto &function: srcFunctions) {
        ++progress;
        if (dst.insertFunction(function))
            ++nInserted;
    }
    mlog[INFO] <<"inserted/updated " <<StringUtility::plural(nInserted, "functions") <<"\n";
}

int
main(int argc, char *argv[]) {
    // Initialization
    ROSE_INITIALIZE;
    Diagnostics::initAndRegister(&mlog, "tool");
    mlog.comment("fast library identification and recognition");
    Bat::checkRoseVersionNumber(MINIMUM_ROSE_LIBRARY_VERSION, mlog[FATAL]);
    Bat::registerSelfTests();

    // Command-line parsing
    Settings settings;
    std::string inputName = parseCommandLine(argc, argv, settings);

    Flir flir;
    flir.settings(settings.flir);
    if (!settings.connectUrl.empty()) {
        flir.connect(settings.connectUrl);
    } else {
        flir.createDatabase(settings.createUrl);
    }

    // Input file is either an RBA file or another database. If it's a database, then the specified functions
    // and libraries are copied from the source database into the destination database.
    if (boost::ends_with(inputName, ".rba")) {
        copyFromRba(settings, flir, inputName);
    } else if (boost::contains(inputName, "://") || boost::ends_with(inputName, ".db")) {
        copyFromDatabase(settings, flir, inputName);
    } else {
        try {
            copyFromDatabase(settings, flir, inputName);
        } catch (...) {
            copyFromRba(settings, flir, inputName);
        }
    }
}

#else // LibraryIdentification is not enable...

#include <Rose/Diagnostics.h>
#include <cstring>

int main(int argc, char *argv[]) {
    ROSE_INITIALIZE;
    Sawyer::Message::Facility mlog;
    Rose::Diagnostics::initAndRegister(&mlog, "tool");

    int exitStatus = 1;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--no-error-if-disabled"))
            exitStatus = 0;
    }

    mlog[Rose::Diagnostics::FATAL]
        <<"fast library identification and recognition (FLIR) is not enabled in this ROSE configuration\n";

    return exitStatus;
}

#endif
