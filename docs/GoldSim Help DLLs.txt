Appendix C of GoldSim Help

GoldSim allows you to develop separate program modules (written in C, C++, Pascal, FORTRAN or other
compatible programming languages) which can then be directly coupled with the main GoldSim algorithms. These
user-defined modules are referred to as external functions, and are linked into GoldSim as DLLs (Dynamic Link
Libraries) at run time. GoldSim interfaces with the DLL via an External element.
Read More: External (DLL) Elements on page 995
Integrating your external program module into GoldSim requires that you develop a "wrapper" or "shell" around
the function and compile it into a DLL. This appendix discusses the details of how external functions must be
coded and compiled.
In this Appendix
This appendix discusses the following:
l Understanding External (DLL) Elements
l Implementing an External Function
l External Function Examples
l External Function Calling Sequence
l DLL Calling Details

Understanding External (DLL) Elements
with External elements to do calculations or other manipulations that are not included in the standard capabilities of
GoldSim. The external function facility allows special purpose calculations or manipulations to be accomplished
with more flexibility, speed or complexity than with the standard GoldSim element types.
The external functions are bound to the GoldSim executable code at run time using DLL technology. The DLL
files should be present in the same folder as the GoldSim .gsm file, in the same folder as the GoldSim executable
file, or elsewhere in the user’s path.
Note that these functions are external to GoldSim and are not covered by the standard GoldSim verification
process. The user is responsible for any necessary testing of external functions.
Every external function is called by GoldSim with specific requests. The requests include initialization, returning
the function version number, performing a normal calculation, and "cleaning up" after a simulation. The function
name and argument list (the set of input and output data for the function) are specified by the GoldSim user when
setting up the External element.
External functions should provide their own error handling, message handling, file management and memory
management if required. It is essential that when it receives a “clean up” request, an external function should
release any dynamically acquired memory and close any open files.
Note: In the case of an error condition, the external function should always return an error code to GoldSim, so
that the user can be informed about the problem and the simulation can be terminated cleanly with no memory
leaks.
Implementing an External Function
Important Restrictions
The implementation of the external function is left to the programmer, but several restrictions apply so that the
functions can be correctly called by GoldSim. They are:
l The function return value is ignored. For example, use void functions in C/C++, or subroutines in
FORTRAN.
l Data are passed from GoldSim to the external function and back again to GoldSim via arrays of double
precision floating point input and output arguments.
l Input arguments must not be modified by the external function.  Doing so may cause memory corruption in
the DLL and/or GoldSim.
l Each function must manage its own initialization and memory allocation, and its own messages to the user
(if any).
l Unique external function (or subroutine) names must be defined in each DLL.  In C++, function names are
case-sensitive, while in FORTRAN, the case of the external subroutine name is determined from the DLL
build options.  In all instances, the function name specified in GoldSim is case-sensitive, and must agree
with the case specified in the DLL.

All files required to run your specific DLL must be properly installed on the machine where GoldSim is
running.  This includes any additional runtime libraries required by your DLL.
l Most development environments allow both static and dynamic binding of runtime libraries to DLLs.  DLLs
built with static binding are stand-alone, with all run-time library references included in the DLL file.  
However, if a DLL is built with dynamic binding, the runtime libraries are not included.  For a DLL with
dynamic binding, the runtime libraries (e. g. msvcrt.dll for Visual C++ or libifcoremd.dll for Intel Visual
Fortran) must be present and in the environment PATH variable on the machine where GoldSim is running.
External Function Format
When calling methods from the DLL, GoldSim always expects the following C/C++ function signature:
extern "C" void __declspec(dllexport)
MyExternalFcn(int XFMethod,
int* XFState,
double* inargs,
double* outargs)
The extern "C" specifies C language style linkage between GoldSim and the DLL, and __declspec(dllexport) makes
the function visible outside the DLL.
For Intel Visual Fortran, the following subroutine signature can be used:
subroutine my_external_fcn(xfmethod, xfstate, inargs, outargs)
!DEC$ ATTRIBUTES dllexport,c :: add_mult_scalars
!DEC$ ATTRIBUTES value :: nmethod
!DEC$ ATTRIBUTES reference :: nstatus
!DEC$ ATTRIBUTES reference :: dinputs
!DEC$ ATTRIBUTES reference :: doutputs
Other FORTRAN development environments may require different attributes for the proper linkage to GoldSim.
Note:  Arrays in C/C++ start at zero, rather than one (the default for FORTRAN).  Array indices in this section
use the C/C++ convention.
The arguments are:
Argument Definition
int  
XFmethod;
Action that the external function must perform  (see table below)
int*  XFState; Returned value success 0 or fatal 1
double*
inargs;
Array of input arguments
double*
outargs;
This array returns different information when different XFmethod values are passed to the
external function
The values for XFmethod are:

