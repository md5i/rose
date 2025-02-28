# HOW this testing works:
# Programs:
#
# testCG: Tests src/midend/programAnalysis/CallGraphAnalysis/CallGraph.C 
#         Generates a completely text and name based CallGraph.  Used by test01, test03
#
# testCallGraph: Tests src/midend/programAnalysis/CallGraphAnalysis/CallGraph.C 
#                Generates 3 graphs, AST, CallGraph, and colored Call Graph.  
#                Does not appear to be used by ANY tests
#
# testNewCallGraph: Tests src/midend/programAnalysis/CallGraphAnalysis/newCallGraph.C
#                   newCallGraph appears to be an attempt by Dan to make a global call
#                   graph analysis that doesn't rely on merge or sqlite, but he seems to 
#                   have forgotten about it.  So Maybe we should delete it?
#                   Has to be explicitly with make testNewCG_1 testNewCG_2 testNewCG_3
#                   But only tests that it doesn't crash, and doesn't check any output
#                   Output is textCallgraph and binaryCallGraph, which just seem to be 
#                   side effects of NewCallGraph
#
# virtualFctsTester: NOT USED
#                    virtualFctsTester generates test files with virtual function calls
#                    to see is the Call Graph handles virtual calls correctly.
#                    example: virtualFctsTester  4 5 0.5 `pwd`/foo
#
# stressTestFactory: Precompiled in the source directory tests/nonsmoke/functional/roseTests/programAnalysisTests/testCallGraphAnalysis
#                    It generates a test input file with X functions each of which 
#                    calls all X functions, so, The call graph should be a complete graph,
# Directories:
#
# test01: A single test with multiple files from the test01-specimens directory.  
#         Does not check output, only checks that it doesn't crash!
#         Uses testCG and produces Callgraph.cg.dmp and test01.passed
#
# test02: Uses stressTestFactory to produce 2 complete graphs, sT10 and sT100.
#         Does not check output, only checks that it doesn't crash!
#         Uses testCG and produces sT10.C, sT10.cmp.dmp, sT10.cg.dmp, and test02-10.passed
#         Uses testCG and produces sT100.C, sT100.cmp.dmp, sT100.cg.dmp, and test02-100.passed
#
# test03: Uses testCG on a set of specimens in test03-specimens, diffs results against 
#         test03-answers
#
# test04: DOES NOT CURRENTLY RUN on EDG 5.0, Leek updated it for EDG 6.0 and gcc > 5.
#         Test the specimens in the CompileTests/Cxx_tests for which we have answers in the 
#         test04-answers 
#
# -Jim Leek (2022/03/10)

include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
MOSTLYCLEANFILES =
EXTRA_DIST =
TEST_TARGETS =
noinst_PROGRAMS =

#########################################################################################################################
## Programs
#########################################################################################################################

noinst_PROGRAMS += testCG
testCG_SOURCES = CG.C
testCG_CPPFLAGS = $(ROSE_INCLUDES)
testCG_LDFLAGS = $(ROSE_RPATHS)
testCG_LDADD = $(ROSE_SEPARATE_LIBS)

# This is compiled, but never used
noinst_PROGRAMS += testCallGraph
testCallGraph_SOURCES = testCallGraph.C
testCallGraph_CPPFLAGS = $(ROSE_INCLUDES)
testCallGraph_LDFLAGS = $(ROSE_RPATHS)
testCallGraph_LDADD = $(ROSE_SEPARATE_LIBS)

# This is compiled, but never used
noinst_PROGRAMS += classHierarchyTestGen
classHierarchyTestGen_SOURCES = ClassHierarchyTestGen.cpp
classHierarchyTestGen_CPPFLAGS = $(ROSE_INCLUDES)
classHierarchyTestGen_LDFLAGS = $(ROSE_RPATHS)
classHierarchyTestGen_LDADD = $(ROSE_SEPARATE_LIBS)

# This is compiled, but never used
noinst_PROGRAMS += virtualFctsTester
virtualFctsTester_SOURCES = VirtualFctsTester.cpp
virtualFctsTester_CPPFLAGS = $(ROSE_INCLUDES)
virtualFctsTester_LDFLAGS = $(ROSE_RPATHS)
virtualFctsTester_LDADD = $(ROSE_LIBS)

# This is compiled, but never used
noinst_PROGRAMS += testNewCallGraph
testNewCallGraph_SOURCES = testNewCallGraph.C
testNewCallGraph_CPPFLAGS = $(ROSE_INCLUDES)
testNewCallGraph_LDFLAGS = $(ROSE_RPATHS)
testNewCallGraph_LDADD = $(ROSE_SEPARATE_LIBS)

#########################################################################################################################
## Tests
#########################################################################################################################

Test01SpecimenDir = $(srcdir)/test01-specimens
Test01SpecimenSource = $(addprefix $(Test01SpecimenDir)/, main.C f1.C f2.C)
Test01SpecimenHeaders = $(Test01SpecimenDir)/functions.h

# DQ (1/19/2017): These is a problem for GNU before 5.1
if ROSE_USING_GCC_VERSION_LATER_5_1
TEST_TARGETS += test01.passed
endif

