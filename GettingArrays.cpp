
#include "sierrachart.h"


SCDLLName("Getting Arrays DLL")





SCSFExport scsf_GetStudyArrayFromChartUsingID(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_Plot = sc.Subgraph[0];
	
	SCInputRef StudySubgraphReference = sc.Input[0];

	if (sc.SetDefaults)
	{
		sc.GraphName = "Get Study Array From Chart Using ID";
		sc.StudyDescription = "Get a Study Array from a Chart using ID";
		sc.AutoLoop = 1;
		
		Subgraph_Plot.Name = "Plot";
		Subgraph_Plot.DrawStyle = DRAWSTYLE_POINT;
		Subgraph_Plot.PrimaryColor = RGB(128,128,128);
		Subgraph_Plot.DrawZeros = 1;
		
		StudySubgraphReference.Name = "Study Subgraph Reference";
		StudySubgraphReference.SetChartStudySubgraphValues(1, 1, 0);
	
		return;
	}
	
	
	SCFloatArray StudyArray;
	
	sc.GetStudyArrayFromChartUsingID(StudySubgraphReference.GetChartStudySubgraphValues(), StudyArray);
	if (StudyArray.GetArraySize() == 0)
		return;  
	
	Subgraph_Plot[sc.Index] = StudyArray[sc.Index];
}


// 



SCSFExport scsf_GetStudyArrayUsingID(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_Plot = sc.Subgraph[0];
	
	SCInputRef StudyReference = sc.Input[0];
	
	if (sc.SetDefaults)
	{
		sc.GraphName = "Get Study Array Using ID";
		sc.StudyDescription = "Get a Study Array from Same Chart using ID";
		sc.AutoLoop = 1;
		
		Subgraph_Plot.Name = "Plot";
		Subgraph_Plot.DrawStyle = DRAWSTYLE_POINT;
		Subgraph_Plot.PrimaryColor = RGB(128,128,128);
		Subgraph_Plot.DrawZeros = 1;
		
		StudyReference.Name = "Study Subgraph Reference";
		StudyReference.SetStudySubgraphValues(0, 0);
		
		return;
	}
	
	SCFloatArray StudyArray;
	
	sc.GetStudyArrayUsingID(StudyReference.GetStudyID(), StudyReference.GetSubgraphIndex(), StudyArray);
	if (StudyArray.GetArraySize() == 0)
		return;  
	
	Subgraph_Plot[sc.Index] = StudyArray[sc.Index];
}



//


SCSFExport scsf_GetStudyExtraArrayFromChartUsingID(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_Plot = sc.Subgraph[0];

   int ip = 0;
    
	SCInputRef StudyChartNumber = sc.Input[ip++];
	SCInputRef SubgraphReference = sc.Input[ip++];
    SCInputRef ExtraArryNumber = sc.Input[ip++];
	
	
	if (sc.SetDefaults)
	{
		
		sc.GraphName = "Get Study Extra Array From Chart Using ID";
		sc.StudyDescription = "Get an extra eubgraph array from a chart using ID2";
		sc.AutoLoop = 1;
		
		Subgraph_Plot.Name = "Plot";
		Subgraph_Plot.DrawStyle = DRAWSTYLE_POINT;
		Subgraph_Plot.PrimaryColor = RGB(128,128,128);
		Subgraph_Plot.DrawZeros = 1;
		
		StudyChartNumber.Name = "Chart Number"; 
		StudyChartNumber.SetChartNumber(1);
		   
		SubgraphReference.Name = "Study Subgraph Reference";
		SubgraphReference.SetStudySubgraphValues(0, 0);
		
		ExtraArryNumber.Name = "Extra Array";    
		ExtraArryNumber.SetInt(0);
		
		return;
	}
	
	
	SCFloatArray StudyArray;
	
	sc.GetStudyExtraArrayFromChartUsingID(StudyChartNumber.GetChartNumber(),SubgraphReference.GetStudyID(), SubgraphReference.GetSubgraphIndex(),ExtraArryNumber.GetInt(),StudyArray);
	if (StudyArray.GetArraySize() == 0)
		return;  
	
	Subgraph_Plot[sc.Index] = StudyArray[sc.Index];
}

