# Deprecated Files

This folder contains files from the previous IR learning system that was replaced with the Gree AC library integration.

## Files moved here:

- `ir_control_old.cpp` - Old implementation of IR learning and control system
- `validate_ir_system.py` - Validation script for the IR learning system  
- `IR_MAPPING_DOCUMENTATION.md` - Documentation for the IR button mapping system

## Why these files were deprecated:

The system was upgraded from a manual IR learning approach to direct Gree AC protocol communication using the `ir_Gree.h` library. This provides:

- Zero setup required (no IR learning needed)
- More reliable communication
- Direct protocol support
- Simplified user experience

These files are kept for reference but are no longer part of the active codebase.
