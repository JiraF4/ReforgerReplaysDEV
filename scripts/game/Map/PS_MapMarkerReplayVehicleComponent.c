
class PS_MapMarkerReplayVehicleComponent : PS_MapMarkerReplayEntityComponent
{
	EVehicleType m_eVehicleType;
	ResourceName m_rImageSet = "{629CDE8BEAA36D23}UI/Icons/Replay_Atlas.imageset";
	
	void SetVehicleType(EVehicleType vehicleType)
	{
		m_eVehicleType = vehicleType;
		
		switch(vehicleType)
		{
			case EVehicleType.CAR:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "CAR");
				m_fEntitySize = 8;
				break;
			case EVehicleType.APC:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "APC");
				m_fEntitySize = 14;
				break;
			case EVehicleType.TRUCK:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "TRUCK");
				m_fEntitySize = 14;
				break;
			case EVehicleType.SUPPLY_TRUCK:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "TRUCK");
				m_fEntitySize = 14;
				break;
			case EVehicleType.FUEL_TRUCK:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "TRUCK");
				m_fEntitySize = 14;
				break;
			case EVehicleType.COMM_TRUCK:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "TRUCK");
				m_fEntitySize = 14;
				break;
			case EVehicleType.VEHICLE:
				m_wMarkerIcon.LoadImageFromSet(0, m_rImageSet, "HELICOPTER");
				m_fEntitySize = 24;
				break;
		}
		
	}
}