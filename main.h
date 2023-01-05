#pragma once

#include <BMLPlus/BMLAll.h>
#include "exported_client.h"

extern "C" {
  __declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

typedef const char* C_CKSTRING;

class SpeedrunShortcuts : public IMod {
  CKDataArray* current_level{};
  InputHook* input_manager = m_bml->GetInputManager();
  CK_ID esc_event{}, restart_level{};
  bool init = false, nav_inactive = true;
  IProperty* props[3]{}; // cheat, spawn, reset
  CKKEYBOARD keys[3]{}; // cheat, spawn, reset
  bmmo::exported::client* mmo_client = nullptr;
  IProperty* prop_mmo_strict_mode{};
  bool mmo_strict_mode = false;

  void update_key_property(int index);

public:
  SpeedrunShortcuts(IBML* bml) : IMod(bml) {}

  virtual C_CKSTRING GetID() override { return "SpeedrunShortcuts"; }
  virtual C_CKSTRING GetVersion() override { return "0.1.0"; }
  virtual C_CKSTRING GetName() override { return "Speedrun Shortcuts"; }
  virtual C_CKSTRING GetAuthor() override { return "BallanceBug"; }
  virtual C_CKSTRING GetDescription() override { return "Provides keyboard shortcuts for speedrunning."; }
  DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnPostStartMenu() override;
  void OnLoadScript(C_CKSTRING filename, CKBehavior* script) override;
  void OnBallNavInactive() override;
  void OnBallNavActive() override;
  void OnLoad() override;
  void OnModifyConfig(C_CKSTRING category, C_CKSTRING key, IProperty* prop) override;

  // void ResetBall(CKContext* ctx);
};