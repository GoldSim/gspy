# Security Guidelines for GSPy

## What GSPy Is (and Is Not)

**What GSPy Is**: A lightweight, open-source C++ bridge that connects GoldSim to a user's own standard Python environment. It acts as a data marshaller between GoldSim's External element and Python scripts, enabling users to leverage Python's capabilities within GoldSim simulations.

**What GSPy Is Not**: GSPy is **not** a formal GoldSim product, a Python distribution, or a formally supported tool. GoldSim support cannot provide assistance for custom Python code, library usage, or environment configuration issues.

## DISCLAIMER

**USE AT YOUR OWN RISK**: GSPy is provided "as is" without warranty of any kind. Users assume full responsibility for testing, validation, and deployment in their specific environments. GoldSim Technology Group and contributors shall not be liable for any damages, data loss, system issues, or other consequences arising from the use of this software.

**TESTING REQUIRED**: All users must thoroughly test GSPy in their specific environment before production use.

---

## Security Model: Three Components, Shared Responsibility

GSPy consists of three distinct components with different security profiles:

### Part 1: The Renamed `GSPy.dll` (Safe by Design)

The GSPy dynamic library itself is designed to be enterprise-safe:

- **No Admin Rights Required** - Installs and runs with user-level permissions only
- **No System Modifications** - Does not modify Windows PATH, Registry, or system settings  
- **No Network Access** - The DLL itself makes no network connections or external communications
- **Limited File Access** - Only reads the matching JSON configuration file and Python script as specified in the config
- **Memory Safe** - Proper exception handling, bounds checking, and resource cleanup
- **Process Isolated** - All operations contained within the GoldSim process boundary
- **Naming Convention Security** - Uses a strict naming convention where the DLL and JSON config must have identical names

### Part 2: The JSON Configuration File (User-Controlled)

The configuration file defines the interface contract:

- **No Executable Code** - Contains only data definitions and file paths
- **Human Readable** - JSON format allows easy inspection and validation
- **Version Controlled** - Can be tracked in source control systems
- **Audit Friendly** - Clear specification of inputs, outputs, and Python script location

### Part 3: Your Python Script (NOT Sandboxed)

**CRITICAL**: Python scripts executed by GSPy have **full system access** and are **NOT sandboxed**.

Your Python scripts can:
- Access any file on the system
- Make network connections  
- Execute system commands (`os.system`, `subprocess`, etc.)
- Import and use any Python library
- Modify system settings (if user has permissions)
- Do anything the current user can do

---

## Security Best Practices

### For Users
1. **Review Your Code** - Always audit Python scripts before deployment
2. **Use Trusted Sources** - Only use Python libraries from trusted repositories
3. **Test in Isolation** - Test scripts in development environments first
4. **Principle of Least Privilege** - Run GoldSim with minimal necessary permissions
5. **Validate Configuration** - Ensure JSON configuration files contain only expected data
6. **Secure File Paths** - Use absolute paths in JSON config to prevent path traversal issues

### For IT Departments  
1. **Whitelist the DLL** - `GSPy.dll` (and renamed versions) can be safely whitelisted in security software
2. **Review User Scripts** - Implement code review processes for Python scripts and JSON configurations
3. **Monitor Network Activity** - Watch for unexpected network connections from user scripts
4. **Audit Logs** - Review generated log files (e.g., `my_calc_log.txt`) for operational monitoring
5. **File System Monitoring** - Monitor access to Python scripts and configuration files
6. **Version Control** - Track changes to Python scripts and JSON configurations

---

## Enterprise Deployment

GSPy is designed for corporate environments:

- **Antivirus Friendly**: Standard Windows DLL with proper code signing
- **Audit Trail**: Comprehensive logging for compliance requirements  
- **Deterministic**: Predictable, repeatable operations
- **Version Tracking**: Embedded version information for asset management
- **Configuration Management**: JSON-based configuration enables version control and change tracking
- **Naming Convention**: Strict DLL-JSON naming requirement prevents configuration mismatches

---

## Data Security Considerations

### Data Flow
- **Input Data**: GoldSim passes data to GSPy via memory arrays (not files)
- **Output Data**: Python results are passed back through memory (not stored on disk)
- **Configuration**: JSON file contains interface definitions but no sensitive data
- **Logging**: Log files may contain debug information about data processing

### Recommendations
- **Sensitive Data**: Be cautious when processing sensitive data in Python scripts
- **Log Files**: Review log file contents for any unintended data exposure
- **Memory**: Data exists temporarily in memory during processing
- **Error Messages**: Python exceptions may contain data that gets logged

---

## Legal Notice

GSPy is provided under the terms specified in the LICENSE file. Users acknowledge they have read, understood, and agree to the terms of use and disclaimer before deploying this software.
