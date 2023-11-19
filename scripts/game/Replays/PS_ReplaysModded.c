modded class SCR_EditableVehicleComponent
{
	RplId m_iRemoveRpl;
	void ~SCR_EditableVehicleComponent()
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (!GetGame().GetWorld())
			return;
		
		if (!Replication.IsServer())
			return;
		
		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;
		
		RplComponent rpl = RplComponent.Cast(this.FindComponent(RplComponent));
		PS_ReplayWriter.GetInstance().WriteEntityDelete(m_iRemoveRpl);
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode())
			return;
		GetGame().GetCallqueue().CallLater(RegisterToReplay, 0, false, owner);
	}
	
	void RegisterToReplay(IEntity owner)
	{
		SCR_UIInfo uIInfo = GetInfo();
		Vehicle vehicle = Vehicle.Cast(owner);
		RplComponent Rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		PS_ReplayWriter replayWriter = PS_ReplayWriter.GetInstance();
		RplId rplId = Rpl.Id();
		SCR_VehicleFactionAffiliationComponent factionComponent = SCR_VehicleFactionAffiliationComponent.Cast(owner.FindComponent(SCR_VehicleFactionAffiliationComponent));
		Faction faction = factionComponent.GetDefaultAffiliatedFaction();
		FactionKey factionKey = "";
		if (faction) factionKey = faction.GetFactionKey();
		string name = "";
		if (uIInfo) name = uIInfo.GetName();
		replayWriter.WriteVehicleRegistration(rplId, name, vehicle.m_eVehicleType, factionKey);
		
		m_iRemoveRpl = Rpl.Id();
		
		GetGame().GetCallqueue().CallLater(PositionLogger, 100, false, rplId, owner);
	}
	
	protected void PositionLogger(RplId rplId, IEntity owner)
	{
		// Regulary write position to replay
		PS_ReplayWriter.GetInstance().WriteEntityMove(rplId, owner);
		GetGame().GetCallqueue().CallLater(PositionLogger, 500, false, rplId, owner);
	}
}

modded class SCR_ChimeraCharacter 
{
	void ~SCR_ChimeraCharacter()
	{
		if (!GetGame().InPlayMode())
			return;
		
		if (!GetGame().GetWorld())
			return;
		
		if (!Replication.IsServer())
			return;
		
		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;
		
		RplComponent rpl = RplComponent.Cast(this.FindComponent(RplComponent));
		PS_ReplayWriter.GetInstance().WriteEntityDelete(rpl.Id());
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(this);
		if (!Replication.IsServer()) return; // Register character to replay
		if (!GetGame().InPlayMode())
			return;
		GetGame().GetCallqueue().CallLater(RegisterToReplay, 0, false);
	}
	
	void RegisterToReplay()
	{
		if (PS_ReplayWriter.GetInstance()) {
			RplComponent rpl = RplComponent.Cast(this.FindComponent(RplComponent));
			SCR_CharacterDamageManagerComponent damageComponent = SCR_CharacterDamageManagerComponent.Cast(this.FindComponent(SCR_CharacterDamageManagerComponent));
			ScriptInvoker damageEvent = damageComponent.GetOnDamageStateChanged();
			damageEvent.Insert(DieLogger);
			PS_ReplayWriter.GetInstance().WriteCharacterRegistration(rpl.Id(), this);
			GetGame().GetCallqueue().CallLater(PositionLogger, 0, false, rpl.Id());
		}
	}
	
	protected void PositionLogger(RplId rplId)
	{
		PS_ReplayWriter.GetInstance().WriteEntityMove(rplId, this);
		GetGame().GetCallqueue().CallLater(PositionLogger, 500, false, rplId);
	}
	
	protected void DieLogger(EDamageState state)
	{
		RplComponent rpl = RplComponent.Cast(this.FindComponent(RplComponent));
		PS_ReplayWriter.GetInstance().WriteCharacterDamageStateChanged(rpl.Id(), state);
	}
}

modded class SCR_PlayerController
{
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (to) {
			RplComponent rpl = RplComponent.Cast(to.FindComponent(RplComponent));
			PS_ReplayWriter.GetInstance().WriteCharacterPossess(rpl.Id(), GetPlayerId());
		} else {
			PS_ReplayWriter.GetInstance().WriteCharacterPossess(RplId.Invalid(), GetPlayerId());
		}
		super.OnControlledEntityChanged(from, to);
	}
}