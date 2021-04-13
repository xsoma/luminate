var last_shot_tick = 0
var in_recharge = false
var shot = false
var was_not_dt = false
UI.AddSliderInt("Shift amount", 8, 14)
UI.AddSliderInt("Tolerance", 1, 8)
UI.AddCheckbox("Fast recharge")
UI.AddSliderInt("Recharge time", 0, 64)
function weapon_fire() {
    if (Entity.GetEntityFromUserID(Event.GetInt("userid")) == Entity.GetLocalPlayer()) {
        last_shot_tick = Globals.Tickcount()
        shot = true
    }
}
function cm() {
    
    Exploit.OverrideShift(UI.GetValue("Script items", "Shift amount"))
    Exploit.OverrideTolerance(UI.GetValue("Script items", "Tolerance"))
    var dt = UI.IsHotkeyActive("Rage", "GENERAL", "Exploits", "Doubletap") && UI.GetValue("Rage", "GENERAL", "Exploits", "Doubletap")
    var hs = UI.IsHotkeyActive("Rage", "GENERAL", "Exploits", "Hide shots") && UI.GetValue("Rage", "GENERAL", "Exploits", "Hide shots")
    if(!UI.GetValue("Script items", "Fast recharge") || (hs && !dt)) {
        Exploit.EnableRecharge()
        return
    }
    Exploit.DisableRecharge()
    var charge = Exploit.GetCharge()
    if (charge != 1) {
        if (UI.GetValue("Anti-Aim", "Extra", "Fake duck") || 
            !dt)
        {
            was_not_dt = true
            return
        }
        if (Globals.Tickcount() - last_shot_tick > UI.GetValue("Script items", "Recharge time") && shot) {
            in_recharge = true
            shot = false
        }
        if(was_not_dt)
        {
            was_not_dt = false
            in_recharge = true
        }
    }
    if (in_recharge) {
        Exploit.Recharge()
        if (charge == 1) {
            in_recharge = false
        }
    }
}
function unload() {
    Exploit.EnableRecharge()
    Exploit.OverrideShift(12)
    Exploit.OverrideTolerance(2)
}
function round_start() {
    last_shot_tick = 0
    in_recharge = true
}
Cheat.RegisterCallback("round_start", "round_start")
Cheat.RegisterCallback("Unload", "unload")
Cheat.RegisterCallback("CreateMove", "cm")
Cheat.RegisterCallback("weapon_fire", "weapon_fire")