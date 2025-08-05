# Security Guidelines for GSPy

## DISCLAIMER

**USE AT YOUR OWN RISK**: GSPy is provided "as is" without warranty of any kind. Users assume full responsibility for testing, validation, and deployment in their specific environments. GoldSim Technology Group and contributors shall not be liable for any damages, data loss, system issues, or other consequences arising from the use of this software.

**TESTING REQUIRED**: All users must thoroughly test GSPy in their specific environment before production use.

---

## Security Model: Two Parts, Two Responsibilities

GSPy consists of two distinct components with very different security profiles:

### Part 1: The `GSPy.dll` (Safe by Design)

The GSPy dynamic library itself is designed to be enterprise-safe:

- **No Admin Rights Required** - Installs and runs with user-level permissions only
- **No System Modifications** - Does not modify Windows PATH, Registry, or system settings  
- **No Network Access** - Makes no network connections or external communications
- **Limited File Access** - Only reads `gspy_script.py` from the same directory as the DLL
- **Memory Safe** - Proper exception handling, bounds checking, and resource cleanup
- **Process Isolated** - All operations contained within the GoldSim process boundary

### Part 2: Your `gspy_script.py` (NOT Sandboxed)

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

### For IT Departments  
1. **Whitelist the DLL** - `GSPy.dll` can be safely whitelisted in security software
2. **Review User Scripts** - Implement code review processes for Python scripts
3. **Monitor Network Activity** - Watch for unexpected network connections from user scripts
4. **Audit Logs** - Review `gspy_log.txt` for operational monitoring

---

## Enterprise Deployment

GSPy is designed for corporate environments:

- **Antivirus Friendly**: Standard Windows DLL with proper code signing
- **Audit Trail**: Comprehensive logging for compliance requirements  
- **Deterministic**: Predictable, repeatable operations
- **Version Tracking**: Embedded version information for asset management

---

## Legal Notice

GSPy is provided under the terms specified in the LICENSE file. Users acknowledge they have read, understood, and agree to the terms of use and disclaimer before deploying this software.
