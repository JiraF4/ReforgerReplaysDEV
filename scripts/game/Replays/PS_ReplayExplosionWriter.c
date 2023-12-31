class PS_ReplayExplosionWriterClass: ScriptComponentClass
{
	
};

class PS_ReplayExplosionWriter : ScriptComponent
{
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(owner.FindComponent(BaseTriggerComponent));
		array<BaseProjectileEffect> projectileEffects = new array<BaseProjectileEffect>();
		trigger.GetProjectileEffects(BaseProjectileEffect, projectileEffects);
		
		if (projectileEffects.Count() > 0)
		{
			PS_ReplayWriter replayWriter = PS_ReplayWriter.GetInstance();
			vector origin = owner.GetOrigin();
			replayWriter.WriteExplosion(origin[0], origin[2], 10.0);
		}
	}
}