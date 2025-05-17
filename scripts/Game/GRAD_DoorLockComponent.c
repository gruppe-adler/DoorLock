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
	
	protected RplComponent m_RplComponent;

    // Called when the game initializes the component
    override void OnPostInit(IEntity owner)
    {	
        super.OnPostInit(owner);
        // Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
		// cache our rpl
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
    }
	
	bool GetLockState() 
	{
		return m_isLocked;
	}
	
	string GetLockOwner() 
	{
		return m_lockOwner;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void Rpc_ToggleLock(int userPlayerId, bool targetState)
	{	
		IEntity userEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(userPlayerId);
	    SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(userEnt);
	    bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
	    string faction = "";
		if (ch) {
			faction = ch.GetFactionKey();
		}
		
		m_isLocked = targetState;
        Replication.BumpMe();
        // play sound on server so clients hear it
        playSound(targetState, GetOwner(), true);
		Print("IsServer: " + Replication.IsServer());
		Print("IsClient: " + Replication.IsClient());
		Print("InRuntimeMode: " + Replication.Runtime());
		Print("InLoadtimeMode: " + Replication.Loadtime());
	}
	
	void RequestToggleLock(IEntity pUserEntity, bool targetState)
    {
		int userPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
        // execute on server
        Rpc(Rpc_ToggleLock, userPlayerId, targetState);
    }
	
	void RequestLockedFeedback(int userPlayerId)
    {
       // calling the stub from _inside_ this class is allowed
       Rpc_PlayLockedFeedback(userPlayerId);
    }
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_PlayLockedFeedback(int userPlayerId)
	{
	    // 1) play the “rattle” sound for everyone
	    playSound(true, GetOwner(), /*canUnlock=*/false);
	
	    // 2) notify only the single player who tried the door
	    SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
	        userPlayerId,
	        ENotification.GRAD_DOORLOCK_NO_KEY,
	        userPlayerId
	    );
	}
	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_HandleDoorUse(int userPlayerId)
	{
	    IEntity doorEnt = GetOwner();
	    IEntity userEnt = GetGame().GetPlayerManager().GetPlayerControlledEntity(userPlayerId);
	    if (!doorEnt || !userEnt) 
	        return;
	
	    // if locked: rattle + notify
	    if (m_isLocked)
	    {
	        playSound(true, doorEnt, /*canUnlock=*/false);
	        SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
	            userPlayerId,
	            ENotification.GRAD_DOORLOCK_NO_KEY,
	            userPlayerId
	        );
	    }
	    else
	    {
	        // unlocked: do the vanilla open/close on the server
	        DoorComponent dc = DoorComponent.Cast(doorEnt.FindComponent(DoorComponent));
	        if (dc) { 
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
				dc.SetControlValue(target) 
			};
	    }
	}
	
	// public wrapper so other classes can invoke the above RPC
	void RequestUse(int userPlayerId)
	{
	    Rpc_HandleDoorUse(userPlayerId);
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