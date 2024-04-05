#include "pch.h"
#include "CameraComponent.h"
#include "Actor.h"
#include "World.h"

CameraComponent::CameraComponent()
{
}

CameraComponent::~CameraComponent()
{
}

void CameraComponent::Init()
{
}

void CameraComponent::Tick(float DeltaTime)
{
	std::shared_ptr<Actor> owner = GetOwner();
	if (owner == nullptr)
		return;

	Vector2D pos = owner->GetPos();

	// TODO: World�� ����� A
	// background map size���� clamp (����)
	pos.X = std::clamp(pos.X, 400.f, 3024.f - 400.f);
	pos.Y = std::clamp(pos.Y, 300.f, 2064.f - 300.f);

	World::SetCameraPos(pos);
}

void CameraComponent::Render(HDC hdc)
{
}