test01 test01.passed: testCG $(Test01SpecimenSource) $(Test01SpecimenHeaders)
	@$(RTH_RUN) \
	    CMD="./testCG --edg:no_warnings -I. -I$(Test01SpecimenDir) $(Test01SpecimenSource) -o Callgraph" \
	    $(top_srcdir)/scripts/test_exit_status $@

EXTRA_DIST += $(Test01SpecimenDir) # test01.conf
MOSTLYCLEANFILES += Callgraph.cg.dmp Callgraph.dot main.ti

#------------------------------------------------------------------------------------------------------------------------
# Test that the call graph generates all the edges it should

EXTRA_DIST += test02.conf stressTestFactory

TEST_TARGETS += test02-10.passed
MOSTLYCLEANFILES += sT10.db sT10.cg.dmp sT10.cmp.dmp sT10.dot sT10.C
test02-10.passed: testCG stressTestFactory test02.conf
	@$(RTH_RUN) N=10 $(srcdir)/test02.conf $@

TEST_TARGETS += test02-100.passed
MOSTLYCLEANFILES += sT100.db sT100.cg.dmp sT100.cmp.dmp sT100.dot sT100.C
test02-100.passed: testCG stressTestFactory test02.conf
	@$(RTH_RUN) N=100 $(srcdir)/test02.conf $@

test02: test02-10.passed test02-100.passed

#------------------------------------------------------------------------------------------------------------------------
# Test a short list of local specimens from our source directory

Test03SpecimenDir = $(srcdir)/test03-specimens
Test03AnswerDir = $(srcdir)/test03-answers
Test03Specimens = test01.C test02.C  test03.C  test04.C  test05.C test07.C test06.C test08.C test09.C test10.C
Test03Targets = $(addprefix t3_, $(addsuffix .passed, $(Test03Specimens)))
TEST_TARGETS += $(Test03Targets)

test03: $(Test03Targets)
$(Test03Targets): t3_%.passed: $(Test03SpecimenDir)/% $(Test03AnswerDir)/%.cg.dmp testCG test03.conf
	@$(RTH_RUN) INPUT=$(notdir $<) OUTPUT=$$(basename $< .C).o ANSWERS=$(Test03AnswerDir) $(srcdir)/test03.conf $@

EXTRA_DIST += test03.conf $(Test03SpecimenDir) $(Test03AnswerDir)
MOSTLYCLEANFILES += $(patsubst %.C, %.o.cg.dmp, $(Test03Specimens))

#------------------------------------------------------------------------------------------------------------------------
# Test the specimens in the CompileTests/Cxx_tests for which we have answers in the $(Test04AnswersDir) directory.
Test04SpecimenDir = $(top_srcdir)/tests/nonsmoke/functional/CompileTests/Cxx_tests
Test04AnswerDir = $(srcdir)/test04-answers
Test04Specimens = $(notdir $(patsubst %.cg.dmp, %, $(wildcard $(Test04AnswerDir)/*.C.cg.dmp)))
Test04Targets = $(addprefix t4_, $(addsuffix .passed, $(Test04Specimens)))

#EDG 5.0 has problems with some of the Cxx compile tests, so we have to skip that.
#And I updated the tests to reflect changes after gcc 4.9.  -Jim Leek (2022/03/10)
if !ROSE_USE_EDG_VERSION_5_0  
if ROSE_USING_GCC_VERSION_LATER_4_9
TEST_TARGETS += $(Test04Targets)
endif
endif

test04: $(Test04Targets)
$(Test04Targets): t4_%.passed: $(Test04SpecimenDir)/% $(Test04AnswerDir)/%.cg.dmp testCG test04.conf
	@$(RTH_RUN) INPUT=$(notdir $<) OUTPUT=$$(basename $< .C).o ANSWERS=$(Test04AnswerDir) $(srcdir)/test04.conf $@


testNewCG_1: testNewCallGraph $(srcdir)/newCallGraph_input_01.c
	./testNewCallGraph -c $(srcdir)/newCallGraph_input_01.c

testNewCG_2: testNewCallGraph $(srcdir)/newCallGraph_input_02.c
	./testNewCallGraph -c $(srcdir)/newCallGraph_input_02.c

testNewCG_3: testNewCallGraph $(srcdir)/newCallGraph_input_03.C
	./testNewCallGraph -c $(srcdir)/newCallGraph_input_03.C




EXTRA_DIST += test04.conf $(Test04AnswerDir)
MOSTLYCLEANFILES += $(patsubst %.C, %.o.cg.dmp, $(Test04Specimens))

#########################################################################################################################
## Stuff for automake
#########################################################################################################################

check-local: $(TEST_TARGETS)
	@echo "*****************************************************************************************************************************"
	@echo "****** ROSE/tests/nonsmoke/functional/roseTests/programAnalysisTests/testCallGraphAnalysis: make check rule complete (terminated normally) ******"
	@echo "*****************************************************************************************************************************"

clean-local:
	rm -f $(TEST_TARGETS)
	rm -f $(TEST_TARGETS:.passed=.failed)
