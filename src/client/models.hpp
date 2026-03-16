#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

#include "raylib.h"

#include "../shared/const.hpp"
#include "../shared/components.hpp"

inline void load_model_with_animations(
        std::unordered_map<std::string, ModelAsset>& loaded_models,
        std::string model_name
) {
    std::string filename = MODEL_DIR + model_name + ".glb";
    ModelAsset asset = {LoadModel(filename.c_str())};
    int* num_animations;
    std::unordered_map<std::string, ModelAnimation> animation_map{};
    ModelAnimation* animations = LoadModelAnimations(filename.c_str(), num_animations);
    for (int i = 0; i < *num_animations; i++) {
        ModelAnimation* animation = animations + i;
        animation_map[animation->name] = *animation;
    }
    asset.animations = animation_map;
    loaded_models[model_name] = asset;
}

inline void load_all_models(std::unordered_map<std::string, ModelAsset>& loaded_models) {
    // TODO: make lists of models shared between server and client
    loaded_models["sample_world"] = {LoadModel((MODEL_DIR "sample_world.glb"))};
    loaded_models["cube"] = {LoadModel((MODEL_DIR "cube.glb"))};
    loaded_models["quad"] = {LoadModel((MODEL_DIR "quad.glb"))};

    load_model_with_animations(loaded_models, "humanoid");
}

