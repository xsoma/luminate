UI.AddSliderFloat("Noscope distance (m)", 0, 100);
var target = -1;
function CreateMove() {
    if(!Ragebot.GetTarget())
        target = closestTarget();
    else
        target = Ragebot.GetTarget();
    if(!Entity.IsAlive(target)) {
        UI.SetValue("Rage", "GENERAL", "General", "Auto scope", true);
        return;
    }
    if(get_metric_distance(Entity.GetRenderOrigin(Entity.GetLocalPlayer()), Entity.GetRenderOrigin(target)) < UI.GetValue("Script items", "Noscope distance (m)")) {
        UI.SetValue("Rage", "GENERAL", "General", "Auto scope", false);
    } else {
        UI.SetValue("Rage", "GENERAL", "General", "Auto scope", true);
    }
}
Cheat.RegisterCallback("CreateMove", "CreateMove");
function closestTarget() {
    var local = Entity.GetLocalPlayer();
    var enemies = Entity.GetEnemies();
    var dists = [];
    var damage = [];
    for(e in enemies) {
        if(!Entity.IsAlive(enemies[e]) || Entity.IsDormant(enemies[e]) || !Entity.IsValid(enemies[e])) continue;
        dists.push([enemies[e], calcDist(Entity.GetHitboxPosition(local, 0), Entity.GetHitboxPosition(enemies[e], 0))]);
    }
    dists.sort(function(a, b)
    {
        return a[1] - b[1];
    });
    if(dists.length == 0 || dists == []) return target = -1; 
    return dists[0][0];
}

// clean dist func, thanks rzr
function calcDist(a, b)
{
    x = a[0] - b[0];
    y = a[1] - b[1];
    z = a[2] - b[2];
    return Math.sqrt( x * x + y * y + z * z );
}

function get_metric_distance(a, b)
{
    return Math.floor(Math.sqrt(Math.pow(a[0] - b[0], 2) + Math.pow(a[1] - b[1], 2) + Math.pow(a[2] - b[2], 2)) * 0.0254 );
}