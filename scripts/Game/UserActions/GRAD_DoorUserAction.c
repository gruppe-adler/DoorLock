//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
modded class SCR_DoorUserAction : DoorUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_doorLock;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) {
		DoorComponent doorComponent = GetDoorComponent();
		m_door = doorComponent;
		
		GRAD_DoorLockComponent doorLockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent)); 
		m_doorLock = doorLockComponent;
		
		super.Init(pOwnerEntity, pManagerComponent);
	}
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			// ShowHint("No owner", "Locked");
			// Print("No owner");
			return;
		}
		
		// if init has failed for whatever reason
		if (!m_door) {
			DoorComponent doorComponent = GetDoorComponent();
			m_door = doorComponent;
			if (!m_door) return;
		}
        
		// if init has failed for whatever reason
		if (!m_doorLock) {
			GRAD_DoorLockComponent doorLockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent)); 
			m_doorLock = doorLockComponent;
			if (!m_doorLock) return;
		}
		
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity);
		m_doorLock.RequestUse(playerId);
    }

    //------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {
		if (m_door) {
			return !IsDoorLocked();
		} else {
			return false;
		}
    }

    //------------------------------------------------------------------------------------------------
    override bool GetActionNameScript(out string outName)
    {
        if (!m_door)
            return false;

        // Logic here is flipped since method returns the opposite of what we expect
        if (Math.AbsFloat(m_door.GetControlValue()) >= 0.5)
            outName = "#AR-UserAction_Close";
        else
            outName = "#AR-UserAction_Open";

        return true;
    }

    // Helper method to check if the door is locked
    private bool IsDoorLocked()
    {
		// m_doorlock is only really possible to get in performaction, thus first time we let the action pass to get actual state
		if (!m_doorLock) {
			return false;
		}
        return m_doorLock.GetLockState();
    }
};
