#pragma once

#include <BML/BMLAll.h>
#include "exported_client.h"

extern "C" {
  __declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

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

  virtual CKSTRING GetID() override { return "SpeedrunShortcuts"; }
  virtual CKSTRING GetVersion() override { return "0.1.0"; }
  virtual CKSTRING GetName() override { return "Speedrun Shortcuts"; }
  virtual CKSTRING GetAuthor() override { return "BallanceBug"; }
  virtual CKSTRING GetDescription() override { return "Provides keyboard shortcuts for speedrunning."; }
  DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnPostStartMenu() override;
  void OnLoadScript(CKSTRING filename, CKBehavior* script) override;
  void OnBallNavInactive() override;
  void OnBallNavActive() override;
  void OnLoad() override;
  void OnModifyConfig(CKSTRING category, CKSTRING key, IProperty* prop) override;

  // void ResetBall(CKContext* ctx);
};