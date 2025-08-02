#pragma once

#include "../../Core/Math/Vector3.h"
#include "../../Core/Math/Matrix4.h"
#include "../Lighting/Light.h"

namespace GameEngine {
    class DebugRenderer {
    public:
        static void Initialize();
        static void Shutdown();
        static void RenderLightGizmo(const Light& light, const Vector3& position);
        static void RenderWireSphere(const Vector3& center, float radius, const Vector3& color);
        static void RenderWireCone(const Vector3& position, const Vector3& direction, float angle, float range, const Vector3& color);
        static void RenderDirectionalArrow(const Vector3& position, const Vector3& direction, const Vector3& color);
        static void RenderSelectionOutline(const Vector3& center, const Vector3& size, const Vector3& color);
        static void RenderMovementGizmo(const Vector3& position, float size = 1.0f);
        
    private:
        static bool s_initialized;
        static void SetupWireframeShader();
        static void RenderWireframeMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices, const Vector3& color);
    };
}
