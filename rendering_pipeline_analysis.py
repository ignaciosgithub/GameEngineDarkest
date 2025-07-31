#!/usr/bin/env python3
"""
Comprehensive step-by-step analysis of the GameEngineDarkest rendering pipeline.
This script traces the complete flow from mesh creation to framebuffer output.
"""

import re
import os
import json

def analyze_rendering_pipeline():
    """Perform detailed step-by-step analysis of the rendering pipeline"""
    
    print("🔍 COMPLETE RENDERING PIPELINE ANALYSIS")
    print("=" * 60)
    
    if not os.path.exists("engine.log"):
        print("❌ No engine.log found - run demo first")
        return
    
    with open("engine.log", 'r') as f:
        log_content = f.read()
    
    steps = []
    
    print("STEP 1: ENGINE INITIALIZATION")
    print("-" * 30)
    
    window_creation = re.findall(r'Window created successfully', log_content)
    glad_init = re.findall(r'GLAD initialized successfully', log_content)
    opengl_version = re.findall(r'OpenGL Version: (.+)', log_content)
    
    step1_status = "✅" if window_creation and glad_init else "❌"
    print(f"{step1_status} Window Creation: {len(window_creation)} successful")
    print(f"{step1_status} GLAD Initialization: {len(glad_init)} successful")
    if opengl_version:
        print(f"✅ OpenGL Version: {opengl_version[0]}")
    
    steps.append({
        "step": 1,
        "name": "Engine Initialization",
        "status": "PASS" if window_creation and glad_init else "FAIL",
        "details": f"Window: {len(window_creation)}, GLAD: {len(glad_init)}"
    })
    
    print("\nSTEP 2: SHADER COMPILATION")
    print("-" * 30)
    
    shader_compiles = re.findall(r'Shader compiled successfully', log_content)
    shader_links = re.findall(r'Shader program linked successfully', log_content)
    shader_errors = re.findall(r'Shader compilation error', log_content)
    
    step2_status = "✅" if shader_compiles and shader_links and not shader_errors else "❌"
    print(f"{step2_status} Shader Compilations: {len(shader_compiles)}")
    print(f"{step2_status} Shader Program Links: {len(shader_links)}")
    print(f"{'❌' if shader_errors else '✅'} Shader Errors: {len(shader_errors)}")
    
    steps.append({
        "step": 2,
        "name": "Shader Compilation",
        "status": "PASS" if shader_compiles and shader_links and not shader_errors else "FAIL",
        "details": f"Compiles: {len(shader_compiles)}, Links: {len(shader_links)}, Errors: {len(shader_errors)}"
    })
    
    print("\nSTEP 3: MESH CREATION AND UPLOAD")
    print("-" * 30)
    
    cube_creation = re.findall(r'Creating cube mesh with size: ([\d.]+)', log_content)
    mesh_uploads = re.findall(r'Mesh uploaded with modern OpenGL buffers', log_content)
    vao_creation = re.findall(r'VertexArray created with ID: (\d+)', log_content)
    vertex_data = re.findall(r'First vertex - pos: \(([-\d.]+), ([-\d.]+), ([-\d.]+)\)', log_content)
    
    step3_status = "✅" if cube_creation and mesh_uploads and vao_creation else "❌"
    print(f"{step3_status} Cube Mesh Creation: {len(cube_creation)}")
    print(f"{step3_status} Mesh Uploads: {len(mesh_uploads)}")
    print(f"{step3_status} VertexArray Creation: {len(vao_creation)}")
    if vertex_data:
        print(f"✅ First Vertex Position: {vertex_data[0]}")
    
    steps.append({
        "step": 3,
        "name": "Mesh Creation and Upload",
        "status": "PASS" if cube_creation and mesh_uploads and vao_creation else "FAIL",
        "details": f"Cubes: {len(cube_creation)}, Uploads: {len(mesh_uploads)}, VAOs: {len(vao_creation)}"
    })
    
    print("\nSTEP 4: VERTEX ATTRIBUTE SETUP")
    print("-" * 30)
    
    attr_setup = re.findall(r'Setting vertex attribute (\d+) with (\d+) components, stride=(\d+), offset=(\d+)', log_content)
    attr_errors = re.findall(r'OpenGL error after glVertexAttribPointer', log_content)
    
    step4_status = "✅" if attr_setup and not attr_errors else "❌"
    print(f"{step4_status} Vertex Attributes Set: {len(attr_setup)}")
    print(f"{'❌' if attr_errors else '✅'} Attribute Setup Errors: {len(attr_errors)}")
    
    if attr_setup:
        print("   Attribute Layout:")
        for attr_idx, components, stride, offset in attr_setup[:3]:  # Show first 3
            print(f"     Attr {attr_idx}: {components} components, stride={stride}, offset={offset}")
    
    steps.append({
        "step": 4,
        "name": "Vertex Attribute Setup",
        "status": "PASS" if attr_setup and not attr_errors else "FAIL",
        "details": f"Attributes: {len(attr_setup)}, Errors: {len(attr_errors)}"
    })
    
    print("\nSTEP 5: CAMERA AND MATRIX SETUP")
    print("-" * 30)
    
    camera_pos = re.findall(r'Camera position: \(([-\d.]+), ([-\d.]+), ([-\d.]+)\)', log_content)
    view_matrix = re.findall(r'View matrix.*?(\[.*?\])', log_content, re.DOTALL)
    proj_matrix = re.findall(r'Projection matrix.*?(\[.*?\])', log_content, re.DOTALL)
    
    step5_status = "✅" if camera_pos and view_matrix and proj_matrix else "❌"
    print(f"{step5_status} Camera Position Set: {len(camera_pos)}")
    print(f"{step5_status} View Matrix Set: {len(view_matrix)}")
    print(f"{step5_status} Projection Matrix Set: {len(proj_matrix)}")
    
    if camera_pos:
        print(f"   Camera Position: {camera_pos[0]}")
    
    steps.append({
        "step": 5,
        "name": "Camera and Matrix Setup",
        "status": "PASS" if camera_pos and view_matrix and proj_matrix else "FAIL",
        "details": f"Camera: {len(camera_pos)}, View: {len(view_matrix)}, Proj: {len(proj_matrix)}"
    })
    
    print("\nSTEP 6: UNIFORM BINDING")
    print("-" * 30)
    
    uniform_locations = re.findall(r'Uniform location for (\w+): (-?\d+)', log_content)
    matrix_uniforms = re.findall(r'Set Matrix4 uniform: (\w+)', log_content)
    vector_uniforms = re.findall(r'Set Vector3 uniform: (\w+)', log_content)
    
    failed_uniforms = [name for name, loc in uniform_locations if int(loc) < 0]
    step6_status = "✅" if uniform_locations and not failed_uniforms else "❌"
    
    print(f"{step6_status} Uniform Locations Found: {len(uniform_locations)}")
    print(f"{step6_status} Matrix4 Uniforms Set: {len(set(matrix_uniforms))}")
    print(f"{step6_status} Vector3 Uniforms Set: {len(set(vector_uniforms))}")
    
    if failed_uniforms:
        print(f"❌ Failed Uniform Bindings: {set(failed_uniforms)}")
    
    steps.append({
        "step": 6,
        "name": "Uniform Binding",
        "status": "PASS" if uniform_locations and not failed_uniforms else "FAIL",
        "details": f"Locations: {len(uniform_locations)}, Failed: {len(failed_uniforms)}"
    })
    
    print("\nSTEP 7: FRAMEBUFFER SETUP")
    print("-" * 30)
    
    fb_creation = re.findall(r'FrameBuffer created with ID: (\d+)', log_content)
    fb_complete = re.findall(r'FrameBuffer is complete', log_content)
    viewport_set = re.findall(r'glViewport\(0, 0, (\d+), (\d+)\)', log_content)
    
    step7_status = "✅" if fb_creation and fb_complete else "❌"
    print(f"{step7_status} FrameBuffer Creation: {len(fb_creation)}")
    print(f"{step7_status} FrameBuffer Complete: {len(fb_complete)}")
    print(f"{'✅' if viewport_set else '❌'} Viewport Set: {len(viewport_set)}")
    
    if viewport_set:
        print(f"   Viewport Size: {viewport_set[0]}")
    
    steps.append({
        "step": 7,
        "name": "Framebuffer Setup",
        "status": "PASS" if fb_creation and fb_complete else "FAIL",
        "details": f"FB Created: {len(fb_creation)}, Complete: {len(fb_complete)}, Viewport: {len(viewport_set)}"
    })
    
    print("\nSTEP 8: RENDERING LOOP")
    print("-" * 30)
    
    clear_calls = re.findall(r'glClear.*called', log_content)
    draw_elements = re.findall(r'Drawing mesh with (\d+) indices using glDrawElements', log_content)
    draw_success = re.findall(r'glDrawElements completed successfully', log_content)
    
    step8_status = "✅" if draw_elements and draw_success else "❌"
    print(f"{'✅' if clear_calls else '❌'} Screen Clear Calls: {len(clear_calls)}")
    print(f"{step8_status} Draw Elements Calls: {len(draw_elements)}")
    print(f"{step8_status} Successful Draw Calls: {len(draw_success)}")
    
    if draw_elements:
        indices_counts = [int(count) for count in draw_elements]
        print(f"   Indices per Draw Call: {set(indices_counts)}")
        print(f"   Total Draw Calls: {len(draw_elements)}")
    
    steps.append({
        "step": 8,
        "name": "Rendering Loop",
        "status": "PASS" if draw_elements and draw_success else "FAIL",
        "details": f"Clear: {len(clear_calls)}, Draw: {len(draw_elements)}, Success: {len(draw_success)}"
    })
    
    print("\nSTEP 9: FRAMEBUFFER READING")
    print("-" * 30)
    
    fb_bind = re.findall(r'Binding framebuffer for reading', log_content)
    pixel_read = re.findall(r'glReadPixels.*called', log_content)
    frame_save = re.findall(r'Frame saved to: (.+)', log_content)
    
    step9_status = "✅" if fb_bind and pixel_read and frame_save else "❌"
    print(f"{step9_status} Framebuffer Bind for Reading: {len(fb_bind)}")
    print(f"{step9_status} Pixel Read Calls: {len(pixel_read)}")
    print(f"{step9_status} Frame Save Operations: {len(frame_save)}")
    
    steps.append({
        "step": 9,
        "name": "Framebuffer Reading",
        "status": "PASS" if fb_bind and pixel_read and frame_save else "FAIL",
        "details": f"Bind: {len(fb_bind)}, Read: {len(pixel_read)}, Save: {len(frame_save)}"
    })
    
    print("\nSTEP 10: OPENGL STATE ANALYSIS")
    print("-" * 30)
    
    gl_errors = re.findall(r'OpenGL error.*: (\d+)', log_content)
    depth_test = re.findall(r'glEnable.*GL_DEPTH_TEST', log_content)
    cull_face = re.findall(r'glEnable.*GL_CULL_FACE', log_content)
    
    step10_status = "✅" if not gl_errors else "❌"
    print(f"{step10_status} OpenGL Errors: {len(gl_errors)}")
    print(f"{'✅' if depth_test else '❌'} Depth Testing Enabled: {len(depth_test)}")
    print(f"{'✅' if cull_face else '❌'} Face Culling Enabled: {len(cull_face)}")
    
    if gl_errors:
        error_codes = set(gl_errors)
        print(f"   Error Codes: {error_codes}")
    
    steps.append({
        "step": 10,
        "name": "OpenGL State Analysis",
        "status": "PASS" if not gl_errors else "FAIL",
        "details": f"Errors: {len(gl_errors)}, Depth: {len(depth_test)}, Cull: {len(cull_face)}"
    })
    
    print("\n" + "=" * 60)
    print("🎯 PIPELINE ANALYSIS SUMMARY")
    print("=" * 60)
    
    passed_steps = [s for s in steps if s["status"] == "PASS"]
    failed_steps = [s for s in steps if s["status"] == "FAIL"]
    
    print(f"✅ PASSED STEPS: {len(passed_steps)}/10")
    for step in passed_steps:
        print(f"   Step {step['step']}: {step['name']}")
    
    if failed_steps:
        print(f"\n❌ FAILED STEPS: {len(failed_steps)}/10")
        for step in failed_steps:
            print(f"   Step {step['step']}: {step['name']} - {step['details']}")
    
    print("\n🔍 CRITICAL ISSUE ANALYSIS")
    print("-" * 40)
    
    critical_issues = []
    
    if not window_creation or not glad_init:
        critical_issues.append("❌ CRITICAL: Engine initialization failed")
    
    if not shader_compiles or not shader_links or shader_errors:
        critical_issues.append("❌ CRITICAL: Shader compilation/linking failed")
    
    if not cube_creation or not mesh_uploads:
        critical_issues.append("❌ CRITICAL: Mesh creation/upload failed")
    
    if not attr_setup or attr_errors:
        critical_issues.append("❌ CRITICAL: Vertex attribute setup failed")
    
    if failed_uniforms:
        critical_issues.append(f"❌ CRITICAL: Uniform binding failed for: {set(failed_uniforms)}")
    
    if not draw_elements or not draw_success:
        critical_issues.append("❌ CRITICAL: Draw calls failed")
    
    if gl_errors:
        critical_issues.append(f"❌ CRITICAL: OpenGL errors detected: {set(gl_errors)}")
    
    if len(passed_steps) == 10:
        print("🟡 ALL PIPELINE STEPS PASS - INVESTIGATING SUBTLE ISSUES:")
        
        if view_matrix and proj_matrix:
            print("   📊 Matrix Analysis Required:")
            print("     - View matrix transformation correctness")
            print("     - Projection matrix near/far planes")
            print("     - Model matrix positioning")
        
        if vector_uniforms:
            print("   💡 Lighting Analysis Required:")
            print("     - Light direction vs surface normals")
            print("     - Ambient vs diffuse contribution")
            print("     - Color clamping in fragment shader")
        
        if camera_pos and viewport_set:
            print("   📷 Camera Analysis Required:")
            print("     - Camera frustum vs object positions")
            print("     - Near/far clipping planes")
            print("     - Field of view calculations")
        
        if fb_creation and frame_save:
            print("   🖼️  Framebuffer Analysis Required:")
            print("     - Color attachment format (RGB vs RGBA)")
            print("     - Depth buffer attachment")
            print("     - Pixel format during glReadPixels")
    
    if critical_issues:
        print("CRITICAL ISSUES FOUND:")
        for issue in critical_issues:
            print(f"  {issue}")
    else:
        print("✅ No critical pipeline failures detected")
        print("🔍 Issue likely in mathematical calculations or state management")
    
    print("\n📋 RECOMMENDED DEBUGGING ACTIONS")
    print("-" * 40)
    
    if critical_issues:
        print("1. Fix critical pipeline failures first")
        print("2. Re-run analysis after fixes")
    else:
        print("1. 🧮 Verify matrix multiplication order (Model * View * Projection)")
        print("2. 🎨 Test with single solid color (bypass lighting)")
        print("3. 📐 Check camera frustum contains cube positions")
        print("4. 🔍 Add fragment shader debug output")
        print("5. 📊 Verify framebuffer color format matches glReadPixels")
    
    analysis_data = {
        "timestamp": "2025-01-31T14:39:12Z",
        "total_steps": len(steps),
        "passed_steps": len(passed_steps),
        "failed_steps": len(failed_steps),
        "critical_issues": critical_issues,
        "steps": steps
    }
    
    with open("pipeline_analysis.json", "w") as f:
        json.dump(analysis_data, f, indent=2)
    
    print(f"\n💾 Detailed analysis saved to: pipeline_analysis.json")

if __name__ == "__main__":
    analyze_rendering_pipeline()
