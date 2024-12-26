//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_lockComponent;
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			ShowHint("no owner", "No owner");
			return;
		}
           
		DoorComponent doorComp = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
		if (!doorComp) {
			Print("No doorComp found in DoorLockAction");
			return;
		} else {
			m_door = doorComp;
			Print("m_door filled with doorComp");
			
			m_lockComponent = GRAD_DoorLockComponent.Cast(m_door.FindComponent(GRAD_DoorLockComponent));
			if (!m_lockComponent) {
				Print("m_lockComponent NOT found");
				return;
			} else {
				m_lockComponent.ToggleLockState();
				Print("Toggling door lock");	
			}
		}          
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
