#include "Globals.h"
#include "Parrot.h"
#include "../World.h"
#include "../Entities/Player.h"
#include "../Items/ItemHandler.h"
#include "../Chunk.h"
#include "../BlockEntities/JukeboxEntity.h"

cParrot::cParrot() :
	Super("Parrot", mtParrot, "entity.parrot.hurt", "entity.parrot.death", "entity.parrot.ambient", 0.5f, 0.9f)
{
	SetMaxHealth(6);
	SetHealth(6);
	m_Variant = GetRandomProvider().RandInt(0, 4); // Random variant on spawn
}

void cParrot::Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	Super::Tick(a_Dt, a_Chunk);

	if (m_DanceTimer > 0)
	{
		m_DanceTimer -= static_cast<int>(a_Dt.count());
	}

	CheckForDancing(a_Chunk);
}

void cParrot::InStateIdle(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	// Random movement
	if (GetRandomProvider().RandBool(1 / 200.0))
	{
		Vector3i TargetPos = GetPosition().addedX(GetRandomProvider().RandInt(-10, 10)).addedZ(GetRandomProvider().RandInt(-10, 10));
		MoveToPosition(TargetPos);
	}

	Super::InStateIdle(a_Dt, a_Chunk);
}

void cParrot::InStateChasing(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	// Parrots don't chase
	m_EMState = IDLE;
	Super::InStateIdle(a_Dt, a_Chunk);
}

void cParrot::InStateEscaping(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	// Fly away from danger
	if (GetTarget() != nullptr)
	{
		Vector3d Dir = GetPosition() - GetTarget()->GetPosition();
		Dir.Normalize();
		Vector3d FleePos = GetPosition() + Dir * 10;
		MoveToPosition(FleePos);
	}
	Super::InStateEscaping(a_Dt, a_Chunk);
}

void cParrot::CheckForDancing(cChunk & a_Chunk)
{
	// Check for nearby jukebox playing a record
	Vector3i Pos = GetPosition();
	int ChunkX = Pos.x >> 4;
	int ChunkZ = Pos.z >> 4;

	// Check the current chunk and neighboring chunks for jukeboxes
	for (int cx = ChunkX - 1; cx <= ChunkX + 1; ++cx)
	{
		for (int cz = ChunkZ - 1; cz <= ChunkZ + 1; ++cz)
		{
			GetWorld()->ForEachBlockEntityInChunk(cx, cz, [&](cBlockEntity & a_BlockEntity) -> bool
			{
				if (a_BlockEntity.GetBlockType() == E_BLOCK_JUKEBOX)
				{
					cJukeboxEntity * Jukebox = dynamic_cast<cJukeboxEntity *>(&a_BlockEntity);
					if ((Jukebox != nullptr) && Jukebox->IsPlayingRecord())
					{
						Vector3d JukeboxPos = a_BlockEntity.GetPos();
						if ((JukeboxPos - GetPosition()).SqrLength() < 9.0) // Within 3 blocks
						{
							if (m_DanceTimer <= 0)
							{
								m_DanceTimer = 20000; // Dance for 20 seconds
								GetWorld()->BroadcastEntityMetadata(*this);
							}
							return false; // Stop searching
						}
					}
				}
				return true; // Continue searching
			});
		}
	}
}

void cParrot::OnRightClicked(cPlayer & a_Player)
{
	if (m_IsTame && (m_OwnerUUID == a_Player.GetUUID()))
	{
		// Owner right-clicks: sit/stand toggle
		if (IsSitting())
		{
			SetIsSitting(false);
		}
		else
		{
			SetIsSitting(true);
		}
		GetWorld()->BroadcastEntityMetadata(*this);
		return;
	}

	// Try to tame with seeds
	const cItem & EquippedItem = a_Player.GetEquippedItem();
	if (EquippedItem.m_ItemType == E_ITEM_SEEDS || EquippedItem.m_ItemType == E_ITEM_PUMPKIN_SEEDS ||
		EquippedItem.m_ItemType == E_ITEM_MELON_SEEDS || EquippedItem.m_ItemType == E_ITEM_BEETROOT_SEEDS)
	{
		if (!m_IsTame)
		{
			if (GetRandomProvider().RandBool(0.33)) // 33% chance to tame
			{
				m_IsTame = true;
				m_OwnerName = a_Player.GetName();
				m_OwnerUUID = a_Player.GetUUID();
				SetIsSitting(true);
				GetWorld()->BroadcastEntityMetadata(*this);
				a_Player.GetInventory().RemoveOneEquippedItem();
				return;
			}
			else
			{
				// Show taming fail particles
				GetWorld()->BroadcastParticleEffect("heart", GetPosition(), Vector3f(0, 0, 0), 0, 0);
				a_Player.GetInventory().RemoveOneEquippedItem();
				return;
			}
		}
	}

	Super::OnRightClicked(a_Player);
}

bool cParrot::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	if (!Super::DoTakeDamage(a_TDI))
	{
		return false;
	}

	// If tamed and owner attacks, don't retaliate
	if (m_IsTame && (a_TDI.Attacker != nullptr) && a_TDI.Attacker->IsPlayer())
	{
		auto * Player = static_cast<cPlayer *>(a_TDI.Attacker);
		if (Player->GetUUID() == m_OwnerUUID)
		{
			return true;
		}
	}

	// Escape from attacker
	if (a_TDI.Attacker != nullptr)
	{
		if (a_TDI.Attacker->IsPawn())
		{
			SetTarget(static_cast<cPawn *>(a_TDI.Attacker));
			m_EMState = ESCAPING;
		}
	}
	return true;
}