#pragma once

#include <cstddef>
#include <cstdint>

#include "render/resources/shaders/compiled_shader_objects.h"
#include "render/resources/shaders/shader_backend_state.h"
#include "render/resources/shaders/shader_constant_block.h"
#include "render/resources/shaders/shader_parameter_registry.h"
#include "render/resources/system/render_resource_manager.h"

namespace rb4 {

struct RenderContext;

struct RenderPrimaryShaderResource {
    struct Dispatch {
        void (*destruct)(RenderPrimaryShaderResource* shader);
        void (*delete_resource)(RenderPrimaryShaderResource* shader);
        const void* (*source_identifier)(RenderPrimaryShaderResource* shader);
        void* (*backend_name)(RenderPrimaryShaderResource* shader);
        void (*initialize_support_objects)(
            RenderPrimaryShaderResource* shader,
            RenderShaderConstantRegistry* constants,
            RenderShaderParameterRegistrySet* parameters,
            RenderShaderConstantBlock* constant_block,
            RenderShaderBackendState* backend_state);
        std::int32_t (*mode)(RenderPrimaryShaderResource* shader);
        std::int32_t (*variant)(RenderPrimaryShaderResource* shader);
        bool (*validate_permutation)(
            void* shader,
            std::uint32_t stage,
            std::uint64_t key);
        void (*bind_fallback)(void* shader, void* context);
        bool (*supports_render_target_slices)(void* shader);
        bool (*uses_geometry_program)(void* shader);
    };

    Dispatch* dispatch;
    std::int32_t variant;
    bool compiled;
    std::uint8_t reserved_13[3];
    RenderManagedObjectArray compiled_objects[6];
    void* backend_name;
    RenderShaderConstantRegistry* constants;
    RenderShaderParameterRegistrySet* parameters;
    RenderShaderConstantBlock* constant_block;
    RenderShaderBackendState* backend_state;
    RenderShaderParameterBinding render_target_slice_binding;
    std::uint8_t reserved_268[4];
    RenderResourceListNode manager_link;
};

static_assert(offsetof(RenderPrimaryShaderResource::Dispatch, mode) == 40);
static_assert(offsetof(RenderPrimaryShaderResource::Dispatch, variant) == 48);
static_assert(
    offsetof(RenderPrimaryShaderResource::Dispatch, validate_permutation) ==
    56);
static_assert(sizeof(RenderPrimaryShaderResource::Dispatch) == 88);
static_assert(offsetof(RenderPrimaryShaderResource, compiled) == 12);
static_assert(offsetof(RenderPrimaryShaderResource, compiled_objects) == 16);
static_assert(offsetof(RenderPrimaryShaderResource, backend_name) == 208);
static_assert(offsetof(RenderPrimaryShaderResource, constants) == 216);
static_assert(offsetof(RenderPrimaryShaderResource, parameters) == 224);
static_assert(offsetof(RenderPrimaryShaderResource, constant_block) == 232);
static_assert(offsetof(RenderPrimaryShaderResource, backend_state) == 240);
static_assert(
    offsetof(RenderPrimaryShaderResource, render_target_slice_binding) == 248);
static_assert(
    offsetof(RenderPrimaryShaderResource, manager_link) == 272);
static_assert(sizeof(RenderPrimaryShaderResource) == 288);

RenderPrimaryShaderResource& render_primary_shader_from_link(
    RenderResourceListNode& link);

void render_primary_shader_construct(RenderPrimaryShaderResource& shader);
void render_primary_shader_destruct(RenderPrimaryShaderResource& shader);
void render_primary_shader_prepare(RenderPrimaryShaderResource& shader);
void render_primary_shader_finalize(RenderPrimaryShaderResource& shader);
void render_primary_shader_initialize_backend(
    RenderPrimaryShaderResource& shader);
bool render_primary_shader_bind(
    RenderPrimaryShaderResource& shader,
    RenderContext& context,
    std::uint64_t (&keys)[kRenderShaderProgramKeyCount]);
void render_primary_shader_register(RenderPrimaryShaderResource& shader);
void render_primary_shader_clear_compiled_objects(
    RenderPrimaryShaderResource& shader);

}  // namespace rb4
