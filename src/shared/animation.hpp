#pragma once
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "components.hpp"
#include "animation_components.hpp"

// NOTE: Redefinition of function from Raylib's rmodels.c
static void UpdateModelAnimationVertexBuffers(Model model);

inline LocomotionState get_locomotion_state(const MovementInput& input) {
    if (input.get_backward()) {
        return LocomotionState::Backward;
    }
    if (input.get_forward()) {
        return LocomotionState::Forward;
    }
    if (input.get_strafe_r()) {
        return LocomotionState::StrafeRight;
    }
    if (input.get_strafe_l()) {
        return LocomotionState::StrafeLeft;
    }
    return LocomotionState::Idle;
}

inline Pose sample_pose_from_anim(const ModelAnimation& anim, float frame) {
    Pose pose;

    // Update model animated bones transform matrices for a given frame
    if ((anim.keyframeCount > 0) && (anim.keyframePoses != NULL))
    {
        // Get frame and blending from frame factor required
        int currentFrame = (int)frame;
        int nextFrame = currentFrame + 1;
        float blend = frame - currentFrame;
        blend = Clamp(blend, 0.0f, 1.0f);
        if (currentFrame >= anim.keyframeCount) currentFrame = currentFrame % anim.keyframeCount;
        if (nextFrame >= anim.keyframeCount) nextFrame = nextFrame % anim.keyframeCount;

        // Update all bone transformations
        for (int boneIndex = 0; boneIndex < anim.boneCount; boneIndex++)
        {
            // Compute interpolated pose between current and next frame
            Transform transform;
            transform.translation = Vector3Lerp(
                anim.keyframePoses[currentFrame][boneIndex].translation,
                anim.keyframePoses[nextFrame][boneIndex].translation, blend);
            transform.rotation = QuaternionSlerp(
                anim.keyframePoses[currentFrame][boneIndex].rotation,
                anim.keyframePoses[nextFrame][boneIndex].rotation, blend);
            transform.scale = Vector3Lerp(
                anim.keyframePoses[currentFrame][boneIndex].scale,
                anim.keyframePoses[nextFrame][boneIndex].scale, blend);
            pose.transforms.push_back(transform);
        }
    }

    return pose;
}

inline Pose blend_poses(const Pose& from_pose, const Pose& to_pose, float alpha) {
    if (from_pose.transforms.size() != to_pose.transforms.size()) {
        return to_pose;
    }
    Pose new_pose;
    int bone_count = from_pose.transforms.size();
    alpha = Clamp(alpha, 0.0, 1.0);
    for (int boneIndex = 0; boneIndex < bone_count; boneIndex++) {
        // Get frame-interpolation for first animation
        Vector3 from_pose_trans = from_pose.transforms[boneIndex].translation;
        Quaternion from_pose_rot = from_pose.transforms[boneIndex].rotation;
        Vector3 from_pose_scale = from_pose.transforms[boneIndex].scale;

        // Get frame-interpolation for second animation
        Vector3 to_pose_trans = to_pose.transforms[boneIndex].translation;
        Quaternion to_pose_rot = to_pose.transforms[boneIndex].rotation;
        Vector3 to_pose_scale = to_pose.transforms[boneIndex].scale;

        // Compute interpolated pose between both animations frames
        // NOTE: Storing animation frame data in model.currentPose
        Transform transform;
        transform.translation = Vector3Lerp(from_pose_trans, to_pose_trans, alpha);
        transform.rotation = QuaternionSlerp(from_pose_rot, to_pose_rot, alpha);
        transform.scale = Vector3Lerp(from_pose_scale, to_pose_scale, alpha);

        new_pose.transforms.push_back(transform);
    }
    return new_pose;
}

