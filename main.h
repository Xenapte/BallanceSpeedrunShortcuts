#pragma once

#include <BML/BMLAll.h>

extern "C" {
	__declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class SpeedrunShortcuts : public IMod {
	CKDataArray* current_level;
	InputHook* input_manager = m_bml->GetInputManager();
  CK_ID esc_event, restart_level;
public:
	SpeedrunShortcuts(IBML* bml) : IMod(bml) {}

	virtual CKSTRING GetID() override { return "SpeedrunShortcuts"; }
	virtual CKSTRING GetVersion() override { return "0.0.2"; }
	virtual CKSTRING GetName() override { return "Speedrun Shortcuts"; }
	virtual CKSTRING GetAuthor() override { return "BallanceBug"; }
	virtual CKSTRING GetDescription() override { return "Provides keyboard shortcuts for speedrunning."; }
	DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnLoadScript(CKSTRING filename, CKBehavior* script) override;

  void ResetBall(CKContext* ctx);
};