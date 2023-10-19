#include "main.h"

IMod* BMLEntry(IBML* bml) {
  return new SpeedrunShortcuts(bml);
}

void SpeedrunShortcuts::OnLoadScript(BMMO_CKSTRING filename, CKBehavior* script) {
  if (strcmp(script->GetName(), "Gameplay_Events") == 0) {
    auto* esc = ScriptHelper::FindFirstBB(script, "Key Event");
    esc_event = CKOBJID(esc->GetOutput(0));
  }
  else if (strcmp(script->GetName(), "Menu_Pause") == 0) {
    restart_level_action = CKOBJID(ScriptHelper::FindFirstBB(script, "Restart Level"));
  }
  else if (strcmp(script->GetName(), "Gameplay_Ingame") == 0) {
    CKBehavior* ballMgr = ScriptHelper::FindFirstBB(script, "BallManager");
    CKBehavior* newBall = ScriptHelper::FindFirstBB(ballMgr, "New Ball");
    dynamic_pos = ScriptHelper::FindNextBB(script, ballMgr, "TT Set Dynamic Position");
    phy_new_ball = ScriptHelper::FindFirstBB(newBall, "physicalize new Ball");
  }
}

void SpeedrunShortcuts::OnBallNavInactive() { nav_inactive = true; }
void SpeedrunShortcuts::OnBallNavActive() { nav_inactive = false; }

void SpeedrunShortcuts::OnLoad() {
  GetConfig()->SetCategoryComment("Keys", "Set the keys used to trigger actions. Must be used with the Alt key.");
  auto tmp_prop = GetConfig()->GetProperty("Keys", "CheatToggle");
  tmp_prop->SetDefaultKey(CKKEY_C);
  tmp_prop->SetComment("The key for toggling Cheat Mode.");
  props[0] = tmp_prop;
  tmp_prop = GetConfig()->GetProperty("Keys", "SetSpawn");
  tmp_prop->SetDefaultKey(CKKEY_S);
  tmp_prop->SetComment("The key for setting spawn points (under Cheat Mode).");
  props[1] = tmp_prop;
  tmp_prop = GetConfig()->GetProperty("Keys", "Restart");
  tmp_prop->SetDefaultKey(CKKEY_E);
  tmp_prop->SetComment("The key for restarting the current level.");
  props[2] = tmp_prop;
  tmp_prop = GetConfig()->GetProperty("Keys", "SetSector");
  tmp_prop->SetDefaultKey(CKKEY_W);
  tmp_prop->SetComment("The key for setting the current sector (with a number key).");
  props[3] = tmp_prop;
  tmp_prop = GetConfig()->GetProperty("Keys", "ModifierKey");
  tmp_prop->SetDefaultKey(CKKEY_LMENU);
  tmp_prop->SetComment("The key to use together with keys above to trigger different functions.");
  props[PROP_LENGTH - 1] = tmp_prop;
  for (int i = 0; i < PROP_LENGTH; i ++) {
    update_key_property(i);
  }
  GetConfig()->SetCategoryComment("BMMO", "BMMO-related stuff for cheat detection.");
  prop_mmo_strict_mode = GetConfig()->GetProperty("BMMO", "StrictMode");
  prop_mmo_strict_mode->SetDefaultBoolean(false);
  prop_mmo_strict_mode->SetComment("Completely block restarting during inappropriate conditions (for example, when on a Trafo). Otherwise only a warning will be issued.");
  mmo_strict_mode = prop_mmo_strict_mode->GetBoolean();
}

void SpeedrunShortcuts::OnModifyConfig(BMMO_CKSTRING category, BMMO_CKSTRING key, IProperty* prop) {
  if (strcmp("Keys", category) == 0) {
    for (int i = 0; i < PROP_LENGTH; i++) {
      if (prop == props[i])
        update_key_property(i);
    }
  }
  if (prop == prop_mmo_strict_mode)
    mmo_strict_mode = prop_mmo_strict_mode->GetBoolean();
}

void SpeedrunShortcuts::update_key_property(int index) {
  keys[index] = props[index]->GetKey();
}

void SpeedrunShortcuts::extract_scripts() {
  current_level = m_bml->GetArrayByName("CurrentLevel");
  checkpoints = m_bml->GetArrayByName("Checkpoints");
  reset_points = m_bml->GetArrayByName("ResetPoints");
  ingame_param = m_bml->GetArrayByName("IngameParameter");
  CKBehavior* events = m_bml->GetScriptByName("Gameplay_Events");
  CKBehavior* id = ScriptHelper::FindNextBB(events, events->GetInput(0));
  current_sector = id->GetOutputParameter(0)->GetDestination(0);;
}

