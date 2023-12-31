#include "Object.h"

#include <glm/ext/matrix_transform.hpp>

void Object::Initialize(DXRI* dxri)
{
    cb.Initialize(dxri, sizeof(ObjectCB));
    cbMapped = cb.Map();
	objCb.Model = glm::mat4(1.0f);
	memcpy(cbMapped, &objCb, sizeof(ObjectCB));

    tintcb.Initialize(dxri, sizeof(ObjectTintCB));
    tintcbMapped = tintcb.Map();
	tintCb.tint = glm::vec3(0.0f);
	memcpy(tintcbMapped, &tintCb, sizeof(ObjectTintCB));

}

glm::mat4 Object::GetModelMatrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, location);
	model = glm::scale(model, scale);
	return model;
}


void Object::Draw(ID3D12GraphicsCommandList* cmd, Material* material, ConstantBuffer* sceneBuffer)
{
    material->pipeline->SetPipelineState(cmd);
	material->pipeline->BindConstantBuffer("scene", sceneBuffer, cmd);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, location);
	model = glm::scale(model, scale);

	objCb.Model = model;
	memcpy(cbMapped, &objCb, sizeof(ObjectCB));

	if (selected)
		tintCb.tint = glm::vec3(0.3, 0, 0);
	else
		tintCb.tint = glm::vec3(0.0, 0, 0);

	memcpy(tintcbMapped, &tintCb, sizeof(ObjectTintCB));

	material->pipeline->BindConstantBuffer("object", &cb, cmd);
	material->pipeline->BindConstantBuffer("objectTint", &tintcb, cmd);
	cmd->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
	//commandList->IASetIndexBuffer(&indexBufferView);
	cmd->DrawInstanced(mesh._vertices.size(), 1, 0, 0);
}
