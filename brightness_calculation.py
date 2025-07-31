#!/usr/bin/env python3
"""
Mathematical calculation of expected cube brightness in GameEngineDarkest forward rendering pipeline.
This script calculates the theoretical RGB output values based on the lighting equations.
"""

import numpy as np
import math

def normalize_vector(v):
    """Normalize a 3D vector"""
    length = math.sqrt(v[0]**2 + v[1]**2 + v[2]**2)
    if length == 0:
        return v
    return [v[0]/length, v[1]/length, v[2]/length]

def dot_product(a, b):
    """Calculate dot product of two 3D vectors"""
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]

def calculate_lighting(vertex_color, normal, light_direction, light_color, ambient_strength):
    """
    Calculate lighting based on forward rendering pipeline shader equations
    
    Fragment shader equation from ForwardRenderPipeline.cpp:
    vec3 ambient = ambientStrength * lightColor;
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambient + diffuse) * vertexColor;
    """
    
    normal = normalize_vector(normal)
    light_direction = normalize_vector(light_direction)
    
    ambient = [ambient_strength * light_color[i] for i in range(3)]
    
    diff = max(dot_product(normal, light_direction), 0.0)
    diffuse = [diff * light_color[i] for i in range(3)]
    
    lighting = [ambient[i] + diffuse[i] for i in range(3)]
    
    result = [lighting[i] * vertex_color[i] for i in range(3)]
    
    result = [min(1.0, max(0.0, result[i])) for i in range(3)]
    
    return result, ambient, diffuse, diff

def main():
    print("🔢 GameEngineDarkest Cube Brightness Calculation")
    print("=" * 60)
    
    ambient_strength = 0.6  # Increased from default
    light_color = [5.0, 5.0, 5.0]  # Dramatically increased white light
    light_direction = [0.0, -1.0, 0.0]  # Pointing downward (from above)
    
    print(f"Lighting Parameters:")
    print(f"  Ambient Strength: {ambient_strength}")
    print(f"  Light Color: RGB({light_color[0]}, {light_color[1]}, {light_color[2]})")
    print(f"  Light Direction: ({light_direction[0]}, {light_direction[1]}, {light_direction[2]})")
    print()
    
    cube_faces = [
        {"color": [1.0, 0.0, 0.0], "normal": [0.0, 0.0, 1.0], "name": "Front (Red)"},
        {"color": [0.0, 1.0, 0.0], "normal": [0.0, 0.0, -1.0], "name": "Back (Green)"},
        {"color": [0.0, 0.0, 1.0], "normal": [-1.0, 0.0, 0.0], "name": "Left (Blue)"},
        {"color": [1.0, 1.0, 0.0], "normal": [1.0, 0.0, 0.0], "name": "Right (Yellow)"},
        {"color": [1.0, 0.0, 1.0], "normal": [0.0, 1.0, 0.0], "name": "Top (Magenta)"},
        {"color": [0.0, 1.0, 1.0], "normal": [0.0, -1.0, 0.0], "name": "Bottom (Cyan)"},
    ]
    
    print("Cube Face Brightness Calculations:")
    print("-" * 60)
    
    total_brightness = 0
    max_brightness = 0
    
    for face in cube_faces:
        result, ambient, diffuse, diff_factor = calculate_lighting(
            face["color"], face["normal"], light_direction, light_color, ambient_strength
        )
        
        rgb_255 = [int(result[i] * 255) for i in range(3)]
        brightness = sum(result) / 3.0  # Average brightness
        
        total_brightness += brightness
        max_brightness = max(max_brightness, brightness)
        
        print(f"{face['name']}:")
        print(f"  Vertex Color: RGB({face['color'][0]}, {face['color'][1]}, {face['color'][2]})")
        print(f"  Normal: ({face['normal'][0]}, {face['normal'][1]}, {face['normal'][2]})")
        print(f"  Dot Product (N·L): {diff_factor:.3f}")
        print(f"  Ambient: RGB({ambient[0]:.3f}, {ambient[1]:.3f}, {ambient[2]:.3f})")
        print(f"  Diffuse: RGB({diffuse[0]:.3f}, {diffuse[1]:.3f}, {diffuse[2]:.3f})")
        print(f"  Final Color: RGB({result[0]:.3f}, {result[1]:.3f}, {result[2]:.3f})")
        print(f"  RGB (0-255): RGB({rgb_255[0]}, {rgb_255[1]}, {rgb_255[2]})")
        print(f"  Brightness: {brightness:.3f}")
        print()
    
    avg_brightness = total_brightness / len(cube_faces)
    
    print("Summary:")
    print("-" * 30)
    print(f"Average Brightness: {avg_brightness:.3f}")
    print(f"Maximum Brightness: {max_brightness:.3f}")
    print(f"Expected Visibility: {'VISIBLE' if max_brightness > 0.1 else 'TOO DIM'}")
    
    print("\nCamera Analysis:")
    print("-" * 30)
    print("Camera Position: (0, 20, 10)")
    print("Camera Target: (0, 0, 0)")
    print("Camera looks down at cubes positioned around origin")
    
    camera_direction = normalize_vector([0, -20, -10])  # From camera to origin
    print(f"Camera Direction: ({camera_direction[0]:.3f}, {camera_direction[1]:.3f}, {camera_direction[2]:.3f})")
    
    print("\nFace Visibility from Camera:")
    for face in cube_faces:
        face_to_camera = dot_product(face["normal"], [-camera_direction[i] for i in range(3)])
        visibility = max(0, face_to_camera)
        print(f"  {face['name']}: {visibility:.3f} (0=not visible, 1=fully visible)")
    
    print("\n" + "="*60)
    if max_brightness > 0.1:
        print("🟢 CONCLUSION: Cubes should be CLEARLY VISIBLE")
        print("   The lighting calculations show bright output values.")
        print("   The rendering pipeline issue is likely technical, not mathematical.")
    else:
        print("🔴 CONCLUSION: Cubes may be TOO DIM to see")
        print("   Consider increasing ambient or light intensity further.")

if __name__ == "__main__":
    main()
