[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

[RplComponent]
class GRAD_DoorLockComponent : ScriptComponent
{
    // synced attributes
	[RplProp()]
    protected bool m_isLocked;
		
	[RplProp()]
	protected string m_lockOwner = "";

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
	
	 // Server‐only RPC: enforces lock/unlock permissions and opens/closes
    [RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void Rpc_HandleDoorUse(int userPlayerId, bool wantLockState)
    {
        // 1) Only run on the true server
        if (!Replication.IsServer())
	    {
	        Print("Rpc_HandleDoorUse: skipping because not server");
	        return;
	    }


        // 2) Lookup entities
        IEntity doorEnt = GetOwner();
        IEntity userEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(userPlayerId);
        if (!doorEnt || !userEnt)
            return;

        // 3) Determine the caller’s faction
        SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(userEnt);
        string faction = "";
        if (ch)
            faction = ch.GetFactionKey();
		
		
		PrintFormat(
	        "Rpc_HandleDoorUse: userId=%1, faction='%2', wantLock=%3, currentLocked=%4, lockOwner='%5'",
	        userPlayerId, faction, wantLockState, m_isLocked, m_lockOwner
	    );

        // 4) UNLOCK request?
        if (wantLockState == false && m_isLocked == true)
        {
            // only owning faction may unlock
            if (faction != m_lockOwner)
            {
				 PrintFormat(
	                " → Deny UNLOCK: faction '%1' != lockOwner '%2'",
	                faction, m_lockOwner
	            );
                playSound(true, doorEnt, false);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_NO_KEY,
                    userPlayerId
                );
                return;
            }
			
			Print(" → Allow UNLOCK");
            // allowed → clear lock
            m_isLocked   = false;
            m_lockOwner  = "";
            Replication.BumpMe();
            playSound(false, doorEnt, true);
            return;
        }

        // 5) LOCK request?
        if (wantLockState == true && m_isLocked == false)
        {
            // only players with a faction may lock
            if (faction == "")
            {
                playSound(true, doorEnt, false);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_NO_KEY,
                    userPlayerId
                );
                return;
            }

            m_isLocked  = true;
            m_lockOwner = faction;
            Replication.BumpMe();
            playSound(true, doorEnt, true);
            return;
        }

        // 6) FALLBACK: open/close the door
        DoorComponent dc = DoorComponent.Cast(doorEnt.FindComponent(DoorComponent));
        if (dc)
        {
            float current = dc.GetControlValue();
            float target;
            if (current > 0.5)
            {
                target = 0.0;
            }
            else
            {
                target = 1.0;
            }
            dc.SetControlValue(target);
        }
    }
	
	// public wrapper so other classes can invoke the above RPC
	void RequestUse(int userPlayerId, bool wantLock)
	{
	    Rpc(Rpc_HandleDoorUse, userPlayerId, wantLock);
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