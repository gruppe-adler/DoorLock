//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	protected DoorComponent            m_door;
    protected GRAD_DoorLockComponent   m_lockComponent;
    static const string               SIGNAL_INTERIOR = "GInterior";
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
    {
        super.Init(pOwnerEntity, pManagerComponent);
        m_door          = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
        m_lockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent));
    }
	
	override bool GetActionNameScript(out string outName)
	{
	    if (!m_lockComponent)
	        return false;
		
	    if (m_lockComponent.GetLockState())
	    {
	        outName = "Unlock";
	    }
	    else
	    {
	        outName = "Lock";
	    }
	
	    return true;
	}
	
	override bool CanBePerformedScript(IEntity user)
    {
        if (!m_lockComponent || !IsHumanPlayer(user))
        {
            SetCannotPerformReason("Only human players may lock/unlock doors");
            return false;
        }
        // also prevent locking if door is moving
        return Math.AbsFloat(m_door.GetControlValue()) < 0.01;
    }

    override bool CanBeShownScript(IEntity user)
    {
        if (!IsHumanPlayer(user))
        {
            SetCannotPerformReason("Only human players may lock/unlock doors");
            return false;
        }
        return super.CanBeShownScript(user);
    }
	
   override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
        if (!m_door || !m_lockComponent)
            return;

        bool isGM    = SCR_EditorManagerEntity.IsOpenedInstance();
        string owner = m_lockComponent.GetLockOwner();
        bool locked  = m_lockComponent.GetLockState();
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(pUserEntity);
		bool needsClose = false;
		if (char)
		{
		    string factionKey = char.GetFactionKey();
		    if (factionKey == owner || owner.IsEmpty())
		    {
		        needsClose = true;
		    }
		}

        if (CanUseLock(owner, isGM, SCR_ChimeraCharacter.Cast(pUserEntity), pUserEntity, pOwnerEntity, locked))
        {
            // forward to component; it does the RPC internally
            m_lockComponent.RequestToggleLock(pUserEntity, !locked);
        }
        else
        {
            if (needsClose)
			{
			    ShowNotification(ENotification.GRAD_DOORLOCK_CLOSE_FIRST, pUserEntity, isGM);
			}
			else
			{
			    ShowNotification(ENotification.GRAD_DOORLOCK_NO_KEY, pUserEntity, isGM);
			}
        }
    }
	
	
	private bool CanUseLock(string lockOwner, bool isGM, SCR_ChimeraCharacter charent, IEntity user, IEntity door, bool locked)
	  {
	      // GM always allowed (and we flip ownership)
	      if (isGM)
	          return true;
	
	      // players inside can always unlock
	      float inside = GetGame().GetSignalsManager().GetSignalValue(
	          GetGame().GetSignalsManager().AddOrFindSignal(SIGNAL_INTERIOR)
	      );
	      if (inside > 0.9)
	          return true;
	
	      // if they’re the owner faction (or unclaimed), allow it
	      if (charent && (charent.GetFactionKey() == lockOwner || lockOwner.IsEmpty()))
	      {
	          if (lockOwner.IsEmpty())
	              m_lockComponent.SetLockOwner(charent.GetFactionKey());
	          return true;
	      }
	
	      return false;
	  }
	
	private void ShowNotification(ENotification msg, IEntity user, bool isGM)
	  {
	      int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	      if (isGM)
	          pid = SCR_PlayerController.GetLocalPlayerId();
	
	      SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(pid, msg, pid);
	  }
	
	private bool IsHumanPlayer(IEntity user)
	  {
	      AIControlComponent ai = AIControlComponent.Cast(user.FindComponent(AIControlComponent));
	      return ai == null || !ai.IsAIActivated();
	  }
};
