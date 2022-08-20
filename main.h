#pragma once

#include <BMLPlus/BMLAll.h>

extern "C" {
  __declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

typedef const char* C_CKSTRING;

class SpeedrunShortcuts : public IMod {
  CKDataArray* current_level;
  InputHook* input_manager = m_bml->GetInputManager();
  CK_ID esc_event, restart_level;
public:
  SpeedrunShortcuts(IBML* bml) : IMod(bml) {}

  virtual C_CKSTRING GetID() override { return "SpeedrunShortcuts"; }
  virtual C_CKSTRING GetVersion() override { return "0.0.2"; }
  virtual C_CKSTRING GetName() override { return "Speedrun Shortcuts"; }
  virtual C_CKSTRING GetAuthor() override { return "BallanceBug"; }
  virtual C_CKSTRING GetDescription() override { return "Provides keyboard shortcuts for speedrunning."; }
  DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnLoadScript(C_CKSTRING filename, CKBehavior* script) override;

  void ResetBall(CKContext* ctx);
};