#include "pch.h"
#include "TextureActor.h"
#include "AssetManager.h"
#include "Texture.h"
#include "WinUtils.h"

TextureActor::TextureActor()
{
}

TextureActor::~TextureActor()
{
}

void TextureActor::Init()
{
	Super::Init();
	bool load = GET_SINGLE(AssetManager)->LoadTexture(L"MP", L"Sprite\\UI\\Mp.bmp");
	if (load) 
		_texture = GET_SINGLE(AssetManager)->GetTexture(L"MP");
}

void TextureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void TextureActor::Render(HDC hdc)
{
	Super::Render(hdc);

	Vector2D pos = GetPos();
	Vector2D size = _texture->GetSize();
	// TransparentBlt는 좌상단부터 그리는데 좌표가 중앙이 되도록 보정
	pos -= size * 0.5f;

	::TransparentBlt(hdc,
		// 이미지 출력 위치 
		(int32)pos.X,
		(int32)pos.Y,
		// 출력할 이미지의 크기
		(int32)size.X,
		(int32)size.Y,
		// 이미지의 핸들
		_texture->GetDC(),
		// 이미지에서 가져올 이미지의 시작지점
		0,
		0,
		// 가져올 이미지의 크기
		(int32)size.X,
		(int32)size.Y,
		_texture->GetTransparent());
}
