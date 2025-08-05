# GSPy Quick Start Guide

## 5-Minute Setup

### Step 1: Get the Files
You need these two files in the same directory as your GoldSim model:
- `GSPy.dll` (from the `bin/` folder)
- `gspy_script.py` (your Python script)

### Step 2: Create Your Python Script
Create a file named exactly `gspy_script.py`:

```python
def gspy_info():
    """Tell GSPy how many inputs/outputs you need."""
    return {'inputs': 1, 'outputs': 1}

def goldsim_calculate(inputs):
    """Your calculation goes here."""
    # Get the input value
    input_value = inputs.get('input1', 0.0)
    
    # Do your calculation
    result = input_value * 2.0 + 10.0
    
    # Return the result
    return {'output1': result}
```

### Step 3: Configure GoldSim
1. Add an **External Element** to your model
2. Set **DLL Path** to: `GSPy.dll`
3. **Inputs/Outputs**: Let GSPy auto-detect, or set manually (1 input, 1 output)
4. Connect your input and output to other elements

### Step 4: Run
Run your GoldSim model - that's it!

## Common Patterns

### Multiple Outputs
```python
def gspy_info():
    return {'inputs': 1, 'outputs': 3}

def goldsim_calculate(inputs):
    x = inputs['input1']
    return {
        'output1': x * 2,
        'output2': x ** 2, 
        'output3': x + 100
    }
```

### Vector Outputs (Arrays)
```python
def gspy_info():
    return {'inputs': 1, 'outputs': 1}

def goldsim_calculate(inputs):
    x = inputs['input1']
    concentrations = [x/10, x/20, x/30]  # 3-element array
    return {'output1': concentrations}
```

### Using NumPy
```python
import numpy as np

def gspy_info():
    return {'inputs': 1, 'outputs': 1}

def goldsim_calculate(inputs):
    x = inputs['input1']
    result = np.sin(x) * np.exp(-x/10)
    return {'output1': float(result)}  # Convert to Python float
```

## Troubleshooting

**Problem**: "gspy_script.py not found"  
**Solution**: Make sure the script is in the same folder as GSPy.dll

**Problem**: "Function not found"  
**Solution**: Check that you have both `gspy_info()` and `goldsim_calculate()` functions

**Problem**: Wrong number of inputs/outputs  
**Solution**: Check your `gspy_info()` return values match your script logic

**Problem**: DLL crashes  
**Solution**: Make sure you have Python 3.13 installed and accessible

## Next Steps
- Check out the `examples/` folder for more complex scenarios
- Read the full README.md for complete documentation
- Try different input/output combinations
