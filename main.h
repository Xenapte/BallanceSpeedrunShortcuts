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

  const std::string version = []() -> decltype(version) {
    char version_str[32];
    std::snprintf(version_str, sizeof(version_str), "0.2.2_bmmo-%u.%u.%u",
                  bmmo::current_version.major, bmmo::current_version.minor, bmmo::current_version.subminor);
    return version_str;
  }();

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
  virtual BMMO_CKSTRING GetVersion() override { return version.c_str(); }
  virtual BMMO_CKSTRING GetName() override { return "Speedrun Shortcuts"; }
  virtual BMMO_CKSTRING GetAuthor() override { return "BallanceBug"; }
  virtual BMMO_CKSTRING GetDescription() override { return "Keyboard shortcuts useful for speedrunning."; }
  DECLARE_BML_VERSION;
  
  void OnProcess() override;
  void OnPostStartMenu() override;
  void OnLoadScript(BMMO_CKSTRING filename, CKBehavior* script) override;
  void OnBallNavInactive() override;
  void OnBallNavActive() override;
  void OnLoad() override;
  void OnModifyConfig(BMMO_CKSTRING category, BMMO_CKSTRING key, IProperty* prop) override;
};
