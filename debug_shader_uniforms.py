#!/usr/bin/env python3
"""
Debug script to analyze shader uniform transmission and OpenGL state in GameEngineDarkest.
This script parses debug logs to verify if shader uniforms are properly reaching the GPU.
"""

import re
import os

def analyze_debug_logs():
    """Analyze engine debug logs for shader uniform and OpenGL state issues"""
    
    log_file = "engine.log"
    if not os.path.exists(log_file):
        print("❌ No engine.log file found")
        return
    
    print("🔍 Analyzing Shader Uniform Transmission")
    print("=" * 50)
    
    with open(log_file, 'r') as f:
        log_content = f.read()
    
    shader_compile_matches = re.findall(r'Shader compiled successfully', log_content)
    print(f"✅ Shader Compilations: {len(shader_compile_matches)}")
    
    shader_link_matches = re.findall(r'Shader program linked successfully', log_content)
    print(f"✅ Shader Program Links: {len(shader_link_matches)}")
    
    uniform_warnings = re.findall(r"Uniform '([^']+)' not found in shader", log_content)
    if uniform_warnings:
        print(f"⚠️  Missing Uniforms: {set(uniform_warnings)}")
    else:
        print("✅ No missing uniform warnings")
    
    matrix_uniforms = re.findall(r'Set Matrix4 uniform: (\w+)', log_content)
    print(f"📊 Matrix4 Uniforms Set: {set(matrix_uniforms)}")
    
    vector_uniforms = re.findall(r'Set Vector3 uniform: (\w+)', log_content)
    print(f"📊 Vector3 Uniforms Set: {set(vector_uniforms)}")
    
    gl_errors = re.findall(r'OpenGL error.*: (\d+)', log_content)
    if gl_errors:
        print(f"❌ OpenGL Errors Found: {set(gl_errors)}")
        for error in set(gl_errors):
            error_code = int(error)
            error_name = get_opengl_error_name(error_code)
            print(f"   Error {error_code}: {error_name}")
    else:
        print("✅ No OpenGL errors detected")
    
    draw_calls = re.findall(r'Drawing mesh with (\d+) (indices|vertices)', log_content)
    if draw_calls:
        print(f"🎨 Draw Calls: {len(draw_calls)} total")
        for count, type_name in draw_calls[:3]:  # Show first 3
            print(f"   - {count} {type_name}")
    
    framebuffer_ops = re.findall(r'(glReadPixels|framebuffer)', log_content, re.IGNORECASE)
    print(f"🖼️  Framebuffer Operations: {len(framebuffer_ops)}")
    
    print("\n" + "=" * 50)
    print("🎯 DIAGNOSIS SUMMARY")
    
    issues = []
    if uniform_warnings:
        issues.append("❌ Shader uniforms not binding properly")
    if gl_errors:
        issues.append("❌ OpenGL state errors detected")
    if not matrix_uniforms:
        issues.append("❌ No matrix uniforms being set")
    if not vector_uniforms:
        issues.append("❌ No vector uniforms being set")
    if not draw_calls:
        issues.append("❌ No mesh drawing detected")
    
    if issues:
        print("CRITICAL ISSUES FOUND:")
        for issue in issues:
            print(f"  {issue}")
    else:
        print("✅ All basic rendering operations appear functional")
        print("🔍 Issue likely in vertex attribute layout or shader logic")

def get_opengl_error_name(error_code):
    """Convert OpenGL error code to human-readable name"""
    error_names = {
        1280: "GL_INVALID_ENUM",
        1281: "GL_INVALID_VALUE", 
        1282: "GL_INVALID_OPERATION",
        1283: "GL_STACK_OVERFLOW",
        1284: "GL_STACK_UNDERFLOW",
        1285: "GL_OUT_OF_MEMORY",
        1286: "GL_INVALID_FRAMEBUFFER_OPERATION"
    }
    return error_names.get(error_code, f"UNKNOWN_ERROR_{error_code}")

if __name__ == "__main__":
    analyze_debug_logs()
