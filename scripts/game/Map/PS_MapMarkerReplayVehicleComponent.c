
class PS_MapMarkerReplayVehicleComponent : PS_MapMarkerReplayEntityComponent
{
	EVehicleType m_eVehicleType;
	ResourceName m_rImageSet = "{ED7A1DA5BC4CCBA0}UI/Icons/PS_Atlas.imageset";
	
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
		}
		
	}
}