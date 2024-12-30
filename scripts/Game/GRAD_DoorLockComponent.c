[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

class GRAD_DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
	[RplProp()]  
    bool m_isLocked;
	string m_lockOwner = "";

    // Called when the game initializes the component
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        // Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
    }
	
	bool GetLockState() 
	{
		return m_isLocked;
	}
	
	string GetLockOwner() 
	{
		return m_lockOwner;
	}
	
	void ToggleLockState(IEntity pUserEntity)
	{
		
		// get user identity to set lock side
		if (!m_lockOwner) {
			if (pUserEntity) {
				PrintFormat("pUserEntity: %1", pUserEntity);
				if (SCR_EditorManagerEntity.IsOpenedInstance()) {
					Print("Setting LockOwner to GM");
					SetLockOwner("GM");
				} else {
					SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
					if (character) {
						SetLockOwner(character.GetFactionKey());
						PrintFormat("Setting Lock to ", character.GetFactionKey());
					}
				};
			} else {
				Print("pUserEntity is empty");
			}
		}
		
		if (m_isLocked) {
			playSound(true, pUserEntity);
			m_isLocked = false;
			Replication.BumpMe();
		} else {
			playSound(false, pUserEntity);
			m_isLocked = true;
			Replication.BumpMe();
		}
		Print("Door Lock Component set to: " + m_isLocked.ToString());		
	}
	
	void playSound (bool locked, IEntity pOwnerEntity) {
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		Print("soundmanager exists");
		
		private vector m_vSoundOffset;
		
		SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
		audioConfig.m_sSoundProject = "{604949E18452E5D5}sounds/doorlock1.acp";
		audioConfig.m_sSoundEventName = "door_lock_1";
		audioConfig.m_eFlags = EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed;	
		
		if (!audioConfig || !audioConfig.IsValid())
			return;
		
		Print("audioConfig valid");
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(pOwnerEntity, audioConfig);
		if (!audioSource)
			return;
		
		Print("audioSource played");
		
		soundManagerEntity.CreateAndPlayAudioSource(pOwnerEntity, audioConfig);
	}
	
	void SetLockOwner(string lockOwner) {
		m_lockOwner = lockOwner;
		PrintFormat("Lock Owner set to %1", lockOwner);
	}
}