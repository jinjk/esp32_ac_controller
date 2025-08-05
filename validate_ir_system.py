#!/usr/bin/env python3
"""
Quick validation script to check if our IR mapping system looks correct
"""

import os
import re

def check_file_exists(filepath):
    """Check if a file exists and return its content"""
    if os.path.exists(filepath):
        with open(filepath, 'r') as f:
            return f.read()
    return None

def validate_ir_system():
    """Validate the IR button mapping system implementation"""
    print("🔍 Validating ESP32-S3 AC Controller IR System...")
    
    base_path = "/home/jjin/PlatformIO/Projects/esp32_ac_controller"
    
    # Check header file
    header_path = os.path.join(base_path, "include", "ir_control.h")
    header_content = check_file_exists(header_path)
    
    if not header_content:
        print("❌ IR control header file not found")
        return False
        
    # Check for essential structures
    checks = [
        ("IRButton enum", "enum IRButton"),
        ("IRCodeMap struct", "struct IRCodeMap"),
        ("IRLearningState struct", "struct IRLearningState"),
        ("buttonNames extern", "extern const char* buttonNames"),
        ("learningState extern", "extern IRLearningState learningState"),
        ("startIRLearning function", "void startIRLearning"),
        ("sendIRButton function", "void sendIRButton"),
    ]
    
    print("\n📋 Header File Validation:")
    for check_name, pattern in checks:
        if pattern in header_content:
            print(f"  ✅ {check_name}")
        else:
            print(f"  ❌ {check_name}")
    
    # Check implementation file
    impl_path = os.path.join(base_path, "src", "ir_control.cpp")
    impl_content = check_file_exists(impl_path)
    
    if not impl_content:
        print("\n❌ IR control implementation file not found")
        return False
    
    impl_checks = [
        ("IRCodeMap variable", "IRCodeMap irCodes"),
        ("IRLearningState variable", "IRLearningState learningState"),
        ("buttonNames array", "const char* buttonNames"),
        ("loadAllIRCodes function", "void loadAllIRCodes()"),
        ("saveIRCodeForButton function", "void saveIRCodeForButton"),
        ("getIRCodeForButton function", "String getIRCodeForButton"),
        ("processIRLearning function", "bool processIRLearning()"),
    ]
    
    print("\n📝 Implementation File Validation:")
    for check_name, pattern in impl_checks:
        if pattern in impl_content:
            print(f"  ✅ {check_name}")
        else:
            print(f"  ❌ {check_name}")
    
    # Check web server integration
    web_path = os.path.join(base_path, "src", "web_server.cpp")
    web_content = check_file_exists(web_path)
    
    if web_content:
        web_checks = [
            ("IR button test interface", "IR Button Test"),
            ("sendIRButton function", "sendIRButton"),
            ("handleIRSend handler", "void handleIRSend"),
            ("/send_ir route", '"/send_ir"'),
            ("Button mapping progress", "learningState.learnedButtons"),
        ]
        
        print("\n🌐 Web Interface Validation:")
        for check_name, pattern in web_checks:
            if pattern in web_content:
                print(f"  ✅ {check_name}")
            else:
                print(f"  ❌ {check_name}")
    
    # Count IR buttons defined
    button_matches = re.findall(r'IR_[A-Z_]+', header_content)
    unique_buttons = set(button_matches)
    print(f"\n🎯 IR Button Types Found: {len(unique_buttons)}")
    for button in sorted(unique_buttons):
        print(f"  • {button}")
    
    # Check for potential issues
    print("\n⚠️  Potential Issues:")
    issues = []
    
    if "learnedIRHex" in impl_content:
        issues.append("Legacy learnedIRHex variable still present")
    
    if "void learnIR()" in impl_content:
        issues.append("Legacy learnIR function still present")
    
    if len(issues) == 0:
        print("  ✅ No obvious issues detected")
    else:
        for issue in issues:
            print(f"  ⚠️  {issue}")
    
    print(f"\n✨ IR Mapping System Status: {'READY' if len(issues) == 0 else 'NEEDS CLEANUP'}")
    return True

if __name__ == "__main__":
    validate_ir_system()
