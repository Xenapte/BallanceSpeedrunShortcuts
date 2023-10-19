#pragma once

#include "bml_includes.h"
#include "exported_client.h"

extern "C" {
  __declspec(dllexport) IMod* BMLEntry(IBML* bml);
}

class SpeedrunShortcuts : public IMod {
  CKDataArray* current_level{}, * checkpoints{}, * reset_points{}, * ingame_param{};
  CKParameter* current_sector{};
  InputHook* input_manager = m_bml->GetInputManager();
  CK_ID esc_event{}, restart_level_action{};
  CKBehavior* dynamic_pos{}, * phy_new_ball{};
  bool init = false, nav_inactive = true;
  constexpr static int PROP_LENGTH = 5;
  // cheat, spawn, reset, sector, modifier
  IProperty* props[PROP_LENGTH]{};
  CKKEYBOARD keys[PROP_LENGTH]{};
  bmmo::exported::client* mmo_client = nullptr;
  IProperty* prop_mmo_strict_mode{};
  bool mmo_strict_mode = false;

  void update_key_property(int index);

  void extract_scripts();

  void toggle_cheat();
  void set_spawn();
  void restart_level();
  void set_sector(int sector_input);

  void reset_ball();

public:
  SpeedrunShortcuts(IBML* bml) : IMod(bml) {}

  virtual BMMO_CKSTRING GetID() override { return "SpeedrunShortcuts"; }
  virtual BMMO_CKSTRING GetVersion() override { return "0.2.1"; }
  virtual BMMO_CKSTRING GetName() override { return "Speedrun Shortcuts"; }
  virtual BMMO_CKSTRING GetAuthor() override { return "BallanceBug"; }
  virtual BMMO_CKSTRING GetDescription() override { return "Provides keyboard shortcuts for speedrunning."; }
  DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnPostStartMenu() override;
  void OnLoadScript(BMMO_CKSTRING filename, CKBehavior* script) override;
  void OnBallNavInactive() override;
  void OnBallNavActive() override;
  void OnLoad() override;
  void OnModifyConfig(BMMO_CKSTRING category, BMMO_CKSTRING key, IProperty* prop) override;
};
