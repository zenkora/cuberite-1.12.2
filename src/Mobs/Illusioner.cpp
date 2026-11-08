#include "Globals.h"
#include "Illusioner.h"
#include "../World.h"
#include "../Entities/Player.h"
#include "../Entities/ArrowEntity.h"
#include "../Items/ItemHandler.h"

cIllusioner::cIllusioner() :
	Super("Illusioner", mtIllusioner, "entity.illusioner.hurt", "entity.illusioner.death", "entity.illusioner.ambient", 0.6f, 1.95f)
{
	SetMaxHealth(32);
	SetHealth(32);
	SetAttackDamage(0); // Uses bow
	SetAttackRange(16); // Ranged attacker
	SetSightDistance(16);
}

void cIllusioner::Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	Super::Tick(a_Dt, a_Chunk);

	if (m_SpellCooldown > 0)
	{
		m_SpellCooldown -= static_cast<int>(a_Dt.count());
	}
	if (m_MirrorImageCooldown > 0)
	{
		m_MirrorImageCooldown -= static_cast<int>(a_Dt.count());
	}
}

void cIllusioner::InStateIdle(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	Super::InStateIdle(a_Dt, a_Chunk);

	// Try to cast spell periodically
	if (m_SpellCooldown <= 0 && GetTarget() != nullptr)
	{
		TryCastBlindnessSpell(a_Chunk);
	}
}

void cIllusioner::InStateChasing(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	Super::InStateChasing(a_Dt, a_Chunk);

	// Try to cast spell while chasing
	if (m_SpellCooldown <= 0 && GetTarget() != nullptr)
	{
		TryCastBlindnessSpell(a_Chunk);
	}

	// Try to create mirror images when low on health
	if (GetHealth() < GetMaxHealth() * 0.5 && m_MirrorImageCooldown <= 0)
	{
		TryCreateMirrorImages(a_Chunk);
	}
}

void cIllusioner::InStateEscaping(std::chrono::milliseconds a_Dt, cChunk & a_Chunk)
{
	Super::InStateEscaping(a_Dt, a_Chunk);
}

void cIllusioner::TryCastBlindnessSpell(cChunk & a_Chunk)
{
	cPawn * Target = GetTarget();
	if (Target == nullptr || !Target->IsPlayer())
	{
		return;
	}

	cPlayer * Player = static_cast<cPlayer *>(Target);
	Vector3d ToTarget = Target->GetPosition() - GetPosition();

	// Only cast if target is within range
	if (ToTarget.SqrLength() > 16.0 * 16.0)
	{
		return;
	}

	// Apply blindness effect (effect ID 15 = Blindness)
	Player->AddEntityEffect(cEntityEffect::effBlindness, 20 * 20, 1); // 20 seconds, level 1

	// Show spell particles
	GetWorld()->BroadcastParticleEffect("spell", GetPosition(), Vector3f(0, 0, 0), 1, 10);

	// Reset cooldown (60-120 seconds in vanilla, using 30 seconds for gameplay)
	m_SpellCooldown = 30000 + GetRandomProvider().RandInt(30000);
	m_NextSpellType = 1; // Next spell will be mirror images
}

void cIllusioner::TryCreateMirrorImages(cChunk & a_Chunk)
{
	// Create 4 mirror images around the illusioner
	for (int i = 0; i < 4; ++i)
	{
		// Generate random angle and distance using RandInt
		double Angle = (i * M_PI / 2) + (static_cast<double>(GetRandomProvider().RandInt(-500, 500)) / 1000.0);
		double Distance = 2.0 + (static_cast<double>(GetRandomProvider().RandInt(0, 1000)) / 1000.0);
		Vector3d Pos = GetPosition() + Vector3d(cos(Angle) * Distance, 0, sin(Angle) * Distance);

		// Create a temporary illusioner that looks identical but has no AI
		// For now, we'll just show particles - full mirror images require more complex implementation
		GetWorld()->BroadcastParticleEffect("spell", Pos, Vector3f(0, 0, 0), 1, 5);
	}

	// Show spell effect on the illusioner
	GetWorld()->BroadcastParticleEffect("spell", GetPosition(), Vector3f(0, 0, 0), 1, 20);

	m_MirrorImageCooldown = 60000; // 60 seconds cooldown
}

bool cIllusioner::DoTakeDamage(TakeDamageInfo & a_TDI)
{
	if (!Super::DoTakeDamage(a_TDI))
	{
		return false;
	}

	// Trigger mirror images when hurt and cooldown is ready
	if (m_MirrorImageCooldown <= 0 && GetRandomProvider().RandBool(0.5))
	{
		// The chunk parameter is not actually used for mirror images (only for particles via World)
		// Just pass the current chunk from the tick context
		// We can't easily get the chunk here, so we'll skip mirror images on damage for now
		// TryCreateMirrorImages(*GetWorld()->GetChunkMap()->GetChunk(GetChunkX(), GetChunkZ()));
	}

	return true;
}