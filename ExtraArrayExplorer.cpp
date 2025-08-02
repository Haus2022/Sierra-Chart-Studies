
#include "sierrachart.h"


SCDLLName("Extra Array Explorer DLL")


SCSFExport scsf_ExtraArrayExplorer(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_Plot = sc.Subgraph[0];
	
	SCInputRef Input_1 = sc.Input[0];
	SCInputRef Input_2 = sc.Input[1];
	SCInputRef Input_3 = sc.Input[2];
	
	if (sc.SetDefaults)
	{
		
		sc.GraphName = "Extra Array Explorer";
		sc.StudyDescription = "Explore Extra Arrays";
		
		Subgraph_Plot.Name = "Plot";
		Subgraph_Plot.DrawStyle = DRAWSTYLE_POINT;
		Subgraph_Plot.PrimaryColor = RGB(128,128,128);
		Subgraph_Plot.DrawZeros = 1;
		sc.AutoLoop = 1;
		
		
		Input_1.Name = "1";
		Input_1.SetInt(1);
		
		Input_2.Name = "2";
		Input_2.SetInt(0);
		
		Input_3.Name = "3";
		Input_3.SetInt(0);
		
		
		return;
	}
	
	

	SCFloatArray StudyArray;
	
	
	sc.GetStudyExtraArrayFromChartUsingID(sc.ChartNumber, Input_1.GetInt(), Input_2.GetInt(), Input_3.GetInt(), StudyArray);
	if (StudyArray.GetArraySize() == 0)
		return;  
	
	Subgraph_Plot[sc.Index] = StudyArray[sc.Index];
}

