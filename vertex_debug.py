#!/usr/bin/env python3
"""
Debug script to analyze vertex attribute binding issues in GameEngineDarkest.
This script examines the vertex data flow from mesh creation to GPU rendering.
"""

import re
import os

def analyze_vertex_flow():
    """Analyze the vertex data flow from mesh to GPU"""
    
    print("🔍 Vertex Attribute Flow Analysis")
    print("=" * 50)
    
    if not os.path.exists("engine.log"):
        print("❌ No engine.log found - run demo first")
        return
    
    with open("engine.log", 'r') as f:
        log_content = f.read()
    
    mesh_creation = re.findall(r'Creating cube mesh with size: ([\d.]+)', log_content)
    print(f"🎲 Cube Mesh Creation: {len(mesh_creation)} cubes created")
    
    vertex_data = re.findall(r'First vertex - pos: \(([-\d.]+), ([-\d.]+), ([-\d.]+)\)', log_content)
    if vertex_data:
        print(f"📍 First Vertex Position: {vertex_data[0]}")
    
    color_data = re.findall(r'First vertex - color: \(([-\d.]+), ([-\d.]+), ([-\d.]+)\)', log_content)
    if color_data:
        print(f"🎨 First Vertex Color: {color_data[0]}")
    
    vao_creation = re.findall(r'VertexArray created with ID: (\d+)', log_content)
    print(f"🔗 VertexArrays Created: {len(vao_creation)}")
    
    attr_setup = re.findall(r'Setting vertex attribute (\d+) with (\d+) components, stride=(\d+), offset=(\d+)', log_content)
    if attr_setup:
        print(f"⚙️  Vertex Attributes Setup:")
        for attr_idx, components, stride, offset in attr_setup[:6]:  # Show first 6
            print(f"   Attribute {attr_idx}: {components} components, stride={stride}, offset={offset}")
    
    attr_errors = re.findall(r'OpenGL error after glVertexAttribPointer for attribute (\d+): (\d+)', log_content)
    if attr_errors:
        print(f"❌ Vertex Attribute Errors:")
        for attr_idx, error_code in attr_errors:
            print(f"   Attribute {attr_idx}: Error {error_code}")
    else:
        print("✅ No vertex attribute setup errors")
    
    shader_programs = re.findall(r'Current shader program: (\d+)', log_content)
    active_programs = [p for p in shader_programs if p != '0']
    print(f"🔧 Active Shader Programs: {len(set(active_programs))} unique programs")
    
    uniform_locations = re.findall(r'Uniform location for (\w+): (-?\d+)', log_content)
    if uniform_locations:
        print(f"📋 Shader Uniform Locations:")
        uniform_dict = {}
        for name, location in uniform_locations:
            if name not in uniform_dict:
                uniform_dict[name] = location
        for name, location in list(uniform_dict.items())[:8]:  # Show first 8
            status = "✅" if int(location) >= 0 else "❌"
            print(f"   {status} {name}: {location}")
    
    draw_elements = re.findall(r'Drawing mesh with (\d+) indices using glDrawElements', log_content)
    draw_arrays = re.findall(r'Drawing mesh with (\d+) vertices using glDrawArrays', log_content)
    
    print(f"🎨 Draw Calls:")
    print(f"   glDrawElements: {len(draw_elements)} calls")
    print(f"   glDrawArrays: {len(draw_arrays)} calls")
    
    if draw_elements:
        indices_counts = [int(count) for count in draw_elements]
        print(f"   Indices per call: {set(indices_counts)}")
    
    print("\n" + "=" * 50)
    print("🎯 POTENTIAL ISSUES ANALYSIS")
    
    issues = []
    
    if not vertex_data:
        issues.append("❌ No vertex position data found in logs")
    
    if not color_data:
        issues.append("❌ No vertex color data found in logs")
    
    if not vao_creation:
        issues.append("❌ No VertexArrays being created")
    
    if not attr_setup:
        issues.append("❌ No vertex attributes being set up")
    
    if not active_programs:
        issues.append("❌ No active shader programs during rendering")
    
    if uniform_locations:
        failed_uniforms = [name for name, loc in uniform_locations if int(loc) < 0]
        if failed_uniforms:
            issues.append(f"❌ Failed uniform bindings: {set(failed_uniforms)}")
    
    if draw_elements and len(set([int(count) for count in draw_elements])) == 1:
        expected_indices = int(draw_elements[0])
        if expected_indices != 36:
            issues.append(f"❌ Unexpected indices count: {expected_indices} (expected 36 for cube)")
    
    if issues:
        print("CRITICAL ISSUES FOUND:")
        for issue in issues:
            print(f"  {issue}")
    else:
        print("✅ Vertex data flow appears correct")
        print("🔍 Issue likely in shader logic or matrix transformations")
    
    print("\n📋 RECOMMENDED NEXT STEPS:")
    if issues:
        print("1. Fix vertex attribute binding issues")
        print("2. Verify shader uniform transmission")
        print("3. Check OpenGL state management")
    else:
        print("1. Verify matrix transformations (MVP matrices)")
        print("2. Check fragment shader lighting calculations")
        print("3. Investigate framebuffer/viewport setup")
        print("4. Test with simplified single-color shader")

if __name__ == "__main__":
    analyze_vertex_flow()