void SpeedrunShortcuts::toggle_cheat() {
  m_bml->EnableCheat(!m_bml->IsCheatEnabled());
  m_bml->SendIngameMessage((std::string("Cheat mode ").append(m_bml->IsCheatEnabled() ? "enabled" : "disabled")).c_str());
}

void SpeedrunShortcuts::set_spawn() {
  if (!m_bml->IsCheatEnabled() || !m_bml->IsIngame())
    return;
  if (!current_level) 
    extract_scripts();
    
  if (m_bml->IsIngame() && current_level) {
    CK3dEntity* camRef = m_bml->Get3dEntityByName("Cam_OrientRef");
    VxMatrix matrix = camRef->GetWorldMatrix();
    for (int i = 0;
#ifdef BMMO_USE_BML_PLUS
         i < 3;
#else
         i < 4;
#endif // BMMO_USE_BML_PLUS
         i++) {
      std::swap(matrix[0][i], matrix[2][i]);
      matrix[0][i] = -matrix[0][i];
    }
    current_level->SetElementValue(0, 3, &matrix);
    m_bml->SendIngameMessage(("Set Spawn Point to ("
                            + std::to_string(matrix[3][0]) + ", "
                            + std::to_string(matrix[3][1]) + ", "
                            + std::to_string(matrix[3][2]) + ")").c_str());
  }
}

void SpeedrunShortcuts::restart_level() {
  if (!m_bml->IsIngame())
    return;
  if (mmo_client) {
    if (mmo_client->connected() && !mmo_client->is_spectator() && nav_inactive && m_bml->IsPlaying()) {
      if (mmo_strict_mode) {
        m_bml->SendIngameMessage("Warning: you have connected to a BMMO server in Strict Mode and cannot reset inappropriately.");
        return;
      }
      else {
        bmmo::public_notification_msg msg{};
        msg.type = bmmo::public_notification_type::Warning;
        msg.text_content = mmo_client->get_own_id().second + " just restarted "
          + mmo_client->get_current_map().get_display_name() + " when their ball is not controllable.";
        msg.serialize();
        mmo_client->send(msg.raw.str().data(), msg.size(), k_nSteamNetworkingSend_Reliable);
      }
    }
  }
  auto* esc = static_cast<CKBehaviorIO*>(m_bml->GetCKContext()->GetObject(esc_event));
  esc->Activate();

  m_bml->AddTimer(CKDWORD(3), [this]() {
  // std::thread([this] {
    CKMessageManager* mm = m_bml->GetMessageManager();
    // std::this_thread::sleep_for(std::chrono::milliseconds(120));

    CKMessageType reset_level_msg = mm->AddMessageType("Reset Level");
    mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("Level", CKCID_BEOBJECT, nullptr)));
    mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("All_Balls", CKCID_BEOBJECT, nullptr)));

    auto* beh = static_cast<CKBehavior*>(m_bml->GetCKContext()->GetObject(restart_level_action));
    auto* output = beh->GetOutput(0);
    output->Activate();
  }); //.detach();
}

void SpeedrunShortcuts::set_sector(int sector_input) {
  if (!m_bml->IsCheatEnabled() || !m_bml->IsPlaying())
    return;

  if (!current_level)
    extract_scripts();

  int curSector = ScriptHelper::GetParamValue<int>(current_sector);
  int sector = std::clamp(sector_input, 1, checkpoints->GetRowCount() + 1);
  if (curSector != sector) {
    VxMatrix matrix;
    reset_points->GetElementValue(sector - 1, 0, &matrix);
    current_level->SetElementValue(0, 3, &matrix);

    ingame_param->SetElementValue(0, 1, &sector);
    ingame_param->SetElementValue(0, 2, &curSector);
    ScriptHelper::SetParamValue(current_sector, sector);

    m_bml->SendIngameMessage(("Changed to Sector " + std::to_string(sector)).c_str());

    CKContext* ctx = m_bml->GetCKContext();
    CKBehavior* sector_manager = m_bml->GetScriptByName("Gameplay_SectorManager");
    ctx->GetCurrentScene()->Activate(sector_manager, true);

    m_bml->AddTimerLoop(CKDWORD(1), [this, sector, sector_manager, ctx]() {
      if (sector_manager->IsActive())
        return true;

      m_bml->AddTimer(CKDWORD(2), [this, sector, ctx]() {
        CKBOOL active = false;
        current_level->SetElementValue(0, 4, &active);

        CK_ID flameId;
        checkpoints->GetElementValue(sector % 2, 1, &flameId);
        CK3dEntity* flame = static_cast<CK3dEntity*>(ctx->GetObject(flameId));
        ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);

        checkpoints->GetElementValue(sector - 1, 1, &flameId);
        flame = static_cast<CK3dEntity*>(ctx->GetObject(flameId));
        ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);

        if (sector > checkpoints->GetRowCount()) {
          CKMessageManager* mm = m_bml->GetMessageManager();
          CKMessageType msg = mm->AddMessageType("last Checkpoint reached");
          mm->SendMessageSingle(msg, m_bml->GetGroupByName("All_Sound"));

          reset_ball();
        }
        else {
          m_bml->AddTimer(CKDWORD(2), [this, sector, ctx, flame]() {
            VxMatrix matrix;
            checkpoints->GetElementValue(sector - 1, 0, &matrix);
            flame->SetWorldMatrix(matrix);
            CKBOOL active = true;
            current_level->SetElementValue(0, 4, &active);
            ctx->GetCurrentScene()->Activate(flame->GetScript(0), true);
            m_bml->Show(flame, CKSHOW, true);

            reset_ball();
          });
        }
      });
      return false;
    });
  }
}