inline void update_model_pose(Model model, Pose pose) {
    if (model.boneMatrices == NULL) return;
    /* std::cout << "Pose bone count: " << pose.transforms.size() << "\n"; */
    /* std::cout << "Model bone count: " << model.skeleton.boneCount << "\n"; */
    if (pose.transforms.size() != model.skeleton.boneCount) return;

    // Update model animated bones transform matrices for a given frame
    if ((model.skeleton.bones != NULL))
    {
        Matrix bindPoseMatrix = { 0 };
        Matrix currentPoseMatrix = { 0 };

        // Update all bones and bone matrices of model
        for (int boneIndex = 0; boneIndex < model.skeleton.boneCount; boneIndex++)
        {
            // Compute interpolated pose between current and next frame
            // NOTE: Storing animation frame data in model.currentPose
            model.currentPose[boneIndex] = pose.transforms[boneIndex];

            // Compute runtime bone matrix from model current pose
            //-----------------------------------------------------------------------------------
            Transform *bindPoseTransform = &model.skeleton.bindPose[boneIndex];
            bindPoseMatrix = MatrixMultiply(
                MatrixMultiply(MatrixScale(bindPoseTransform->scale.x, bindPoseTransform->scale.y, bindPoseTransform->scale.z),
                    QuaternionToMatrix(bindPoseTransform->rotation)),
                MatrixTranslate(bindPoseTransform->translation.x, bindPoseTransform->translation.y, bindPoseTransform->translation.z));

            Transform *currentPoseTransform = &model.currentPose[boneIndex];
            currentPoseMatrix = MatrixMultiply(
                MatrixMultiply(MatrixScale(currentPoseTransform->scale.x, currentPoseTransform->scale.y, currentPoseTransform->scale.z),
                    QuaternionToMatrix(currentPoseTransform->rotation)),
                MatrixTranslate(currentPoseTransform->translation.x, currentPoseTransform->translation.y, currentPoseTransform->translation.z));

            model.boneMatrices[boneIndex] = MatrixMultiply(MatrixInvert(bindPoseMatrix), currentPoseMatrix);
            //-----------------------------------------------------------------------------------
        }

        // CPU skinning, updates CPU buffers and uploads them to GPU
        // NOTE: On GPU skinning not supported, use CPU skinning
        UpdateModelAnimationVertexBuffers(model);
    }
}

static void UpdateModelAnimationVertexBuffers(Model model)
{
    for (int m = 0; m < model.meshCount; m++)
    {
        Mesh mesh = model.meshes[m];
        Vector3 animVertex = { 0 };
        Vector3 animNormal = { 0 };
        const int vertexValuesCount = mesh.vertexCount*3;

        int boneIndex = 0;
        int boneCounter = 0;
        float boneWeight = 0.0f;
        bool bufferUpdateRequired = false; // Flag to check when anim vertex information is updated

        // Skip if missing bone data or missing anim buffers initialization
        if ((mesh.boneWeights == NULL) || (mesh.boneIndices == NULL) ||
            (mesh.animVertices == NULL) || (mesh.animNormals == NULL)) continue;

        for (int vCounter = 0; vCounter < vertexValuesCount; vCounter += 3)
        {
            mesh.animVertices[vCounter] = 0;
            mesh.animVertices[vCounter + 1] = 0;
            mesh.animVertices[vCounter + 2] = 0;
            if (mesh.animNormals != NULL)
            {
                mesh.animNormals[vCounter] = 0;
                mesh.animNormals[vCounter + 1] = 0;
                mesh.animNormals[vCounter + 2] = 0;
            }

            // Iterates over 4 bones per vertex
            for (int j = 0; j < 4; j++, boneCounter++)
            {
                boneWeight = mesh.boneWeights[boneCounter];
                boneIndex = mesh.boneIndices[boneCounter];

                // Early stop when no transformation will be applied
                if (boneWeight == 0.0f) continue;
                animVertex = (Vector3){ mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                animVertex = Vector3Transform(animVertex, model.boneMatrices[boneIndex]);
                mesh.animVertices[vCounter] += animVertex.x*boneWeight;
                mesh.animVertices[vCounter + 1] += animVertex.y*boneWeight;
                mesh.animVertices[vCounter + 2] += animVertex.z*boneWeight;
                bufferUpdateRequired = true;

                // Normals processing
                // NOTE: Using meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                if ((mesh.normals != NULL) && (mesh.animNormals != NULL ))
                {
                    animNormal = (Vector3){ mesh.normals[vCounter], mesh.normals[vCounter + 1], mesh.normals[vCounter + 2] };
                    animNormal = Vector3Transform(animNormal, MatrixTranspose(MatrixInvert(model.boneMatrices[boneIndex])));
                    mesh.animNormals[vCounter] += animNormal.x*boneWeight;
                    mesh.animNormals[vCounter + 1] += animNormal.y*boneWeight;
                    mesh.animNormals[vCounter + 2] += animNormal.z*boneWeight;
                }
            }
        }

        if (bufferUpdateRequired)
        {
            // Update GPU vertex buffers with updated data (position + normals)
            rlUpdateVertexBuffer(mesh.vboId[SHADER_LOC_VERTEX_POSITION], mesh.animVertices, mesh.vertexCount*3*sizeof(float), 0);
            if (mesh.normals != NULL) rlUpdateVertexBuffer(mesh.vboId[SHADER_LOC_VERTEX_NORMAL], mesh.animNormals, mesh.vertexCount*3*sizeof(float), 0);
        }
    }
}
