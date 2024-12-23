//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
modded class SCR_DoorUserAction : DoorUserAction
{
	IEntity m_door;
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity || !pUserEntity)
            return;

        m_door = pOwnerEntity;
		
        // Ensure the door is not locked
        if (IsDoorLocked())
        {
            ShowHint("Damn, I cannot open this", "Locked");
            return;
        }

        DoorComponent doorComponent = GetDoorComponent();
        if (doorComponent)
        {
            vector doorOpeningVecWS = pOwnerEntity.VectorToParent(vector.Forward);
            if (doorComponent.GetAngleRange() < 0.0)
                doorOpeningVecWS = -1.0 * doorOpeningVecWS;

            vector userMat[4];
            pUserEntity.GetWorldTransform(userMat);
            float dotP = vector.Dot(doorOpeningVecWS, userMat[3] - doorComponent.GetDoorPivotPointWS());

            // Flip the control value if it is above a threshold
            float controlValue = 1.0;
            float currentState = doorComponent.GetDoorState();
            if ((dotP < 0.0 && currentState <= 0.0) || (dotP > 0.0 && currentState < 0.0))
                controlValue = -1.0;
            if (Math.AbsFloat(doorComponent.GetControlValue()) > 0.5)
                controlValue = 0.0;

            // Perform the action
            doorComponent.SetActionInstigator(pUserEntity);
            doorComponent.SetControlValue(controlValue);
        }

        super.PerformAction(pOwnerEntity, pUserEntity);
    }

    //------------------------------------------------------------------------------------------------
    override bool CanBePerformedScript(IEntity user)
    {
        // Prevent action if the door is locked
        if (IsDoorLocked())
            return false;

        DoorComponent doorComponent = GetDoorComponent();
        return doorComponent != null;
    }

    //------------------------------------------------------------------------------------------------
    override bool GetActionNameScript(out string outName)
    {
        DoorComponent doorComponent = GetDoorComponent();
        if (!doorComponent)
            return false;

        // Prevent action name change if the door is locked
        if (IsDoorLocked())
        {
            outName = "Locked"; // Or any appropriate locked message
            return true;
        }

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
		if (!m_door)
            return false;

        GRAD_DoorLockComponent lockComponent = GRAD_DoorLockComponent.Cast(m_door.FindComponent(GRAD_DoorLockComponent));
        if (!lockComponent)
            return false;

        return lockComponent.GetLockState();
    }

    // Helper method to show hints
    private void ShowHint(string message, string title, bool isPersistent = false)
    {
        SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
        if (!hintManager)
            return;

        hintManager.ShowCustomHint(message, title, 12, isPersistent, EHint.UNDEFINED, false);
    }
};
