//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
modded class SCR_DoorUserAction : DoorUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_doorLock;
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			// ShowHint("No owner", "Locked");
			// Print("No owner");
			return;
		}

        DoorComponent doorComponent = GetDoorComponent();
		if (!doorComponent) {
			// Print("No doorComp found in DoorUserAction");
			return;
		} else {
			m_door = doorComponent;
			// Print("m_door filled with doorComponent");
		}
		
		GRAD_DoorLockComponent doorLockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent));
		if (!doorLockComponent) {
			// Print("No door-Lock-Component found in DoorUserAction");
			return;
		} else {
			m_doorLock = doorLockComponent;
			if (m_doorLock.GetLockState()) {
				// Print("door locked");
				// ShowHint("Door is locked", "Locked", false);
				doorLockComponent.playSound(true, pOwnerEntity, false);
				
				return;
			} else {
				// Print("door not locked");
				// ShowHint("door not locked", "not locked", false);
			}
		}

        super.PerformAction(pOwnerEntity, pUserEntity);
    }

    //------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {
        DoorComponent doorComponent = GetDoorComponent();
		
		if (doorComponent) {
			return !IsDoorLocked();
		} else {
			return false;
		}
    }

    //------------------------------------------------------------------------------------------------
    override bool GetActionNameScript(out string outName)
    {
        DoorComponent doorComponent = GetDoorComponent();
        if (!doorComponent)
            return false;

        // Logic here is flipped since method returns the opposite of what we expect
        if (Math.AbsFloat(doorComponent.GetControlValue()) >= 0.5)
            outName = "#AR-UserAction_Close";
        else
            outName = "#AR-UserAction_Open";

        return true;
    }

    // Helper method to check if the door is locked
    private bool IsDoorLocked()
    {
		if (!m_doorLock) {
			// Print("m_doorLock empty");
			return false;
		}

        return m_doorLock.GetLockState();
    }
};
