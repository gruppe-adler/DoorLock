//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_lockComponent;
	IEntity m_building;
	static const string SIGNAL_INTERIOR = "GInterior";
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) {
		
		m_door = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
		m_building = pOwnerEntity.GetRootParent();
		
		m_lockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent));
		
		if (!m_door || !m_lockComponent) {
	        return;
	    }
		
		super.Init(pOwnerEntity, pManagerComponent);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (!m_lockComponent)
			return false;
		
		if (m_lockComponent.GetLockState()) { 
			outName = "Unlock";
			return true;
		} else { 
			outName = "Lock";
			return true;
		};		
	}
	
	 override bool CanBePerformedScript(IEntity user)
    {
		if (!IsHumanPlayer(user)) {
		    SetCannotPerformReason("Only human players may lock/unlock doors");
			PrintFormat("Only human players may lock/unlock doors");
		    return false;
		}

        // for players, only if we actually have a lock component
        return m_lockComponent != null;
    }

    override bool CanBeShownScript(IEntity user)
    {
        if (!IsHumanPlayer(user)) {
	    SetCannotPerformReason("Only human players may lock/unlock doors");
			PrintFormat("Only human players may lock/unlock doors");
	    return false;
	}

        // otherwise fall back to normal scripted-visibility
        return super.CanBeShownScript(user);
    }
	
	private bool IsHumanPlayer(IEntity user) {
    	AIControlComponent ai = AIControlComponent.Cast(user.FindComponent(AIControlComponent));
    	return ai == null || !ai.IsAIActivated();
	}
	
	
    //------------------------------------------------------------------------------------------------
	// to improve performance we will do all checks on perform and not in canPerform
	// this also allows to inform the user only on interaction and not by disabling actions "by default"
	//------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
		
		if (!pOwnerEntity || !m_door) {
			Print("no owner or no m_door in DoorLockAction");
			return;
		}
		
		// avoid double execution
		//		if (!Replication.IsServer()) return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
        string lockOwnerString = m_lockComponent.GetLockOwner();
        bool isLocked = m_lockComponent.GetLockState();
		
		// dirty double execution / local only check 
		if (!isGM) {
			if (SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(pUserEntity) != SCR_PlayerController.GetLocalPlayerId())
			return;
		} else {
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			int playerID = playerController.GetPlayerId();
			if (playerID != SCR_PlayerController.GetLocalPlayerId())
			return;
		}

        if (CanUseLock(lockOwnerString, isGM, character, pUserEntity, pOwnerEntity, isLocked)) {
            m_lockComponent.ToggleLockState(pUserEntity, pOwnerEntity.GetParent(), !isLocked);
        } else {
			if (character.GetFactionKey() == lockOwnerString || lockOwnerString.IsEmpty()) {
            	ShowNotification(ENotification.GRAD_DOORLOCK_CLOSE_FIRST, pUserEntity, isGM);
			} else {
				ShowNotification(ENotification.GRAD_DOORLOCK_NO_KEY, pUserEntity, isGM);
			}
        }
    }
	
	
	private bool CanUseLock(string lockOwnerString, bool isGM, SCR_ChimeraCharacter character, IEntity pUserEntity, IEntity pOwnerEntity, bool isLocked) {
        
		if (!m_door) {
			Print("no m_door in DoorLockAction");
			return false;
		}
		
		if (Math.AbsFloat(m_door.GetControlValue()) > 0) {
			return false;
		}
		
		if (character && !isGM) {
            float gInterior = GetGame().GetSignalsManager().GetSignalValue(
                GetGame().GetSignalsManager().AddOrFindSignal(SIGNAL_INTERIOR)
            );

            // Players inside the house can always unlock to prevent being stuck
            if (gInterior == 1) {
                Print("Player is inside");
                return true;
            }

            if (character.GetFactionKey() == lockOwnerString || lockOwnerString.IsEmpty()) {
				// Take lock for side if unclaimed
                if (lockOwnerString.IsEmpty()) {
                    string factionKey = character.GetFactionKey();
                    m_lockComponent.SetLockOwner(factionKey);
                    PrintFormat("Faction '%1' is now the lock owner", factionKey);
                }
                return true;
            }
        } else if (isGM) {

            if (isLocked) {
                ShowNotification(ENotification.GRAD_DOORLOCK_GM_UNLOCKED, pUserEntity, true);
                m_lockComponent.SetLockOwner("");
            } else {
				// GM locked doors cant be opened by players
                ShowNotification(ENotification.GRAD_DOORLOCK_GM_LOCKED, pUserEntity, true);
                m_lockComponent.SetLockOwner("GM");
            }
            return true;
        }

        return false;
    }
	
	private void ShowNotification(ENotification message, IEntity pUserEntity, bool isGM) {		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		if (isGM) {
			playerID = SCR_PlayerController.GetLocalPlayerId();
		}
				
		bool thatworked = SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(playerID, message, playerID); // SCR_NotificationsComponent.SendToPlayer(playerID, message);
		PrintFormat("Notification sent: %1", thatworked);
	}
};
