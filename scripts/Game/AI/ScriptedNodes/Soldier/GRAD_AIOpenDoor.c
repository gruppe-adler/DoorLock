modded class SCR_AIOpenDoor : AITaskScripted
{
    const string LOGTAG = "[GRAD_AIOpenDoor] ";

    // ── 1) Filter out Lock/Unlock at the very source ────────────────────────────
    override static ScriptedUserAction FindDoorUserAction(notnull IEntity entity)
    {
        Print(LOGTAG + "FindDoorUserAction on: " + entity);

        ActionsManagerComponent amc = ActionsManagerComponent.Cast(entity.FindComponent(ActionsManagerComponent));
        if (!amc)
        {
            Print(LOGTAG + " → no ActionsManagerComponent");
            return null;
        }

        array<BaseUserAction> raw = {};
        amc.GetActionsList(raw);
        Print(LOGTAG + " → total user‐actions: " + raw.Count());

        foreach (BaseUserAction b : raw)
        {
			SCR_DoorUserAction doorAct = SCR_DoorUserAction.Cast(b);
            if (doorAct)
            {
                Print(LOGTAG + "    - skipping LOCK action: " + b.GetActionName());
                continue;
            }

            // single declaration; only pick if unlocked
	
	        if (doorAct && !GRAD_DoorLockComponent.Cast(doorAct.GetOwner().FindComponent(GRAD_DoorLockComponent)).GetLockState())
	        {
	            Print(LOGTAG + "    + selecting OPEN/CLOSE action: " + doorAct.GetActionName());
	            return doorAct;
	        }
        }

        Print(LOGTAG + " → no open/close action found");
        return null;
    }

    // ── 2) Your simulate loop, with logs at every branch ────────────────────────
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
        Print(LOGTAG + "EOnTaskSimulate() start for agent: " + owner);

        // 2.1) fetch the door
        IEntity doorEnt;
        GetVariableIn(PORT_DOOR_ENTITY, doorEnt);
        if (!doorEnt)
        {
            Print(LOGTAG + " → FAIL: no door entity");
            return ENodeResult.FAIL;
        }
        Print(LOGTAG + " → doorEntity = " + doorEnt);

        // 2.3) pick the one “open/close” action (our override of FindDoorUserAction has already killed lock/unlock)
        array<ScriptedUserAction> acts = {};
        super.FindDoorActions(doorEnt, acts, m_bOpenSiblingDoors);
        Print(LOGTAG + " → post‐filter actions: " + acts.Count());
        if (acts.IsEmpty())
        {
            Print(LOGTAG + " → FAIL: no actions available");
            return ENodeResult.FAIL;
        }

        // 2.4) resolve who’s doing it
        IEntity performer = owner.GetControlledEntity();
        if (!performer)
        {
            performer = owner;
            Print(LOGTAG + " → performer fallback to agent");
        }
        else
        {
            Print(LOGTAG + " → performer = " + performer);
        }

        // 2.5) loop through the (filtered) open/close actions
        foreach (ScriptedUserAction a : acts)
        {
            IEntity ao = a.GetOwner();
            Print(LOGTAG + " → considering action “" + a.GetActionName() + "” on " + ao);

            DoorComponent dc = DoorComponent.Cast(ao.FindComponent(DoorComponent));
            GRAD_DoorLockComponent lc = GRAD_DoorLockComponent.Cast(ao.FindComponent(GRAD_DoorLockComponent));

            // sibling lock check
            if (dc && lc && lc.GetLockState() && !dc.IsOpen())
            {
                Print(LOGTAG + "    - sibling door locked, skipping");
                continue; // try next door
            }

            // perform open if needed
            if (dc && !dc.IsOpen() && !dc.IsOpening())
            {
                Print(LOGTAG + "    - PERFORM: “" + a.GetActionName() + "”");
                a.PerformAction(ao, performer);
                return ENodeResult.RUNNING;
            }
            else
            {
                Print(LOGTAG + "    - already open/opening, skipping");
            }
        }

        Print(LOGTAG + " → SUCCESS: all doors open or opening");
        return ENodeResult.SUCCESS;
    }
}