void SpeedrunShortcuts::reset_ball() {
  CKContext* ctx = m_bml->GetCKContext();
  CKMessageManager* mm = m_bml->GetMessageManager();
  CKMessageType ballDeact = mm->AddMessageType("BallNav deactivate");

  mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Gameplay"));
  mm->SendMessageSingle(ballDeact, m_bml->GetGroupByName("All_Sound"));

  m_bml->AddTimer(CKDWORD(2), [this, ctx]() {
    CK3dEntity* curBall = static_cast<CK3dEntity*>(current_level->GetElementObject(0, 1));
    if (curBall) {
      ExecuteBB::Unphysicalize(curBall);

      dynamic_pos->ActivateInput(1);
      dynamic_pos->Activate();

      m_bml->AddTimer(CKDWORD(1), [this, curBall, ctx]() {
        VxMatrix matrix;
        current_level->GetElementValue(0, 3, &matrix);
        curBall->SetWorldMatrix(matrix);

        CK3dEntity* camMF = m_bml->Get3dEntityByName("Cam_MF");
        m_bml->RestoreIC(camMF, true);
        camMF->SetWorldMatrix(matrix);

        m_bml->AddTimer(CKDWORD(1), [this]() {
          dynamic_pos->ActivateInput(0);
          dynamic_pos->Activate();

          phy_new_ball->ActivateInput(0);
          phy_new_ball->Activate();
          phy_new_ball->GetParent()->Activate();
        });
      });
    }
  });
}

void SpeedrunShortcuts::OnProcess() {
  if (!input_manager->IsKeyDown(keys[PROP_LENGTH - 1]))
    return;

  if (input_manager->IsKeyPressed(keys[0])) {
    toggle_cheat();
  }
  else if (input_manager->IsKeyPressed(keys[1])) {
    set_spawn();
  }
  else if (input_manager->IsKeyPressed(keys[2])) {
    restart_level();
  }
  else if (input_manager->IsKeyDown(keys[3])) {
    for (CKDWORD num_key = CKKEY_1; num_key <= CKKEY_9; num_key++) {
      if (!input_manager->IsKeyPressed(num_key))
        continue;
      set_sector(num_key - CKKEY_1 + 1);
      return;
    }
    constexpr CKKEYBOARD numpad_keys[] = { CKKEY_NUMPAD1, CKKEY_NUMPAD2, CKKEY_NUMPAD3,
                                           CKKEY_NUMPAD4, CKKEY_NUMPAD5, CKKEY_NUMPAD6,
                                           CKKEY_NUMPAD7, CKKEY_NUMPAD8, CKKEY_NUMPAD9 };
    for (int i = 0; i < sizeof(numpad_keys) / sizeof(std::remove_all_extents_t<decltype(numpad_keys)>); i++) {
      if (!input_manager->IsKeyPressed(numpad_keys[i]))
        continue;
      set_sector(i + 1);
      return;
    }
  }
}

void SpeedrunShortcuts::OnPostStartMenu() {
  if (init)
    return;
  int count = m_bml->GetModCount();
  for (int i = 0; i < count; i++) {
    mmo_client = dynamic_cast<decltype(mmo_client)>(m_bml->GetMod(i));
    if (mmo_client != nullptr) {
      GetLogger()->Info("Presence of BMMO client detected, got pointer at %#010x", mmo_client);
      break;
    }
  }
  init = true;
}
