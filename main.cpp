#include "main.h"

IMod* BMLEntry(IBML* bml) {
  return new SpeedrunShortcuts(bml);
}

void SpeedrunShortcuts::OnLoadScript(CKSTRING filename, CKBehavior* script) {
	if (strcmp(script->GetName(), "Gameplay_Events") == 0) {
		auto* esc = ScriptHelper::FindFirstBB(script, "Key Event");
		esc_event = CKOBJID(esc->GetOutput(0));
  }
	if (strcmp(script->GetName(), "Menu_Pause") == 0) {
		restart_level = CKOBJID(ScriptHelper::FindFirstBB(script, "Restart Level"));
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
	for (int i = 0; i < 3; i ++) {
		update_key_property(i);
	}
	GetConfig()->SetCategoryComment("BMMO", "BMMO-related stuff for cheat detection.");
	prop_mmo_strict_mode = GetConfig()->GetProperty("BMMO", "StrictMode");
	prop_mmo_strict_mode->SetDefaultBoolean(false);
	prop_mmo_strict_mode->SetComment("Completely block restarting during inappropriate conditions (for example, when on a Trafo). Otherwise only a warning will be issued.");
	mmo_strict_mode = prop_mmo_strict_mode->GetBoolean();
}

void SpeedrunShortcuts::OnModifyConfig(CKSTRING category, CKSTRING key, IProperty* prop) {
	if (strcmp("Keys", category) == 0) {
		for (int i = 0; i < 3; i++) {
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

void SpeedrunShortcuts::OnProcess() {
  if (input_manager->IsKeyDown(CKKEY_LMENU)) {
    if (input_manager->IsKeyPressed(keys[0])) {
      m_bml->EnableCheat(!m_bml->IsCheatEnabled());
			m_bml->SendIngameMessage((std::string("Cheat mode ").append(m_bml->IsCheatEnabled() ? "enabled" : "disabled")).c_str());
    }
    else if (input_manager->IsKeyPressed(keys[1])) {
      if (!m_bml->IsCheatEnabled() || !m_bml->IsIngame())
				return;
			if (!current_level) 
				current_level = m_bml->GetArrayByName("CurrentLevel");
    
			if (m_bml->IsIngame() && current_level) {
				CK3dEntity* camRef = m_bml->Get3dEntityByName("Cam_OrientRef");
				VxMatrix matrix = camRef->GetWorldMatrix();
				for (int i = 0; i < 4; i++) {
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
		else if (input_manager->IsKeyPressed(keys[2])) {
			if (!m_bml->IsIngame())
				return;
			if (mmo_client) {
				if (mmo_client->connected() && nav_inactive && m_bml->IsPlaying()) {
					if (mmo_strict_mode) {
						m_bml->SendIngameMessage("Warning: you are connected to a BMMO server in Strict Mode and cannot reset inappropriately.");
						return;
					}
					else {
						bmmo::public_warning_msg msg{};
						msg.text_content = mmo_client->get_own_name() + " just restarted a level when their ball is not controllable.";
						msg.serialize();
						mmo_client->send(msg.raw.str().data(), msg.size(), k_nSteamNetworkingSend_Reliable);
					}
				}
			}
			auto* esc = static_cast<CKBehaviorIO*>(m_bml->GetCKContext()->GetObject(esc_event));
			esc->Activate();

			//m_bml->AddTimer(3u, [this]() {
			std::thread([this] {
				CKMessageManager* mm = m_bml->GetMessageManager();
				std::this_thread::sleep_for(std::chrono::milliseconds(120));

				CKMessageType reset_level_msg = mm->AddMessageType("Reset Level");
				mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("Level", CKCID_BEOBJECT, nullptr)));
				mm->SendMessageSingle(reset_level_msg, static_cast<CKBeObject*>(m_bml->GetCKContext()->GetObjectByNameAndParentClass("All_Balls", CKCID_BEOBJECT, nullptr)));

				auto* beh = static_cast<CKBehavior*>(m_bml->GetCKContext()->GetObject(restart_level));
				auto* output = beh->GetOutput(0);
				output->Activate();
			}).detach();
		}
  }
}

void SpeedrunShortcuts::OnPostStartMenu() {
	if (init)
		return;
	int count = m_bml->GetModCount();
	for (int i = 0; i < count; i++) {
		mmo_client = dynamic_cast<client*>(m_bml->GetMod(i));
		if (mmo_client != nullptr) {
			GetLogger()->Info("Presence of BMMO client detected, got pointer at %#010x", mmo_client);
			break;
		}
	}
	init = true;
}
