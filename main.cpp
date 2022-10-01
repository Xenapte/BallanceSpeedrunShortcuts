#include "main.h"

IMod* BMLEntry(IBML* bml) {
  return new SpeedrunShortcuts(bml);
}

void SpeedrunShortcuts::OnLoadScript(C_CKSTRING filename, CKBehavior* script) {
  if (strcmp(script->GetName(), "Gameplay_Events") == 0) {
    auto* esc = ScriptHelper::FindFirstBB(script, "Key Event");
    esc_event = CKOBJID(esc->GetOutput(0));
  }
  if (strcmp(script->GetName(), "Menu_Pause") == 0) {
    restart_level = CKOBJID(ScriptHelper::FindFirstBB(script, "Restart Level"));
  }
}

void SpeedrunShortcuts::OnProcess() {
  if (input_manager->IsKeyDown(CKKEY_LMENU)) {
    if (input_manager->IsKeyPressed(CKKEY_C)) {
      m_bml->EnableCheat(!m_bml->IsCheatEnabled());
      m_bml->SendIngameMessage((std::string("Cheat mode ").append(m_bml->IsCheatEnabled() ? "enabled" : "disabled")).c_str());
    }
    else if (input_manager->IsKeyPressed(CKKEY_S)) {
      if (!m_bml->IsCheatEnabled())
        return;
      if (!current_level) 
        current_level = m_bml->GetArrayByName("CurrentLevel");

      if (m_bml->IsIngame() && current_level) {
        CK3dEntity* camRef = m_bml->Get3dEntityByName("Cam_OrientRef");
        VxMatrix mat = camRef->GetWorldMatrix();
        for (int i = 0; i < 3; i++) {
          std::swap(mat[0][i], mat[2][i]);
          mat[0][i] = -mat[0][i];
        }
        current_level->SetElementValue(0, 3, &mat);
        m_bml->SendIngameMessage(
          ("Set Spawn Point to (" +
           std::to_string(mat[3][0]) + ", " +
           std::to_string(mat[3][1]) + ", " +
           std::to_string(mat[3][2]) + ")").c_str());
      }
    }
    else if (input_manager->IsKeyPressed(CKKEY_E)) {
      if (!m_bml->IsIngame())
        return;
      auto* esc = static_cast<CKBehaviorIO*>(m_bml->GetCKContext()->GetObject(esc_event));
      esc->Activate();

      m_bml->AddTimer(3ul, [this]() {
        CKMessageManager* mm = m_bml->GetMessageManager();

        CKMessageType reset_level_msg = mm->AddMessageType("Reset Level");
        mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("Level", CKCID_BEOBJECT, nullptr)));
        mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("All_Balls", CKCID_BEOBJECT, nullptr)));

        auto* beh = static_cast<CKBehavior*>(m_bml->GetCKContext()->GetObject(restart_level));
        auto* output = beh->GetOutput(0);
        output->Activate();
      });
    }
  }
}