0 XF_INITIALIZE Initialize (called at the beginning of each realization). No arguments are
passed on this call.
1 XF_
CALCULATION
Normal calculation. Total number of output arguments are returned on this
call. outargs[0] = 1st result from the function;  outargs[1] = 2nd result from
the function; etc.
2 XF_REP_
VERSION
External functions report their versions. No input arguments are passed on
this call. outargs[0] = external function version, e.g., 1.10
3 XF_REP_
ARGUMENTS
External functions report the # of input and output arguments.  No input
arguments are passed on this call. outargs[0] = # of input arguments.  outargs
[1] = # of output arguments.
99 XF_CLEANUP Close any open files, and optionally release dynamic memory at the end of a
simulation. No arguments are passed on this call.
The returned values for XFState are:
0 OK, continue GoldSim
> 0 and < 99 terminate GoldSim
99 OK, unload the DLL
The following two return codes may only be used for an XF_CALCULATION call:
-1 Fatal error, error message pointer returned
-2 More result memory required; the total amount (in doubles) required is returned in outargs[0]. This
may be required of the external function is returning a table or time series definition. (Note that the
additional memory is retained until the end of the simulation.)
The memory for the inargs and outargs arrays is allocated by GoldSim at the start of the simulation, based upon the
inputs and outputs specified in the Interface tab of the External element properties dialog.  The number of input and
output arguments is verified during the XF_REP_ARGUMENTS request.  GoldSim counts up the number of input
and output arguments it expects, and compares it to the numbers for each reported by the DLL.
Warning:  It is the responsibility of the external function to ensure that it does not write to any output
arguments beyond the number reported for XF_REP_ARGUMENTS.  Doing so may cause memory corruption
in GoldSim.
Argument Checking
GoldSim calculates the total number of input and output arguments by summing over the the inputs and outputs
specified on the Interface tab of the External element properties dialog.  Note that each scalar input or output counts
as one argument, while array inputs or outputs count as the size of the array (rows * columns). The calculated totals
are then compared to the external function’s reported number of input and output arguments.  If the number of
arguments defined by GoldSim does not agree with the number reported by the external function, GoldSim
terminates with an error message.

However, note the following exceptions:
l If outargs[0] is returned as -1, the number of input arguments is not tested. This allows for functions where
the DLL does not know in advance the number of input argument that it will receive.
l If the external function will be returning definitions for one or more Tables or Time Series (see below),
GoldSim will not know in advance how long the Table definitions will be. In this case, the external function
should specify a value for outargs[1] greater than or equal to the actual total number of arguments that may
be returned. GoldSim will allocate this amount of memory for outargs. Note that this can be reset during the
simulation by returning XFState=-2.
The Input and Output Argument Arrays
The content of input and output argument arrays is determined from the Interface tab of the External element
properties dialog.
The following points should be noted:
l Input or outputs are processed in the order specified in the interface.  All data from one input or output is
contiguous, followed by the data from the next input or output.
l Scalar inputs and outputs are mapped to a single array argument.
l Vector input and output items are specified in order, one argument per item, according to the order specified
in the array label.
l Matrix input and output items are specified item-by-item, with all items in one row together, followed by all
items in the next row, and so on.
l Lookup Table definitions can be specified in the output interface, via a special format.
l Time Series definitions can be specified as inputs or outputs, also via a special format.
Lookup Table Definitions
External functions can also return Table definition elements to GoldSim. A table definition requires a specific
sequence of values, depending whether it is a 1-D, 2-D, or 3-D table.
The sequence for a 1-D table is as follows:
l number of dimensions (in this case, 1)
l the number of rows
l row value1, row value 2, …, row value n
l dependent value 1, dependent value 2, …, dependent value n
The sequence for a 2-D table is as follows:
l number of dimensions (in this case, 2)
l the number of rows, the number of columns
l row value1, row value 2, …, row value n
l column value 1, column value 2, …, column value n
l dependent(row 1, column 1), …, dependent(row 1,column n)
l dependent(row 2, column 1), …, dependent(row 2,column n)

