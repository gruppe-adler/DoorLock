//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	protected DoorComponent            m_door;
    protected GRAD_DoorLockComponent   m_lockComponent;
	
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

        bool isGM = SCR_EditorManagerEntity.IsOpenedInstance();
        int playerId;

        if (isGM)
        {
            playerId = SCR_PlayerController.GetLocalPlayerId();
        }
        else
        {
            playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
        }

        if (playerId < 0)
        {
            Print("GRAD_DoorLockAction: Invalid player ID");
            return;
        }

        // Let the server RPC handle all validation, permissions, and state changes
        bool wantLock = !m_lockComponent.GetLockState();
        m_lockComponent.RequestUse(playerId, wantLock);
    }
	
	private bool IsHumanPlayer(IEntity user)
	  {
	      AIControlComponent ai = AIControlComponent.Cast(user.FindComponent(AIControlComponent));
	      return ai == null || !ai.IsAIActivated();
	  }
};
