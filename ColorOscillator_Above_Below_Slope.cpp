#include "sierrachart.h"

SCDLLName("ColorOscillator DLL")

SCSFExport scsf_GetStudyArrayUsingID(SCStudyInterfaceRef sc)
{
	SCSubgraphRef Subgraph_Plot = sc.Subgraph[0];
	
	int ip = 0;
	
	SCInputRef StudyReference = sc.Input[ip++];
	
	SCInputRef Color1Input = sc.Input[ip++];
    SCInputRef Color2Input = sc.Input[ip++];
    SCInputRef Color3Input = sc.Input[ip++];
    SCInputRef Color4Input = sc.Input[ip++];
	
	if (sc.SetDefaults)
	{
		sc.GraphName = "Color Oscillator";
		sc.StudyDescription = "Color Oscillator Above / Below 0 & Slope";
		sc.AutoLoop = 1;
		
		Subgraph_Plot.Name = "Plot";
		Subgraph_Plot.DrawStyle = DRAWSTYLE_BAR;
		Subgraph_Plot.LineWidth = 3;
		Subgraph_Plot.PrimaryColor = RGB(255,255,255);
		Subgraph_Plot.DrawZeros = 1;
		
		StudyReference.Name = "Study Subgraph Reference";
		StudyReference.SetStudySubgraphValues(0, 0);
		
		Color1Input.Name = "Above 0 Color";
        Color1Input.SetColor(255,255,255);
		
		Color2Input.Name = "Above 0 Slope Down Color";
        Color2Input.SetColor(255,255,255);
		
		Color3Input.Name = "Below 0 Color";
        Color3Input.SetColor(255,255,255);
		
		Color4Input.Name = "Below 0 Slope Down Color";
        Color4Input.SetColor(255,255,255);
		
		return;
	}
	
	SCFloatArray StudyArray;
	
	sc.GetStudyArrayUsingID(StudyReference.GetStudyID(), StudyReference.GetSubgraphIndex(), StudyArray);
	if (StudyArray.GetArraySize() == 0)
		return;  
	
	Subgraph_Plot[sc.Index] = StudyArray[sc.Index];
	
	if(StudyArray[sc.Index] > 0 && StudyArray[sc.Index] > StudyArray[sc.Index-1])
	{
		Subgraph_Plot.DataColor[sc.Index] = Color1Input.GetColor();
	}
	else if(StudyArray[sc.Index] > 0 && StudyArray[sc.Index] < StudyArray[sc.Index-1])
	{
		Subgraph_Plot.DataColor[sc.Index] = Color2Input.GetColor();
	}
	else if(StudyArray[sc.Index] < 0 && StudyArray[sc.Index] < StudyArray[sc.Index-1])
	{
		Subgraph_Plot.DataColor[sc.Index] = Color3Input.GetColor();
	}
	else if(StudyArray[sc.Index] < 0 && StudyArray[sc.Index] > StudyArray[sc.Index-1])
	{
		Subgraph_Plot.DataColor[sc.Index] = Color4Input.GetColor();
	}
	
}





