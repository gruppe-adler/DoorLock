[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

class GRAD_DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
	[RplProp()]
    bool m_isLocked;
		
	[RplProp()]
	string m_lockOwner = "";

    // Called when the game initializes the component
    override void OnPostInit(IEntity owner)
    {
		Replication.BumpMe();
		
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
	
	void ToggleLockState(IEntity pUserEntity, IEntity door)
	{	
		if (m_isLocked) {
			playSound(true, door, true);
			m_isLocked = false;
			Replication.BumpMe();
		} else {
			playSound(false, door, true);
			m_isLocked = true;
			Replication.BumpMe();
		}
		// Print("Door Lock Component set to: " + m_isLocked.ToString());		
	}
	
	void playSound (bool locked, IEntity pOwnerEntity, bool canUnlock) {
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		// Print("soundmanager exists");
		
		private vector m_vSoundOffset;
		
		SCR_AudioSourceConfiguration audioConfig;
		
		if (locked && canUnlock) {
			audioConfig	= CreateSoundAudioConfig("{F9CD6FDD2DB344FC}sounds/doorlock2.acp", "SOUND_DOOR_LOCK_2");
		} else if (!locked && canUnlock) {
			audioConfig	= CreateSoundAudioConfig("{604949E18452E5D5}sounds/doorlock1.acp", "SOUND_DOOR_LOCK_1");
		} else if (locked && !canUnlock) {
			audioConfig	= CreateSoundAudioConfig("{A393DED2C2547136}sounds/rattle.acp", "SOUND_DOOR_LOCK_RATTLE");
		}
		
		if (!audioConfig || !audioConfig.IsValid())
			return;
		
		soundManagerEntity.CreateAndPlayAudioSource(pOwnerEntity, audioConfig);
		
		// PrintFormat("audioConfig is %2, pOwnerEntity is %1", pOwnerEntity, audioConfig);
	}
	
	// ripped from SCR_BaseDamageHealSupportStationComponent
	SCR_AudioSourceConfiguration CreateSoundAudioConfig(string soundFile, string soundEventName)
	{		
		//~ Does not have audio assigned
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundFile) || SCR_StringHelper.IsEmptyOrWhiteSpace(soundEventName))
			return null;
		
		//~ Create Audio source
		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration();
		audioSourceConfiguration.m_sSoundProject = soundFile;
		audioSourceConfiguration.m_sSoundEventName = soundEventName;
		audioSourceConfiguration.m_eFlags = SCR_Enum.SetFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
		
		
		return audioSourceConfiguration;
	}	
	
	void SetLockOwner(string lockOwner) {
		m_lockOwner = lockOwner;
		Replication.BumpMe();
		PrintFormat("Lock Owner set to %1", lockOwner);
	}
}