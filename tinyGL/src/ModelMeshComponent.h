#pragma once
#include "MeshComponent.h"

namespace tinyGL
{
	class CModelMeshComponent : public CMeshComponent
	{
	public:
		CModelMeshComponent(const SRenderResourceDesc& render_resource_desc);
		virtual void Draw(const SSceneRenderInfo& scene_render_info) override;
	};
}