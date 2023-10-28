#include "Object.h"

#include <glm/ext/matrix_transform.hpp>

void Object::Initialize()
{
    cb.Initialize(GDevice, sizeof(ObjectCB));
    cbMapped = cb.Map();
	objCb.Model = glm::mat4(1.0f);
	memcpy(cbMapped, &objCb, sizeof(ObjectCB));
}

void Object::Draw(ID3D12GraphicsCommandList* cmd, Pipeline& pipeline)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, location);
	model = glm::scale(model, scale);

	objCb.Model = model;
	memcpy(cbMapped, &objCb, sizeof(ObjectCB));

	pipeline.BindConstantBuffer("object", &cb, cmd);
	cmd->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
	//commandList->IASetIndexBuffer(&indexBufferView);
	cmd->DrawInstanced(mesh._vertices.size(), 1, 0, 0);
}
