OpenFEC.org project


1- HOW-TO COMPILE?
------------------

In this folder, you can find all the necesary files to compile the library and
the various applications. In order to compile:

- Create a "build" directory, if not already present
  (NB: you can change the directory name, "build" is not mandatory).

- Enter this directory, then call cmake (i.e. the build tool we're using) with
  the appropriate option:
  . to generate the Release version (i.e. maximum speed, no debug code):
	bash$ cmake .. -DDEBUG:STRING=OFF
  . to generate the Debug version:
	bash$ cmake .. -DDEBUG:STRING=ON

- Then, run make command to compile library from the "build" directory.
  The target is created in the "../bin/{Debug|Release}/" folder.


Comments:
---------

- There is no Makefile by default in the distribution. This is normal, as this
  project relies on the cmake tool, available on all current platforms, to produce
  all the required makefiles. For more information on cmake, please see:
	http://www.cmake.org/

- The advantage of creating a specific "build" directory and running cmake there
  (see step below) is to keep all the automatically created files (including
  makefiles) as sub-directories of "build", instead of having them throughout
  the source directories. However this is not mandatory, so feel free to ignore
  this.

- Add the DevIL library if you want to have the possibility to produce H/G matrix
  images with LDPC codes. See http://openil.sourceforge.net/
  Please install it on your machine before compiling the OpenFEC library if needed.
  Otherwise please edit and remove the IL library from the src/CMakeLists.txt file
  and from any src/lib_stable/*/of_codec_profile.h file.


2- HOW-TO CHECK THE BUILD BEFORE USING IT?
------------------------------------------

From the build directory, type:
	make test
A long list of unit tests is automatically launched.

NB: Some tests use the applis/eperftool performance evaluation tool, with different
parameter values, others are written explicitely in tests/*.c. Tests are all described
in tests/CMakeLists.txt, using cmake facilities.


3- HOW-TO MAKE EXTENSIVE PERFORMANCE EVALUATIONS?
-------------------------------------------------

Several tools have been designed to carry out extensive performance evaluations,
using an SQL database to store individual results. Some of the tools enable to launch
performance tests, the others to analyse the results and produce several kinds of
curves.

Basically:
- Enter the "perf_eval" directory.
- Edit the params.txt ASCII file, that describes the tests to carry out.
- Launch the tests with the following PERL file:
	./run_tests.pl params.txt
  (you may give any name you want to the params.txt file, to better reflect the
   nature of the tests being run)
- Analyze the results and generates the associated curves with:
	generate_curves -curve=<curve_id>

For more information on performance evaluation, please have a look at:
	doc/performance_tool.pdf

NB: there are some requirements in terms of PERL modules available, in particular
because of the use of mySQL or SQLite.

