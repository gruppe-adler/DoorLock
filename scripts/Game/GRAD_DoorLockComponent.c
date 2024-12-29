[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class GRAD_DoorLockComponentClass : ScriptComponentClass {
}

class GRAD_DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
	[RplProp()]  
    bool m_isLocked;
	string m_lockOwner = "";

    // Called when the game initializes the component
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        // Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
    }
	
	bool GetLockState() 
	{
		return m_isLocked;
	}
	
	string GetLockOwner() 
	{
		return m_lockOwner;
	}
	
	void ToggleLockState(IEntity pUserEntity)
	{
		
		// get user identity to set lock side
		if (!m_lockOwner) {
			if (pUserEntity) {
				PrintFormat("pUserEntity: %1", pUserEntity);
				if (SCR_EditorManagerEntity.IsOpenedInstance()) {
					Print("Setting LockOwner to GM");
					SetLockOwner("GM");
				} else {
					SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(pUserEntity);
					if (character) {
						SetLockOwner(character.GetFactionKey());
						PrintFormat("Setting Lock to ", character.GetFactionKey());
					}
				};
			} else {
				Print("pUserEntity is empty");
			}
		}
		
		if (m_isLocked) {
			AudioSystem.PlaySound("{8636DEE6CA8BB1AE}sounds/grad_doorlock_1.wav");
			m_isLocked = false;
			Replication.BumpMe();
		} else {
			AudioSystem.PlaySound("{8636DEE6CA8BB1AE}sounds/grad_doorlock_2.wav");
			m_isLocked = true;
			Replication.BumpMe();
		}
		Print("Door Lock Component set to: " + m_isLocked.ToString());		
	}
	
	void SetLockOwner(string lockOwner) {
		m_lockOwner = lockOwner;
		PrintFormat("Lock Owner set to %1", lockOwner);
	}
}