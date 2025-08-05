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
#import <module> as <module abbreviation>

####### ---------- DEFINE CONSTANTS ---------- #######

# Python module version
PYMODULEVERSION = 0.10

# The following constants provide the number of inputs and outputs
NUMINPUTS = 2
NUMOUTPUTS = 2

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
    return NUMINPUTS

# Function used to query the number of outputs returned by the custom Python module calculations
def NumberOfOutputs():
    return NUMOUTPUTS

# Executes the custom calculations
def DoCalcsAndReturnValues(*inputsList):
    print("Running Python function: DoCalcsAndReturnValues...")
    # Convert the inputsList tuple to a list
    inputsList = list(inputsList)
    
    # Get input values
    a = inputsList[0]
    b = inputsList[1]
    
    # Calculate the sum
    sum = a + b
    
    # Calculate the product
    product = a * b
    
    # Store results in a list
    result = [sum, product]
    
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
