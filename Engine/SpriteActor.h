#pragma once
#include "Actor.h"

class Sprite;

class SpriteActor : public Actor
{
	using Super = Actor;
public:
	SpriteActor();
	virtual ~SpriteActor() override;

	virtual void Init() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Render(HDC hdc) override;

public:
	void SetSprite(std::shared_ptr<Sprite> sprite);

	
protected:
	std::shared_ptr<Sprite> _sprite;
};