l …
l dependent(row n, column 1), …, dependent(row n, column n)
Warning:  This is different than the sequence used to read in an ASCII text file for a 2-D table.
The sequence for a 3-D table is as follows:
l number of dimensions (must be 3)
l the number of rows, the number of columns, the number of layers
l row value1, row value 2, …, row value y
l column value 1, column value 2, …, column value x
l layer value1, layer value 2, …, layer value z
l dependent(row 1, column 1, layer 1), …, dependent(row 1,column x, layer 1)
l dependent(row 2, column 1, layer 1), …, dependent(row 2,column x, layer 1)
l …
l dependent(row y, column 1, layer 1), …, dependent(row y, column x, layer 1)
l .
.
.
l dependent(row 1, column 1, layer z), …, dependent(row 1,column x, layer z)
l dependent(row 2, column 1, layer z), …, dependent(row 2,column x, layer z)
l …
l dependent(row y, column 1, layer 1), …, dependent(row y, column x, layer z)
Warning:  This is different than the sequence used to read in an ASCII text file for a 3-D table.
Time Series Definitions
External functions can also read and return Time Series Definition.  A Time Series Definition consists of the
following specific sequence of values.
1. The number 20 (this informs GoldSim that this is a Time Series)
2. The number -3 (this is a format number that infoms GoldSim what version of the time series format is
expected)
3. Calendar-baed index: 0 if elapsed time; 1 if dates
4. An index (0,1,2,3) indicating what the data represents (0=instantaneous value, 1=constant value over the
next time interval, 2=change over the next time interval, 3=discrete change)
5. The number of rows (0 for scalar time series)
6. The number of columns (0 for scalar and vector time series)
7. Number of series

For each series, the following is repeated:
l The total number of time points in the series
l Time point 1, Time point 2, …, Time point n
The structure of the remainder of the file depends on whether the Time Series Definition represents a scalar,
a vector, or a matrix.
For a scalar, the next sequence of values is as follows:
l Value 1[time point 1], Value 2[time point 2], …, Value[time point n]
For a vector, the next sequence of values is as follows:
l Value[row1, time point 1], Value[row1, time point 2], …, Value[row1, time point n]
l Value[row2, time point 1], Value[row2, time point 2], …, Value[row2, time point n]
l …
l Value[rowr, time point 1], Value[rowr, time point 2], …, Value[rowr, time point n]
For a matrix, the next sequence of values is as follows:
l Value[row1, column1, time point 1], Value[row1, column1, time point 2], …, Value[row1, column1,
time point n]
l Value[row1, column2, time point 1], Value[row1, column2, time point 2], …, Value[row1, column2,
time point n]
l …
l Value[row1, columnc, time point 1], Value[row1, columnc, time point 2], …, Value[row1, columnc,
time point n]
l .
.
.
l Value[rowr, column1, time point 1], Value[rowr, column1, time point 2], …, Value[rowr, column1,
time point n]
l Value[rowr, column2, time point 1], Value[rowr, column2, time point 2], …, Value[rowr, column2,
time point n]
l …
l Value[rowr, columnc, time point 1], Value[rowr, columnc, time point 2], …, Value[rowr, columnc,
time point n]
External Function Examples
The following is a simple example implementation of DLL code that will work with an External element.  This
code takes two scalar elements as input, and returns the sum and product to GoldSim.  For simplicity, this code is
written in C, implemented with Visual C++.  This external function can be used with the External.gsm example
which can be found in the External subfolder of the General Examples folder in your GoldSim directory (accessed
by selecting File | Open Example... from the main menu).

