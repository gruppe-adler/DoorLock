//! User action that ought to be attached to an entity with door component.
//! When performed either opens or closes the door based on the previous state of the door.
modded class SCR_DoorUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
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
			if ((dotP < 0.0 && currentState <= 0.0)  || (dotP > 0.0 && currentState < 0.0))
				controlValue = -1.0;
			if (Math.AbsFloat(doorComponent.GetControlValue()) > 0.5)
				controlValue = 0.0;
			
			//Print(controlValue);
			doorComponent.SetActionInstigator(pUserEntity);
			doorComponent.SetControlValue(controlValue);
		}
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
};
