UI.AddCheckbox("animfucker");
UI.AddSliderInt("animfucker speed", 1, 10);
var old_tick_count = 0;
function draw()
{
    if (UI.GetValue("Script items", "animfucker") && (Globals.Tickcount() - old_tick_count) > (UI.GetValue("Script items", "animfucker speed")))
    {
        if (UI.GetValue("Misc", "GENERAL", "Movement", "Slide walk"))
            UI.SetValue("Misc", "GENERAL", "Movement", "Slide walk", 0);
        else
            UI.SetValue("Misc", "GENERAL", "Movement", "Slide walk", 1);
        old_tick_count = Globals.Tickcount();
    }
}
Cheat.RegisterCallback("Draw", "draw");