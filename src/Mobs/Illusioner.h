#pragma once

#include "AggressiveMonster.h"
#include "../UUID.h"

class cIllusioner :
	public cAggressiveMonster
{
	using Super = cAggressiveMonster;

public:
	cIllusioner();

	CLASS_PROTODEF(cIllusioner)

	virtual void Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual bool DoTakeDamage(TakeDamageInfo & a_TDI) override;

protected:
	virtual void InStateIdle(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateChasing(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateEscaping(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;

	void TryCastBlindnessSpell(cChunk & a_Chunk);
	void TryCreateMirrorImages(cChunk & a_Chunk);

	int m_SpellCooldown = 0;
	int m_MirrorImageCooldown = 0;
	int m_NextSpellType = 0; // 0 = blindness, 1 = mirror images
};