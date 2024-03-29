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
	// TransparentBlt�� �»�ܺ��� �׸��µ� ��ǥ�� �߾��� �ǵ��� ����
	pos -= size * 0.5f;

	::TransparentBlt(hdc,
		// �̹��� ��� ��ġ 
		(int32)pos.X,
		(int32)pos.Y,
		// ����� �̹����� ũ��
		(int32)size.X,
		(int32)size.Y,
		// �̹����� �ڵ�
		_texture->GetDC(),
		// �̹������� ������ �̹����� ��������
		0,
		0,
		// ������ �̹����� ũ��
		(int32)size.X,
		(int32)size.Y,
		_texture->GetTransparent());
}