// Global enumerations, useful for C-style implementations
//
// XFMethodID identifies the method types, used to identify
// the phase of the simulation that is currently in progress.
//
// XF_INITIALIZE - Called after DLL is loaded and before each realization.
// XF_CALCULATE - Called during the simulation, each time the inputs change.
// XF_REP_VERSION - Called after DLL load to report the external fcn version number.
// XF_REP_ARGUMENTS - Called after DLL load to report the number of input
// and output arguments.
// XF_CLEANUP - Called before the DLL is unloaded.
//
enum XFMethodID
{
XF_INITIALIZE = 0, XF_CALCULATE = 1, XF_REP_VERSION = 2, XF_REP_ARGUMENTS = 3,
XF_CLEANUP = 99
};
// XFStatusID identifies the return codes for external functions.
//
// XF_SUCCESS – Call completed successfully.
// XF_CLEANUP_NOW - Call was successful, but GoldSim should clean up
// and unload the DLL immediately.
// XF_FAILURE - Failure (no error information returned).
// XF_FAILURE_WITH_MSG – Failure, with DLL-supplied error message available.
// Address of error message is returned in the first element
// of the output arguments array.
// XF_INCREASE_MEMORY - Failed because the memory allocated for output arguments
// is too small. GoldSim will increase the size of the
// output argument array and try again.
//
enum XFStatusID
{
XF_SUCCESS = 0, XF_FAILURE = 1, XF_CLEANUP_NOW = 99, XF_FAILURE_WITH_MSG = -1,
XF_INCREASE_MEMORY = -2
};
///////////////////////////////////////////////////////////////////////////////
// AddMultScalarsInC
// Adds and multiplies two input scalar values (C Language implementation).
//-----------------------------------------------------------------------------
extern "C" void __declspec(dllexport) AddMultScalarsInC(int methodID,
int* status,
double* inargs,
double* outargs)
{
*status = XF_SUCCESS;
switch ( methodID )
{
case XF_INITIALIZE:
break; // nothing required
case XF_REP_VERSION:
outargs[0] = 1.03;
break;
case XF_REP_ARGUMENTS:
outargs[0] = 2.0; // 2 scalar inputs expected
outargs[1] = 2.0; // 2 scalar outputs returned
break;
case XF_CALCULATE:
outargs[0] = inargs[0] + inargs[1]; // return the sum
outargs[1] = inargs[0]*inargs[1]; // return the product
break;

case XF_CLEANUP:
break; // No clean-up required
default:
*status = XF_FAILURE;
break;
}
}
The following code is the same algorithm, implemented in Intel Visual Fortran.
! Utility module to specify the GoldSim parameter constants
module gs_parameters
implicit none
! Parameters to identify the method types, which indicate the phase of the
! simulation that is currently in progress.
!
! INITIALIZE - Called after DLL is loaded and before each realization.
! CALCULATE - Called during the simulation, each time the inputs change.
! REPORT_VERSION - Called after DLL load to report the external fcn version number.
! REPORT_ARGUMENTS - Called after DLL load to report the number of input and output
! arguments.
! CLEANUP - Called before the DLL is unloaded.
integer(4), parameter :: INITIALIZE = 0
integer(4), parameter :: CALCULATE = 1
integer(4), parameter :: REPORT_VERSION = 2
integer(4), parameter :: REPORT_ARGUMENTS = 3
integer(4), parameter :: CLEANUP = 99
! Parameters to identify the return codes for external functions.
!
! SUCCESS – Call completed successfully.
! CLEANUP_NOW - Call was successful, but GoldSim should clean up
! and unload the DLL immediately.
! FAILURE - Failure (no error information returned).
! FAILURE_WITH_MSG – Failure, with DLL-supplied error message available.
! Address of error message is returned in the first element
! of the output arguments array.
! INCREASE_MEMORY - Failed because the memory allocated for output arguments
! is too small. GoldSim will increase the size of the output
! argument array and try again.
integer(4), parameter :: SUCCESS = 0
integer(4), parameter :: FAILURE = 1
integer(4), parameter :: CLEANUP_NOW = 99
integer(4), parameter :: FAILURE_WITH_MSG = -1
integer(4), parameter :: INCREASE_MEMORY = -2
end module gs_parameters
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
! add_mult_scalars
! Adds and multiplies two input scalar values.
!----------------------------------------------------------------------
subroutine add_mult_scalars(method_id, status, inargs, outargs)
!DEC$ ATTRIBUTES dllexport,c :: add_mult_scalars
!DEC$ ATTRIBUTES value :: method_id
!DEC$ ATTRIBUTES reference :: status
!DEC$ ATTRIBUTES reference :: inargs
!DEC$ ATTRIBUTES reference :: outargs
use gs_parameters
implicit none
real(8), parameter :: VERSION = 1.03
integer(4), parameter :: NINPUTS = 2 ! Two scalar inputs expected
integer(4), parameter :: NOUTPUTS = 2 ! Two scalar outputs returned
integer(4) method_id, status
real(8) inargs(*), outargs(*)
select case (method_id)

