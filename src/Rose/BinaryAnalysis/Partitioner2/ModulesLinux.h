#ifndef ROSE_BinaryAnalysis_Partitioner2_ModulesLinux_H
#define ROSE_BinaryAnalysis_Partitioner2_ModulesLinux_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/Partitioner2/Modules.h>

#include <Rose/BinaryAnalysis/SystemCall.h>
#include <Rose/BinaryAnalysis/InstructionSemantics2/BaseSemantics/Dispatcher.h>

namespace Rose {
namespace BinaryAnalysis {
namespace Partitioner2 {

/** Disassembly and partitioning utilities for Linux. */
namespace ModulesLinux {

/** Create a system call analyzer suitable for this architecture. */
SystemCall systemCallAnalyzer(const Partitioner&, const boost::filesystem::path &syscallHeader = "");

/** Basic block callback to detect system calls that don't return.
 *
 *  Examines the instructions of a basic block to determine if they end with a system call that doesn't return, such as Linux's
 *  "exit". */
class SyscallSuccessors: public BasicBlockCallback {
    SystemCall analyzer_;
protected:
    SyscallSuccessors(const Partitioner&, const boost::filesystem::path &syscallHeader);
public:
    /** Allocating constructor.
     *
     *  An optional Linux system call header file can be provided to override the default. */
    static Ptr instance(const Partitioner&, const boost::filesystem::path &syscallHeader = "");

    virtual bool operator()(bool chain, const Args&) override;
};

/** Basic block callback to add "main" address as a function.
 *
 *  If the last instruction of a basic block is a call to __libc_start_main in a shared library, then one of its arguments is
 *  the address of the C "main" function. */
class LibcStartMain: public BasicBlockCallback {
    Sawyer::Optional<rose_addr_t> mainVa_;
public:
    /** Shared ownership pointer to LibcStartMain callback. */
    typedef Sawyer::SharedPointer<LibcStartMain> Ptr;

    static Ptr instance() { return Ptr(new LibcStartMain); } /**< Allocating constructor. */
    virtual bool operator()(bool chain, const Args&) override;

    /** Give the name "main" to the main function if it has no name yet. */
    void nameMainFunction(const Partitioner&) const;

private:
    // Read a value from the stack
    InstructionSemantics2::BaseSemantics::SValuePtr
    readStack(const Partitioner &partitioner, const InstructionSemantics2::BaseSemantics::DispatcherPtr &cpu, int byteOffset,
              size_t nBitsToRead, RegisterDescriptor segmentRegister);
};

/** Adds comments to system call instructions.
 *
 *  Adds a comment to each system call instruction for which the actual system call can be identified. A Linux header file can
 *  be provided to override the default location. */
void nameSystemCalls(const Partitioner&, const boost::filesystem::path &syscallHeader = "");

} // namespace
} // namespace
} // namespace
} // namespace

#endif
#endif
