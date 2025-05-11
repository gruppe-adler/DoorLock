modded class SCR_AIOpenDoor : AITaskScripted
{

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity doorEntity;
		GetVariableIn(PORT_DOOR_ENTITY, doorEntity);
		
		if (!doorEntity)
			return ENodeResult.FAIL;
		
		array<ScriptedUserAction> doorActions = {};
		FindDoorActions(doorEntity, doorActions, m_bOpenSiblingDoors);
		
		if (doorActions.IsEmpty())
			return ENodeResult.FAIL;
		
		IEntity actionPerformer = owner.GetControlledEntity();
		if (!actionPerformer)
			actionPerformer = owner;
		
		foreach (ScriptedUserAction action : doorActions)
		{
			IEntity actionOwner = action.GetOwner();
			if (!actionOwner)
				continue;
			DoorComponent doorComp = DoorComponent.Cast(actionOwner.FindComponent(DoorComponent));
			if (!doorComp)
				continue;
				
			GRAD_DoorLockComponent doorLock = GRAD_DoorLockComponent.Cast(doorComp.FindComponent(GRAD_DoorLockComponent));
			bool isLocked = false;
			if (doorLock) {
				isLocked = doorLock.GetLockState();
			}
			
			if (!doorComp.IsOpen() && !doorComp.IsOpening() && !isLocked)
				action.PerformAction(actionOwner, actionPerformer);
		}
			
		return ENodeResult.SUCCESS;
	}
};