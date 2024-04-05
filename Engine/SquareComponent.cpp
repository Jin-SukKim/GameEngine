#include "pch.h"
#include "SquareComponent.h"
#include "Actor.h"
#include "World.h"
#include "Engine.h"
#include "WinUtils.h"
#include "CircleComponent.h"

SquareComponent::SquareComponent() : Collider(ColliderType::CT_Square) {}

SquareComponent::~SquareComponent() {}

void SquareComponent::Init()
{
	Super::Init();
}

void SquareComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void SquareComponent::Render(HDC hdc)
{
	Super::Render(hdc);

	// ����
	const Vector2D camPos = World::GetCameraPos();
	Vector2D pos = GetOwner()->GetPos();
	pos -= camPos - Engine::GetScreenSize() * 0.5f;

	HBRUSH myBrush = (HBRUSH)::GetStockObject(NULL_BRUSH); // �׸� �� �� �����ؼ� ��������
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, myBrush);
	WinUtils::DrawRect(hdc, pos, static_cast<int32>(_size.X), static_cast<int32>(_size.Y));
	::SelectObject(hdc, oldBrush);
	::DeleteObject(myBrush);
}

bool SquareComponent::CheckCollision(std::weak_ptr<Collider> other)
{
	if (Super::CheckCollision(other) == false) // ���� �浹�� �� �ִ��� Ȯ��
		return false;

	std::shared_ptr<Collider> collider = other.lock();
	if (collider == nullptr)
		return false;

	switch (collider->GetColliderType())
	{
	case ColliderType::CT_Square:
		return CheckCollisionSquareToSqaure(std::dynamic_pointer_cast<SquareComponent>(shared_from_this()), std::dynamic_pointer_cast<SquareComponent>(collider));
	case ColliderType::CT_Circle:
		return CheckCollisionCircleToSquare(std::dynamic_pointer_cast<CircleComponent>(collider), std::dynamic_pointer_cast<SquareComponent>(shared_from_this()));
	default:
		break;
	}
	return false;
}

RECT SquareComponent::GetRect()
{
	Vector2D pos = GetOwner()->GetPos();
	Vector2D halfSize = GetSize() * 0.5f;

	RECT rect = {
		static_cast<int32>(pos.X - halfSize.X),
		static_cast<int32>(pos.Y - halfSize.Y),
		static_cast<int32>(pos.X + halfSize.X),
		static_cast<int32>(pos.Y + halfSize.Y)
	};

	return rect;
}