case (INITIALIZE)
status = SUCCESS
case (REPORT_VERSION)
outargs(1) = VERSION
status = SUCCESS
case (REPORT_ARGUMENTS)
outargs(1) = NINPUTS
outargs(2) = NOUTPUTS
status = SUCCESS
case (CALCULATE)
outargs(1) = inargs(1) + inargs(2) ! return the sum
outargs(2) = inargs(1)*inargs(2) ! return the product
status = SUCCESS
case (CLEANUP)
status = SUCCESS
case default
status = FAILURE
end select
end subroutine add_mult_scalars
Additional DLL code examples can be found (including examples for arrays, Lookup Tables, and Time Series
Elements) can be found in the GoldSim install directory.  In addition to the source files, example solution and
project files for Microsoft Visual C++ and Intel Visual Fortran are also included (under General
Examples/External).

External Function Calling Sequence
GoldSim makes calls to the DLL external function at different times during the course of a GoldSim simulation:
l Before the simulation starts (while checking model integrity).
l The first time that the External element values are calculated.
l Every time that the input values of the External element change.
l Before each (subsequent) realization.
l After each realization (Cleanup After Realization option only).
l After the simulation finishes.
l If any DLL external function call returns a value of 99.
GoldSim users can control when the DLL is unloaded via the Unload After Each Use and Cleanup After
Realization options.  These options are selected via checkboxes in the External element properties dialog ("Unload
DLL after each use" and "Run Cleanup after each realization").  As the name implies, Unload After Each Use will
clean up (XFMethod = XF_CLEANUP) and unload the DLL after each calculation call (XFMethod =
XFCalculate).  Similarly, Cleanup After Realization will clean up and unload the DLL at the end of each realization
(if the DLL is loaded).
The DLL external function code can also control when the DLL is unloaded.  If any call to a DLL external function
returns a status of 99, GoldSim will treat the call as a success, but will clean up and unload the DLL immediately
after processing the returned data.
Before the Simulation
GoldSim calls the DLL external function before the simulation starts, as part of the process to check the validity of
the model.  The main reason is to get the number of input and output arguments, to insure that they are consistent
with what is specified in the Interface tab. The call sequence is as follows:

1. Load the DLL and check for the existence of the external function
2. Ask for the version number (XFMethod = XF_REP_VERSION).
3. Ask for the number of input and output arguments (XFMethod = XF_REP_ARGUMENTS), and compare
to the values determined from the External element interface.
4. Clean up (XFMethod = XF_CLEANUP) and unload the DLL.
If any of these calls fail, or if the number of input or output arguments in step 3 are inconsistent, GoldSim will
display an error message and return to the previous state (Edit or Ready mode).
During Each Realization
During each realization, GoldSim will call the DLL external function when the corresponding External element
needs to be updated.  This happens the first time that the Element is referenced, and every subsequent time-step or
event update where an input to the Element changes.  In this case, the following call sequence is used:
1. Check to see if the DLL is loaded.  If so, skip to step 6.
2. Load the DLL and check for the existence of the external function
3. Ask for the version number (XFMethod = XF_REP_VERSION), and write it to the log file.
4. Ask for the number of input and output arguments (XFMethod = XF_REP_ARGUMENTS).
5. Initialize the DLL (XFMethod = XF_INITIALIZE).
6. Calculate (XFMethod = XF_CALCULATE)
7. If Unload After Each Use is specified, clean up (XFMethod = XF_CALCULATE) and unload the DLL.
Before Each Realization
Before each realization, GoldSim will check to see if the DLL is loaded for each External element.  If the DLL is
loaded, GoldSim will reinitialize the DLL (XFMethod = XF_INITIALIZE).
After Each Realization
If the Cleanup After Realization option is specified, and the DLL is loaded, GoldSim will clean up (XFMethod =
XF_CLEANUP) and unload the DLL after each realization.
After the Simulation
After the simulation completes (either successfully or after a fatal error), GoldSim will clean up (XFMethod = XF_
CLEANUP) and unload the DLL if it is still loaded.
DLL Calling Details
GoldSim can call DLL external functions by two different mechanisms, depending upon the Separate Process
Space option.  For each External element, the GoldSim user can select this option in the properties dialog, via the
"Run in separate process space" checkbox.  If this option is not selected, the DLL will be loaded with the Win32
LoadLibrary function.  As a result, this DLL will share the same memory address space as the GoldSim process,
and any memory used in the DLL will be charged to the GoldSim process.  By default, External elements are
created without the Separate Process Space option enabled.
If the Separate Process Space option is selected, GoldSim will call the DLL using a Component Object Model
(COM) interface.  The interface is implemented as a COM LocalServer executable, which is started when GoldSim
first requests to load the DLL.  Once the LocalServer is started, it loads the DLL into its own memory address space

(separate from GoldSim), and acts a proxy between GoldSim and the DLL for all external function requests.  After
the DLL is unloaded, GoldSim frees the COM interface and the LocalServer executable terminates.  Because this
option loads the DLL into a separate memory space, it may be a better option for DLLs with a large memory
footprint.
64-Bit DLL Support
GoldSim also supports loading of external DLLs that are built as 64-bit libraries.  The main advantage for a 64-bit
DLL is a significant increase in the amount of virtual memory available (from 4GB to 8TB).  Migrating DLL code
from 32-bit to 64-bit requires minimal (if any) changes; just install the 64-bit compilers for Visual C++ or Visual
Fortran and build with a 64-bit target.  No change is GoldSim model configuration is required to use 64-bit DLLs,
since GoldSim will automatically determine the type of DLL and call the appropriate interface.  However, the
following caveats do apply when using 64-bit DLLs in GoldSim:
l 64-bit DLLs can only be run on a computer with a 64-bit Windows operating system. If a DLL needs to run
on both 32-bit and 64-bit Windows, it should be a 32-bit DLL (which will run on both 64-bit and 32-bit
OS).
l 64-bit DLLs must run in a separate process space.
l For Distributed Processing runs, models that contain 64-bit DLLs can only be launched from a 64-bit
Windows operating system. GoldSim will inspect the model to see if it contains a 64-bit DLL, and will
disconnect all slaves that are running a 32-bit Windows OS.
Returning Error Messages from External Functions
To help GoldSim users debug problems with DLL external functions, GoldSim lets users send an error message
from the DLL back to GoldSim through the External element interface when the call to an external function fails.  
The error message is then displayed to the user in a pop-up dialog.
The DLL external function signals the presence of an error message by returning a status value of -2.  When
GoldSim processes the results of the DLL external function call, it will interpret the first element of the output
arguments arrray (outargs in our source-code example) as a pointer to a memory location where the error string can
be found.  The memory containing the string must have static scope, so that it will still be available when GoldSim
retrieves the string.  The string must also be NULL-terminated, even when returning from a FORTRAN DLL.  If
either of these recommendations are not followed, GoldSim will likely crash when it tries to display the error
message.
The following code is an example of a C language function that will properly handle passing a message from a
DLL external function to GoldSim.  The ULONG_PTR is cast to different types on 64-bit (unsigned long) and 32-
bit (unsigned __int64), so that it will work for building both 32-bit and 64-bit binaries.
// Utility method used to simplify the sending of an error message to GoldSim
void CopyMsgToOutputs(const char* sMsg, double* outargs)
{
// Static character array used to hold the error message.
// This needs to be static so that it will be "in scope" when it is read by GoldSim.
static char sBuffer[200];
// Clear out any old data from the buffer
memset(sBuffer, 0, sizeof(sBuffer));
// Cast the first output array element as a pointer.
// ULONG_PTR is used because it will work for both 32-bit and 64-bit DLLs
ULONG_PTR* pAddr = (ULONG_PTR*) outargs;
// Copy the string data supplied into the static buffer.
// Safer version of string copy is used. If your compiler does not support this you
// should remove the comments in front of the next line and add to the next one.
// strncpy(sBuffer, sMsg, sizeof(sBuffer) - 1);

strncpy_s(sBuffer, sMsg, sizeof(sBuffer) - 1);
// Copy the static buffer pointer to the first output array element.
*pAddr = (ULONG_PTR) sBuffer;
}
For FORTRAN DLLs, the following code performs the same function:
! Utility subroutine to simplify sending an error message to GoldSim
subroutine copy_msg_to_outputs(smsg, outargs)
implicit none
character(*) smsg
real(8) outargs(*)
! "Static" character buffer, used to store the error message so
! that it can be returned to GoldSim.
character(80), save :: sbuffer
! Create a shared memory variable that can be interpreted either as integer or real.
integer(8) ioutput1
real(8) doutput1
equivalence(ioutput1, doutput1)
! Copy the message into the buffer. Truncate it if it is too long
! Make sure that it is null terminated!
if (len(smsg) .lt. 80) then
sbuffer = smsg // char(0)
else
sbuffer = smsg(1:79) // char(0)
end if
! Since we are sending back to C++, we need an actual address.
! The "loc" function is not standard, but it is supported by all
! compilers that we checked.
ioutput1 = loc(sbuffer)
outargs(1) = doutput1
end subroutine copy_msg_to_outputs