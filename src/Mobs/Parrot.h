#pragma once

#include "PassiveMonster.h"
#include "../UUID.h"

class cParrot :
	public cPassiveMonster
{
	using Super = cPassiveMonster;

public:
	cParrot();

	CLASS_PROTODEF(cParrot)

	virtual void Tick(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void OnRightClicked(cPlayer & a_Player) override;
	virtual bool DoTakeDamage(TakeDamageInfo & a_TDI) override;

	// Get functions
	bool IsSitting(void) const { return m_IsSitting; }
	bool IsTame(void) const { return m_IsTame; }
	cUUID GetOwnerUUID(void) const { return m_OwnerUUID; }
	int GetVariant(void) const { return m_Variant; }

	// Set functions
	void SetIsSitting(bool a_IsSitting) { m_IsSitting = a_IsSitting; }
	void SetIsTame(bool a_IsTame) { m_IsTame = a_IsTame; }
	void SetOwner(const AString & a_NewOwnerName, const cUUID & a_NewOwnerUUID)
	{
		m_OwnerName = a_NewOwnerName;
		m_OwnerUUID = a_NewOwnerUUID;
	}
	void SetVariant(int a_Variant) { m_Variant = a_Variant; }

protected:
	virtual void InStateIdle(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateChasing(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;
	virtual void InStateEscaping(std::chrono::milliseconds a_Dt, cChunk & a_Chunk) override;

	void CheckForDancing(cChunk & a_Chunk);

	bool m_IsSitting = false;
	bool m_IsTame = false;
	AString m_OwnerName;
	cUUID m_OwnerUUID;
	int m_DanceTimer = 0;
	int m_Variant = 0; // 0=Red, 1=Blue, 2=Green, 3=Cyan, 4=Gray
};