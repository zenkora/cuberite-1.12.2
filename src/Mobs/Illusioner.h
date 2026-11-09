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

	// Getters for NBT saving
	int GetSpellCooldown(void) const { return m_SpellCooldown; }
	int GetMirrorImageCooldown(void) const { return m_MirrorImageCooldown; }
	int GetNextSpellType(void) const { return m_NextSpellType; }
	bool IsMirrorImage(void) const { return m_IsMirrorImage; }

	// Setters for NBT loading
	void SetSpellCooldown(int a_Cooldown) { m_SpellCooldown = a_Cooldown; }
	void SetMirrorImageCooldown(int a_Cooldown) { m_MirrorImageCooldown = a_Cooldown; }
	void SetNextSpellType(int a_Type) { m_NextSpellType = a_Type; }
	void SetIsMirrorImage(bool a_IsMirrorImage) { m_IsMirrorImage = a_IsMirrorImage; }

protected:
	virtual void InStateIdle(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateChasing(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateEscaping(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;

	void TryCastBlindnessSpell(cChunk & a_Chunk);
	void TryCreateMirrorImages(cChunk & a_Chunk);

	int m_SpellCooldown = 0;
	int m_MirrorImageCooldown = 0;
	int m_NextSpellType = 0; // 0 = blindness, 1 = mirror images
	bool m_IsMirrorImage = false;
};