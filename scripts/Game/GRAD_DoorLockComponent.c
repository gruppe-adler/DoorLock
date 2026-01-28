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
	
	static const string SIGNAL_INTERIOR = "GInterior";
	static const float SOUND_AUDIBLE_RANGE = 50.0; // meters - sounds only play within this distance

	// Server-only RPC: enforces lock/unlock permissions and opens/closes
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

        // 3) Check if this is a GM
        bool isGM = false;
        SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
        if (editorManager)
        {
            SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
            if (core)
            {
                array<int> playerIds = {};
                core.GetPlayers(playerIds);
                isGM = playerIds.Contains(userPlayerId);
            }
        }

        // 4) Determine the caller's faction
        SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(userEnt);
        string faction = "";
        if (ch)
            faction = ch.GetFactionKey();

		PrintFormat(
	        "Rpc_HandleDoorUse: userId=%1, faction='%2', wantLock=%3, currentLocked=%4, lockOwner='%5', isGM=%6",
	        userPlayerId, faction, wantLockState, m_isLocked, m_lockOwner, isGM
	    );

        // 5) Check if player is inside (can always unlock if inside)
        bool isInside = false;
        if (ch)
        {
            float insideValue = GetGame().GetSignalsManager().GetSignalValue(
                GetGame().GetSignalsManager().AddOrFindSignal(SIGNAL_INTERIOR)
            );
            isInside = insideValue > 0.9;
        }

        // 6) Get door component for state checks
        DoorComponent dc = DoorComponent.Cast(doorEnt.FindComponent(DoorComponent));
        if (!dc)
            return;

        // 7) UNLOCK request?
        if (wantLockState == false && m_isLocked == true)
        {
            // GM can always unlock and clears ownership
            if (isGM)
            {
                Print(" → GM UNLOCK");
                m_isLocked = false;
                m_lockOwner = "";
                Replication.BumpMe();
                Rpc(Rpc_PlaySoundBroadcast, false, true);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_GM_UNLOCKED,
                    userPlayerId
                );
                return;
            }

            // Players inside can always unlock
            if (isInside)
            {
                Print(" → Inside player UNLOCK");
                m_isLocked = false;
                m_lockOwner = "";
                Replication.BumpMe();
                Rpc(Rpc_PlaySoundBroadcast, false, true);
                return;
            }

            // Only owning faction may unlock
            if (faction != m_lockOwner)
            {
				PrintFormat(
	                " → Deny UNLOCK: faction '%1' != lockOwner '%2'",
	                faction, m_lockOwner
	            );
                Rpc(Rpc_PlaySoundBroadcast, true, false);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_NO_KEY,
                    userPlayerId
                );
                return;
            }

			Print(" → Allow UNLOCK");
            m_isLocked = false;
            m_lockOwner = "";
            Replication.BumpMe();
            Rpc(Rpc_PlaySoundBroadcast, false, true);
            return;
        }

        // 8) LOCK request?
        if (wantLockState == true && m_isLocked == false)
        {
            // Check if door is closed (can't lock open door)
            if (Math.AbsFloat(dc.GetControlValue()) > 0.01)
            {
                Print(" → Deny LOCK: door not closed");
                Rpc(Rpc_PlaySoundBroadcast, true, false);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_CLOSE_FIRST,
                    userPlayerId
                );
                return;
            }

            // GM can always lock and sets ownership to GM
            if (isGM)
            {
                Print(" → GM LOCK");
                m_isLocked = true;
                m_lockOwner = "GM";
                Replication.BumpMe();
                Rpc(Rpc_PlaySoundBroadcast, true, true);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_GM_LOCKED,
                    userPlayerId
                );
                return;
            }

            // Only players with a faction may lock
            if (faction == "")
            {
                Print(" → Deny LOCK: no faction");
                Rpc(Rpc_PlaySoundBroadcast, true, false);
                SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(
                    userPlayerId,
                    ENotification.GRAD_DOORLOCK_NO_KEY,
                    userPlayerId
                );
                return;
            }

            Print(" → Allow LOCK");
            m_isLocked = true;
            m_lockOwner = faction;
            Replication.BumpMe();
            Rpc(Rpc_PlaySoundBroadcast, true, true);
            return;
        }

        // 9) FALLBACK: open/close the door (if not locked)
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
            dc.SetControlValue(target, true);
        }
    }
	
	// public wrapper so other classes can invoke the above RPC
	void RequestUse(int userPlayerId, bool wantLock)
	{
	    Rpc(Rpc_HandleDoorUse, userPlayerId, wantLock);
	}

	// Broadcast RPC to play sounds on all clients (with proximity check)
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_PlaySoundBroadcast(bool locked, bool canUnlock)
	{
		IEntity doorEnt = GetOwner();
		if (!doorEnt)
			return;

		// Proximity check: only play sound if local player is nearby
		IEntity localPlayer = SCR_PlayerController.GetLocalControlledEntity();
		if (!localPlayer)
			return;

		vector doorPos = doorEnt.GetOrigin();
		vector playerPos = localPlayer.GetOrigin();
		float distanceSquared = vector.DistanceSq(doorPos, playerPos);
		float rangeSquared = SOUND_AUDIBLE_RANGE * SOUND_AUDIBLE_RANGE;

		// Only play sound if within audible range
		if (distanceSquared <= rangeSquared)
			PlaySoundLocal(locked, doorEnt, canUnlock);
	}

	// Local sound playback (called on each client via RPC)
	protected void PlaySoundLocal(bool locked, IEntity pOwnerEntity, bool canUnlock)
	{
		
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
	
	// Protected - should only be called from RPC on server
	protected void SetLockOwner(string lockOwner)
	{
		m_lockOwner = lockOwner;
		Replication.BumpMe();
		PrintFormat("Lock Owner set to %1", lockOwner);
	}
}