//! User action that ought to be attached to an entity with door component.
//! When performed either locks or unlcoks the door based on the previous state of the door.
// Script File
class GRAD_DoorLockAction : ScriptedUserAction
{
	DoorComponent m_door;
	GRAD_DoorLockComponent m_lockComponent;
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent) {
		
		DoorComponent doorComp = DoorComponent.Cast(pOwnerEntity.FindComponent(DoorComponent));
		
		if (!doorComp) {
			Print("No doorComp found in DoorLockAction");
			return;
		} else {
			m_door = doorComp;
			Print("m_door filled with doorComp");
			
			m_lockComponent = GRAD_DoorLockComponent.Cast(pOwnerEntity.FindComponent(GRAD_DoorLockComponent));
			if (!m_lockComponent) {
				Print("m_lockComponent NOT initially set");
				return;
			} else {
				PrintFormat("m_lockComponent initially set to %1", m_lockComponent.GetLockState());
			}
		}
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (!m_lockComponent)
			return false;
		
		if (m_lockComponent.GetLockState()) { 
			outName = "Unlock";
			return true;
		} else { 
			outName = "Lock";
			return true;
		};		
	}
	
    //------------------------------------------------------------------------------------------------
    override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
    {
		if (!pOwnerEntity) {
			ShowHint("no owner", "No owner");
			return;
		}    
		
		if (!m_lockComponent) {
			Print("m_lockComponent NOT set at runtime");
			return;
		} else {
			m_lockComponent.ToggleLockState();
			Print("Toggling door lock");
			
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
