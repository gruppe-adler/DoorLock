/* thanks gemini for fixing my shit :-* */
modded class SCR_AIOpenDoor : AITaskScripted{

    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
        IEntity doorEntity;
        GetVariableIn(PORT_DOOR_ENTITY, doorEntity);

        if (!doorEntity)
            return ENodeResult.FAIL;

        // It might be beneficial to check the lock status of the *primary* doorEntity even before finding all actions.
        // This provides a quick exit if the main target is locked.
        DoorComponent primaryDoorComp = DoorComponent.Cast(doorEntity.FindComponent(DoorComponent));
        if (primaryDoorComp)
        {
            GRAD_DoorLockComponent primaryDoorLock = GRAD_DoorLockComponent.Cast(primaryDoorComp.FindComponent(GRAD_DoorLockComponent)); // Assuming lock component is sibling to DoorComponent
            if (primaryDoorLock && primaryDoorLock.GetLockState() && !primaryDoorComp.IsOpen()) // If primary door is locked and closed
            {
                return ENodeResult.FAIL; // Cannot open the specifically targeted locked door.
            }
        }

        array<ScriptedUserAction> doorActions = {};
        FindDoorActions(doorEntity, doorActions, m_bOpenSiblingDoors);

        if (doorActions.IsEmpty())
            return ENodeResult.FAIL;

        IEntity actionPerformer = owner.GetControlledEntity();
        if (!actionPerformer)
            actionPerformer = owner;

        bool allDoorsWereAlreadyOpen = true; // Assume success initially

        foreach (ScriptedUserAction action : doorActions)
        {
            IEntity actionOwner = action.GetOwner();
            if (!actionOwner)
                continue;

            DoorComponent doorComp = DoorComponent.Cast(actionOwner.FindComponent(DoorComponent));
            if (!doorComp)
                continue;

            GRAD_DoorLockComponent doorLock = GRAD_DoorLockComponent.Cast(actionOwner.FindComponent(GRAD_DoorLockComponent)); // Get lock component from the action owner
            bool isLocked = false;
            if (doorLock) {
                isLocked = doorLock.GetLockState();
            }

            // Using the original's check for "is open" for robustness, can be adapted if IsOpen() is preferred.
            // bool isTotallyOpen = Math.AbsFloat(doorComp.GetAngleRange() - doorComp.GetDoorState()) < ANGLE_EPSILON;
            // If using your simpler IsOpen():
            bool isCurrentlyOpen = doorComp.IsOpen();


            if (isLocked)
            {
                if (!isCurrentlyOpen) // If it's locked AND closed
                {
                    allDoorsWereAlreadyOpen = false; // This door is an obstacle we can't overcome right now
                }
                // If locked, we can't do anything with it, so continue to the next door (if any)
                // If this was the *only* door or the primary door, the initial check or the final return will handle it.
                continue;
            }

            // If not locked:
            if (!isCurrentlyOpen) // If it's not open (and we know it's not locked)
            {
                if (!doorComp.IsOpening()) // And not already in the process of opening
                {
                    action.PerformAction(actionOwner, actionPerformer);
                    // Action performed, so the task is now running to wait for the door to open.
                    return ENodeResult.RUNNING; // IMPORTANT: Return RUNNING
                }
                else
                {
                    // Door is already opening. Task is also considered running.
                    allDoorsWereAlreadyOpen = false; // It's not *statically* open yet
                    // No need to return RUNNING here if another door might also need action,
                    // but the original would return RUNNING if *any* action was taken or door was opening.
                    // For simplicity and correctness, if any door is currently opening, the task is RUNNING.
                    return ENodeResult.RUNNING;
                }
            }
            // If doorComp.IsOpen() is true and not locked, it's fine. Loop continues.
            // allDoorsWereAlreadyOpen remains true (if it was true before this iteration and this door is open).
        }

        // After checking all doors:
        if (allDoorsWereAlreadyOpen)
        {
            // All doors we could interact with (i.e., not locked or already open) were indeed already open.
            return ENodeResult.SUCCESS;
        }
        else
        {
            // This means at least one door was not open.
            // If we initiated an action or found a door opening, we would have returned RUNNING.
            // So, if we reach here, it implies any doors that were not open were locked (and we skipped them).
            // In this scenario, the task has failed to open all desired doors because of locks.
            return ENodeResult.FAIL;
        }
    }
};