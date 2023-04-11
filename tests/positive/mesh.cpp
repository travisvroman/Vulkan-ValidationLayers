/*
 * Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (c) 2015-2023 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "generated/vk_extension_helper.h"

TEST_F(VkPositiveLayerTest, MeshShaderOnly) {
    TEST_DESCRIPTION("Test using a mesh shader without a vertex shader.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char meshShaderText[] = R"glsl(
        #version 450
        #extension GL_NV_mesh_shader : require
        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;
        void main() {
              gl_MeshVerticesNV[0].gl_Position = vec4(-1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[1].gl_Position = vec4( 1.0, -1.0, 0, 1);
              gl_MeshVerticesNV[2].gl_Position = vec4( 0.0,  1.0, 0, 1);
              gl_PrimitiveIndicesNV[0] = 0;
              gl_PrimitiveIndicesNV[1] = 1;
              gl_PrimitiveIndicesNV[2] = 2;
              gl_PrimitiveCountNV = 1;
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    helper.gp_ci_.pVertexInputState = nullptr;
    helper.gp_ci_.pInputAssemblyState = nullptr;

    helper.InitState();

    helper.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, MeshShaderPointSize) {
    TEST_DESCRIPTION("Test writing point size in a mesh shader.");

    AddRequiredExtensions(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    AddRequiredExtensions(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    // Create a device that enables mesh_shader
    auto mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    auto features2 = GetPhysicalDeviceFeatures2(mesh_shader_features);
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &features2));
    if (mesh_shader_features.meshShader != VK_TRUE) {
        GTEST_SKIP() << "Mesh shader feature not supported";
    }

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    static const char meshShaderText[] = R"glsl(
        #version 460
        #extension GL_NV_mesh_shader : enable
        layout (local_size_x=1) in;
        layout (points) out;
        layout (max_vertices=1, max_primitives=1) out;
        void main ()
        {
            gl_PrimitiveCountNV = 1u;
            gl_PrimitiveIndicesNV[0] = 0;
            gl_MeshVerticesNV[0].gl_Position = vec4(-0.5, -0.5, 0.0, 1.0);
            gl_MeshVerticesNV[0].gl_PointSize = 4;
        }
    )glsl";

    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV);
    VkShaderObj fs(this, bindStateFragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreatePipelineHelper helper(*this);
    helper.InitInfo();
    helper.shader_stages_ = {ms.GetStageCreateInfo(), fs.GetStageCreateInfo()};

    // Ensure pVertexInputState and pInputAssembly state are null, as these should be ignored.
    helper.gp_ci_.pVertexInputState = nullptr;
    helper.gp_ci_.pInputAssemblyState = nullptr;

    helper.InitState();

    helper.CreateGraphicsPipeline();
}

TEST_F(VkPositiveLayerTest, TaskAndMeshShader) {
    TEST_DESCRIPTION("Test task and mesh shader");

    SetTargetApiVersion(VK_API_VERSION_1_2);
    AddRequiredExtensions(VK_NV_MESH_SHADER_EXTENSION_NAME);
    ASSERT_NO_FATAL_FAILURE(InitFramework());
    if (DeviceValidationVersion() < VK_API_VERSION_1_2) {
        GTEST_SKIP() << "At least Vulkan version 1.2 is required";
    }
    if (!AreRequiredExtensionsEnabled()) {
        GTEST_SKIP() << RequiredExtensionsNotSupported() << " not supported";
    }

    VkPhysicalDeviceMeshShaderFeaturesNV mesh_shader_features = LvlInitStruct<VkPhysicalDeviceMeshShaderFeaturesNV>();
    GetPhysicalDeviceFeatures2(mesh_shader_features);
    if (!mesh_shader_features.meshShader || !mesh_shader_features.taskShader) {
        GTEST_SKIP() << "Test requires (unsupported) meshShader and taskShader features, skipping test.";
    }
    ASSERT_NO_FATAL_FAILURE(InitState(nullptr, &mesh_shader_features));
    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());

    VkPhysicalDeviceVulkan11Properties vulkan11_props = LvlInitStruct<VkPhysicalDeviceVulkan11Properties>();
    GetPhysicalDeviceProperties2(vulkan11_props);

    if ((vulkan11_props.subgroupSupportedStages & VK_SHADER_STAGE_TASK_BIT_NV) == 0) {
        GTEST_SKIP() << "%s VkPhysicalDeviceVulkan11Properties::subgroupSupportedStages does not include "
                        "VK_SHADER_STAGE_TASK_BIT_NV, skipping test.";
    }

    static const char taskShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require
        #extension GL_KHR_shader_subgroup_ballot : require

        #define GROUP_SIZE 32

        layout(local_size_x = 32) in;

        taskNV out Task {
          uint baseID;
          uint subIDs[GROUP_SIZE];
        } OUT;

        void main() {
            uvec4 desc = uvec4(gl_GlobalInvocationID.x);

            // implement some early culling function
            bool render = gl_GlobalInvocationID.x < 32;

            uvec4 vote  = subgroupBallot(render);
            uint tasks = subgroupBallotBitCount(vote);

            if (gl_LocalInvocationID.x == 0) {
                // write the number of surviving meshlets, i.e.
                // mesh workgroups to spawn
                gl_TaskCountNV = tasks;

                // where the meshletIDs started from for this task workgroup
                OUT.baseID = gl_WorkGroupID.x * GROUP_SIZE;
            }
        }
    )glsl";

    static const char meshShaderText[] = R"glsl(
        #version 450

        #extension GL_NV_mesh_shader : require

        layout(local_size_x = 1) in;
        layout(max_vertices = 3) out;
        layout(max_primitives = 1) out;
        layout(triangles) out;

        taskNV in Task {
          uint baseID;
          uint subIDs[32];
        } IN;

        void main() {
            uint meshletID = IN.baseID + IN.subIDs[gl_WorkGroupID.x];
            uvec4 desc = uvec4(meshletID);
        }
    )glsl";

    VkShaderObj ts(this, taskShaderText, VK_SHADER_STAGE_TASK_BIT_NV, SPV_ENV_VULKAN_1_2);
    VkShaderObj ms(this, meshShaderText, VK_SHADER_STAGE_MESH_BIT_NV, SPV_ENV_VULKAN_1_2);

    const auto break_vp = [&](CreatePipelineHelper &helper) {
        helper.shader_stages_ = {ts.GetStageCreateInfo(), ms.GetStageCreateInfo(), helper.fs_->GetStageCreateInfo()};
    };
    CreatePipelineHelper::OneshotTest(*this, break_vp, kErrorBit);
}