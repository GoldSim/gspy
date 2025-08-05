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
import collections, datetime

####### ---------- DEFINE CONSTANTS ---------- #######

# Python module version
PYMODULEVERSION = 0.15

# Date corresponding to Julian day zero in GoldSim
DAYZERO = datetime.datetime(1899, 12, 30)

# Number of zones
NUMZONES = 3

# Name of log file
LOGFILENAME = "Snowmelt_Log_File.txt" 

# NOTE: snow17.py *must* be in the same folder as this file
import snow17 as s17

# Initialize separate instances of SnowmeltCalculator
smcalc = []
for idx in range(NUMZONES):
    smcalc.append(s17.SnowmeltCalculator())

# IVARPROPS is short for input variable properties. This OrderedDict contains start
# indexes and sizes of all inputs expected from GoldSim/GSPy. The keys are the
# variable names. The values are tuples where the first item is the start index for
# the corresponding variable and the second item is the size of that variable. 
# Conveniently, the start index of any variable is the sum of the tuple for the
# immediately preceding variable (i.e. the start index of the previous variable plus
# the size of the previous variable).
IVARPROPS = collections.OrderedDict()
IVARPROPS['eday'] = (0, 1)
IVARPROPS['time'] = (sum(IVARPROPS['eday']), 1)
IVARPROPS['precip'] = (sum(IVARPROPS['time']), 1)
IVARPROPS['tairs'] = (sum(IVARPROPS['precip']), NUMZONES)      # Air temps for NUMZONES zones
IVARPROPS['elevations'] = (sum(IVARPROPS['tairs']), NUMZONES)  # Elevations for NUMZONES zones

# Store the total number of individual values (this will be the size of the inputs list
# provided from GoldSim/GSPy)
TOTALIVARS = sum(list(IVARPROPS.items())[-1][1])

# OVARPROPS is short for output variable properties. This OrderedDict contains start
# indexes and sizes of all outputs to be returned to GoldSim/GSPy. The keys are the
# variable names. The values are tuples where the first item is the start index for
# the corresponding variable and the second item is the size of that variable.
OVARPROPS = collections.OrderedDict()
OVARPROPS['SWE'] = (0, NUMZONES)   # SWE for NUMZONES zones
OVARPROPS['outflow'] = (sum(OVARPROPS['SWE']), NUMZONES)   # outflow for NUMZONES zones

# Store the total number of individual values (this will be the size of the outputs list
# returned to GoldSim/GSPy)
TOTALOVARS = sum(list(OVARPROPS.items())[-1][1])

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
#    global fo
#    
#    # Create/open log file (use "w" to start with empty log file)
#    fo = open(LOGFILENAME, "w")
#    
#    # Return 'None' if error encountered; Otherwise return any other value
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
    # Declare global variables that will be used in this function
    global SWE
    global outflow
    #global fo
    
    # Open log file (use "a" to append content to log file)
    #fo = open(LOGFILENAME, "a")
    
    # Convert the inputsList tuple to a list
    inputsList = list(inputsList)
    
    # Retrieve input values from GoldSim (using start index and size)
    # NOTE: If the variable is a scalar, just reference by start index to get the
    # input as a scalar (rather than a list). If it is a vector input, use start
    # index and size to get the input as a list
    current_eday = inputsList[ IVARPROPS['eday'][0] ]
    time_julian = inputsList[ IVARPROPS['time'][0] ]
    precip = inputsList[ IVARPROPS['precip'][0] ]
    tairs = inputsList[ IVARPROPS['tairs'][0]:sum(IVARPROPS['tairs']) ]
    elevations = inputsList[ IVARPROPS['elevations'][0]:sum(IVARPROPS['elevations']) ]
    
    # Convert Julian days (from GoldSim) to datetime objects
    time = DAYZERO + datetime.timedelta(days = time_julian)
    
    # Call snowmelt function inside try statement in case there is an error in snow17
    try:
        # Create empty lists to store the SWE and outflow results (for different zones)
        SWE = []
        outflow = []
        # Loop over each of the zones
        for idx in range(NUMZONES):
            # Prepare inputs for snow17
            time_input = [time]
            precip_input = [precip]
            tair_input = [tairs[idx]]
            # Call snow17 for the current zone
            current_swe, current_outflow = \
                smcalc[idx].snow17(time_input, precip_input, tair_input)
            # Store SWE and outflow return values
            SWE.append(current_swe[0])
            outflow.append(current_outflow[0])
    except:
        # Record error message in log file
        #fo.write("Error in snow17 at " + str(current_eday) + " days!)\n")
        #fo.close()
        # Return zero values for SWE and outflow
        SWE = [0] * NUMZONES
        outflow = [0] * NUMZONES
        # Concatenate the results
        result = SWE + outflow
        # Return the result to GSPy.pyd as a tuple
        return tuple(result)
    
    # Arrival at this point in code execution implies snow17 call was successful
    
    # Write to run log that no errors were encountered
    #fo.write("Execution of snow17 at " + str(current_eday) + " days appears to have been successful!)\n")
    #fo.close()
    
    # Return SWE and outflow results for the different elevations
    # Concatenate the results
    result = SWE + outflow
    # Return the result to GSPy.pyd as a tuple
    return tuple(result)
    

####### ----- DEFINE ADDITIONAL HELPER FUNCTIONS ----- #######

# The following functions are not called directly by GSPy.pyd, but they are
# essential helper functions to carry out the custom calculations

