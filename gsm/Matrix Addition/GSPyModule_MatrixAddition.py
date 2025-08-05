# This file *must* have the name GSPyModule.py.
# 
# The dynamically-linked library (DLL) GSPy.pyd forms the interface between GoldSim
# and this custom Python module. GSPy.pyd imports GSPyModule.py and calls the
# following functions (which *must* be defined in this file):
#
#   - ReturnPyModuleVersion()
#   - NumberOfInputs()
#   - NumberOfOutputs()
#   - DoCalcsAndReturnValues(*inputsList)
#
# The following functions are optional:
#
#   - InitializeSimulation()
#   - FinalizeSimulation()
#
# GSPy.pyd also has a Python API that allows functions (defined in GSPy.pyd)
# to be called from this custom Python module. Include the following command
# to import the Python API:
#
# import GSPy
#
# To relay error messages to GoldSim, use the following function:
#
# GSPy.setErrorMessage(<error message as a string>)
#
# Call the above function within any of the methods defined below. For this error
# message to be displayed by GoldSim, you must return a value of 'None'.


# Import necessary modules
import collections
import numpy as np

####### ---------- DEFINE CONSTANTS ---------- #######

# Python module version
PYMODULEVERSION = 0.10

# The following constants provide the dimensions of the input matrices
NUMROWS = 2
NUMCOLS = 3

# IVARPROPS is short for input variable properties. This OrderedDict contains start
# indexes and sizes of all inputs expected from GoldSim/GSPy. The keys are the
# variable names. The values are tuples where the first item is the start index for
# the corresponding variable and the second item is the size of that variable. 
# Conveniently, the start index of any variable is the sum of the tuple for the
# immediately preceding variable (i.e. the start index of the previous variable plus
# the size of the previous variable).
IVARPROPS = collections.OrderedDict()
IVARPROPS['A'] = (0, NUMROWS * NUMCOLS)
IVARPROPS['B'] = (sum(IVARPROPS['A']), NUMROWS * NUMCOLS)

# Store the total number of individual values (this will be the size of the inputs list
# provided from GoldSim/GSPy)
TOTALIVARS = sum(list(IVARPROPS.items())[-1][1])

# OVARPROPS is short for output variable properties. This OrderedDict contains start
# indexes and sizes of all outputs to be returned to GoldSim/GSPy. The keys are the
# variable names. The values are tuples where the first item is the start index for
# the corresponding variable and the second item is the size of that variable.
OVARPROPS = collections.OrderedDict()
OVARPROPS['sum'] = (0, NUMROWS * NUMCOLS)

# Store the total number of individual values (this will be the size of the outputs list
# returned to GoldSim/GSPy)
TOTALOVARS = sum(list(OVARPROPS.items())[-1][1])

# Initialize global variables to store outputs returned to GoldSim/GSPy
# <Initialize output values> (useful if an error is encountered and default values must be returned)

####### ----- ESSENTIAL FUNCTIONS TO DEFINE ----- #######

# The following functions are called by GSPy.pyd and therefore *must* be defined in GSPyModule.py:
#
#   - ReturnPyModuleVersion(): Returns the version of the custom Python module
#   - NumberOfInputs(): Returns number of inputs needed for custom Python module calculations
#   - NumberOfOutputs(): Returns number of outputs returned by custom Python module calculations
#   - DoCalcsAndReturnValues(*inputsList): Executes the custom calculations specified by the user and returns results
#
# NOTE: The asterisk in *inputsList is essential syntax (but another variable name can be used
#       instead of 'inputsList', if desired). GSPy.pyd passes input arguments as individual scalar
#       values. The asterisk combines these into a tuple.

# Optional initialization function
#def InitializeSimulation():
#    print("Running Python function: InitializeSimulation...")
#    # <include any initialization commands>
#    return 0

# Function used to query the version of this custom Python module
def ReturnPyModuleVersion():
    return PYMODULEVERSION

# Function used to query the number of inputs needed for the custom Python module calculations
def NumberOfInputs():
    return TOTALIVARS

# Function used to query the number of outputs returned by the custom Python module calculations
def NumberOfOutputs():
    return TOTALOVARS

# Executes the custom calculations
def DoCalcsAndReturnValues(*inputsList):
    print("Running Python function: DoCalcsAndReturnValues...")
    # Convert the inputsList tuple to a list
    inputsList = list(inputsList)
    
    # Get lists of values for each matrix
    A = inputsList[IVARPROPS['A'][0]:sum(IVARPROPS['A'])]
    B = inputsList[IVARPROPS['B'][0]:sum(IVARPROPS['B'])]
    
    # Convert lists to numpy arrays of correct dimensions
    A = np.array(A).reshape(NUMROWS, NUMCOLS)
    B = np.array(B).reshape(NUMROWS, NUMCOLS)
    
    # Add the matrices
    result = A + B
    
    # Convert the result back to a list
    result = result.flatten()
    result = result.tolist()
    
    # Return the result to GSPy.pyd as a tuple
    return tuple(result)

#def FinalizeSimulation():
#    print("Running Python function: FinalizeSimulation...")
#    # <include any initialization commands>
#    return 0

####### ----- DEFINE ADDITIONAL HELPER FUNCTIONS ----- #######

# The following functions are not called directly by GSPy.pyd, but they are
# essential helper functions to carry out the custom calculations

# <FUNCTIONS DEFINED HERE>
