#include "pch.h"
#include "Enemy.h"
#include "SquareComponent.h"
#include "AssetManager.h"
#include "Flipbook.h"
#include "World.h"
#include "Level.h"
#include "AlgorithmUtils.h"
#include "Tilemap.h"
#include "TilemapActor.h"

Enemy::Enemy() {

	SetEnemyAnimation();

	_square = std::make_shared<SquareComponent>();
	_square->SetSize({ 50.f, 50.f });
	AddComponent(_square);
}

Enemy::~Enemy() {}

void Enemy::Init()
{
	Super::Init();

	_square->_beginOverlapDelegate.BindDelegate(this, &Enemy::BeginOverlapFunction);
}

void Enemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetState() == ActionState::AS_Idle)
		return;
	
	Chase();
	EnemyMove(DeltaTime);
}

void Enemy::Render(HDC hdc)
{
	Super::Render(hdc);
}

void Enemy::BeginOverlapFunction(std::weak_ptr<Collider> comp, std::weak_ptr<Actor> other, std::weak_ptr<Collider> otherComp)
{
	SetState(ActionState::AS_Idle);
}

void Enemy::EndOverlapFunction(std::weak_ptr<Collider> comp, std::weak_ptr<Actor> other, std::weak_ptr<Collider> otherComp)
{
}

void Enemy::OnDamaged(std::weak_ptr<Actor> attacker)
{
}

void Enemy::UpdateAnimation()
{
	SetFlipbook(_move[_dir]);
}

void Enemy::SetEnemyAnimation()
{
	GET_SINGLE(AssetManager)->LoadTexture(L"Snake", L"Sprite\\Monster\\Snake.bmp", RGB(128, 128, 128));

	// Move
	if (std::shared_ptr<Texture> texture = GET_SINGLE(AssetManager)->GetTexture(L"Snake")) {
		_move[DIR_Up] = GET_SINGLE(AssetManager)->CreateFlipbook(L"FB_SnakeUp");
		_move[DIR_Up]->SetInfo({ texture, L"FB_SnakeUp", {100, 100}, 0, 3, 3, 0.5f });
	}
	if (std::shared_ptr<Texture> texture = GET_SINGLE(AssetManager)->GetTexture(L"Snake")) {
		_move[DIR_Down] = GET_SINGLE(AssetManager)->CreateFlipbook(L"FB_SnakeDown");
		_move[DIR_Down]->SetInfo({ texture, L"FB_SnakeDown", {100, 100}, 0, 3, 0, 0.5f });
	}
	if (std::shared_ptr<Texture> texture = GET_SINGLE(AssetManager)->GetTexture(L"Snake")) {
		_move[DIR_Left] = GET_SINGLE(AssetManager)->CreateFlipbook(L"FB_SnakeLeft");
		_move[DIR_Left]->SetInfo({ texture, L"FB_SnakeLeft", {100, 100}, 0, 3, 2, 0.5f });
	}
	if (std::shared_ptr<Texture> texture = GET_SINGLE(AssetManager)->GetTexture(L"Snake")) {
		_move[DIR_Right] = GET_SINGLE(AssetManager)->CreateFlipbook(L"FB_SnakeRight");
		_move[DIR_Right]->SetInfo({ texture, L"FB_SnakeRight", {100, 100}, 0, 3, 1, 0.5f });
	}
}

void Enemy::EnemyMove(float DeltaTime)
{
	Vector2D dir = (_destPos - GetPos());
	if (dir.Length() < 5.f) // 도착지점에 충분히 가까우면
	{
		SetSpeed(Vector2D::Zero);
	}
	// 도착지까지 부드럽게 움직이도록 보정
	else {
		bool horizontal = std::abs(dir.X) > abs(dir.Y);
		if (horizontal)
			SetDir(dir.X < 0 ? DIR_Left : DIR_Right);
		else
			SetDir(dir.Y < 0 ? DIR_Up : DIR_Down);

		Vector2D move = GetPos() + GetDirVector2D(GetDir()) * GetSpeed() * DeltaTime;
		SetPos(GetPos() + GetDirVector2D(GetDir()) * GetSpeed() * DeltaTime);
	}
}

void Enemy::Chase()
{
	std::shared_ptr<Actor> target = _target.lock();
	if (target == nullptr)
		target = World::GetCurrentLevel()->FindClosestTarget(GetPos());


	if (target) {
		Vector2D dir = target->GetPos() - GetPos();
		float dist = std::abs(dir.X) + std::abs(dir.Y); // 대각선은 신경쓰지 않고 상하좌우만 계산
		if (dist == 1.f) // 바로 앞이라면
		{
			SetDir(GetLookAtDir(target->GetPos()));
			SetState(ActionState::AS_Attack);
			SetWaitSeconds(GetMaxWaitSeconds());
		}
		else {
			// 목표까지 길을 찾고 1칸 이동하고 다시 찾는 걸 반복
			// (계산량에 부담은 되나 더 자연스럽다. 부담되면 일정 시간에 찾도록 변경)
			std::vector<Vector2D> path;
			std::shared_ptr<TilemapActor> tmActor = Level::GetCurrentTilemapActor();
			if (tmActor)
			{
				Vector2D cellPos = tmActor->ConvertToTilemapPos(GetPos());
				Vector2D targetCellPos = tmActor->ConvertToTilemapPos(target->GetPos());
			
				if (AlgorithmUtils::FindPathAStar(cellPos, targetCellPos, OUT path)) {
					// index 0은 현재 위치
					if (path.size() > 1)
					{
						Vector2D nextPos = path[1];
						if (tmActor->GetTilemap()->CanGo(nextPos))
						{
							SetDestPos(nextPos * tmActor->GetTilemap()->GetTileSize());
							SetState(ActionState::AS_Move);
						}
					}
					else {
						SetDestPos(GetPos());
					}
				}
			}
		}
	}
}

Dir Enemy::GetLookAtDir(Vector2D pos)
{
	Vector2D dir = pos - GetPos();
	if (dir.X > 0)
		return DIR_Right;	
	else if (dir.X < 0)
		return DIR_Left;
	else if (dir.Y > 0)
		return DIR_Down;
	else
		return DIR_Up;
}

void Enemy::FindPath(Vector2D pos, Vector2D targetPos, std::vector<Vector2D>& path)
{
}

Vector2D Enemy::GetDirVector2D(Dir dir)
{
	// enum Dir과 순서를 맞춰준다. (상하좌우)
	static Vector2D nextDir[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} }; // 다음 방향
	return nextDir[dir];
}

void Enemy::SetDir(Dir dir)
{
	if (_dir == dir)
		return;

	_dir = dir;
	// 방향이 바뀌면 Animation도 상태에 맞게 바꿔준다.
	UpdateAnimation();
}

void Enemy::SetState(ActionState state)
{
	if (_state == state)
		return;

	_state = state;
	// 상태가 바뀌면 Animation도 상태에 맞게 바꿔준다.
	UpdateAnimation();
}
