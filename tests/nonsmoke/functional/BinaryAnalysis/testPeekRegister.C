#include <rose.h>
#include <Rose/BinaryAnalysis/InstructionSemantics2/BaseSemantics/RegisterStateGeneric.h>
#include <Rose/BinaryAnalysis/InstructionSemantics2/SymbolicSemantics.h>
#include <sstream>

using namespace Rose;
using namespace Rose::BinaryAnalysis;
using namespace Rose::BinaryAnalysis::InstructionSemantics2;
using namespace Rose::BinaryAnalysis::InstructionSemantics2::BaseSemantics;

int
main() {
    

    const RegisterDictionary *regdict = RegisterDictionary::dictionary_amd64();
    RiscOperatorsPtr ops = SymbolicSemantics::RiscOperators::instance(regdict);
    ASSERT_always_not_null(ops);
    RegisterStateGenericPtr registers = RegisterStateGeneric::promote(ops->currentState()->registerState());

    // Store some things in the register state
    const RegisterDescriptor RIP = regdict->findOrThrow("rip");
    RegisterDescriptor bit1 = RegisterDescriptor(RIP.majorNumber(), RIP.minorNumber(), 1, 1);
    RegisterDescriptor bit3 = RegisterDescriptor(RIP.majorNumber(), RIP.minorNumber(), 3, 1);
    registers->writeRegister(bit1, ops->boolean_(true), ops.get());
    registers->writeRegister(bit3, ops->boolean_(true), ops.get());
    std::ostringstream ss1;
    ss1 <<*ops;
    std::cout <<"state before peeking:\n" <<ss1.str();

    // Read the eight low-order bits
    RegisterDescriptor byte0 = RegisterDescriptor(RIP.majorNumber(), RIP.minorNumber(), 0, 8);
    SValuePtr zeroByte = ops->number_(8, 0);
    SValuePtr read = registers->peekRegister(byte0, zeroByte, ops.get());
    ASSERT_always_require(read->nBits() == 8);
    ASSERT_always_require(read->isConcrete());
    ASSERT_always_require2(read->toUnsigned().get() == 10, StringUtility::numberToString(read->toUnsigned().get()));

    // The state should be the same as before
    std::ostringstream ss2;
    ss2 <<*ops;
    ASSERT_always_require2(ss1.str() == ss2.str(), ss2.str());
}
