[ComponentEditorProps(category: "Custom", description: "Door Lock Component")]
class DoorLockComponentClass : ScriptComponentClass {
}

class DoorLockComponent : ScriptComponent
{
    // Variables exposed to the editor
    [Attribute("0", UIWidgets.CheckBox, "Door Lock State", "" )]
    bool m_isLocked;

    // Called when the game initializes the component
    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);
        Print("Door Lock Component Initialized with: " + m_isLocked.ToString());
    }
}