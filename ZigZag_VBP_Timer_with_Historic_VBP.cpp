#include "sierrachart.h"

SCDLLName("VolumeProfileTimer_StaticHistoric DLL")

/* This is a general study to make the Sierra ZigZag indicator update the Sierra VolumebyPrice indicator */
/* Iteration: When using two VBPs, VBP1 shows the current swing, and VBP2 shows the prior swing  */

void UpdateVBPStudies(SCStudyInterfaceRef sc, int vbpID1, int vbpID2, int numberOfProfiles, SCDateTime currentSwingTime, SCDateTime priorSwingTime)
{
    // Update VBP1 with the current swing time
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 36, currentSwingTime.GetDateAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 38, currentSwingTime.GetTimeAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputInt(sc.ChartNumber, vbpID1, 32, 5);

    // Update VBP2 with the prior swing time only if two profiles are selected
    if (numberOfProfiles == 1)
    {  
		sc.SetChartStudyInputInt(sc.ChartNumber, vbpID2, 32, 8);
		sc.SetChartStudyInputInt(sc.ChartNumber, vbpID2, 38, 1);
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 36, priorSwingTime.GetDateAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 38, priorSwingTime.GetTimeAsSCDateTime().GetAsDouble());
		sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 39, currentSwingTime.GetDateAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 40, currentSwingTime.GetTimeAsSCDateTime().GetAsDouble());  
    }
}

SCSFExport scsf_VolumeProfileTimerStaticHistoric(SCStudyInterfaceRef sc)
{
    int ip = 0;

    SCInputRef ZigZagStudyRef = sc.Input[ip++];
    SCInputRef Input_NumberofProfiles = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef1 = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef2 = sc.Input[ip++];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Volume Profile Timer with Static Historic.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        ZigZagStudyRef.Name = "Zig Zag Study Ref";
        ZigZagStudyRef.SetStudyID(0);
		
        Input_NumberofProfiles.Name = "Number of Volume Profiles";
        Input_NumberofProfiles.SetCustomInputStrings("One;Two");
        Input_NumberofProfiles.SetCustomInputIndex(0);

        VolumeByPriceStudyRef1.Name = "Volume by Price Study Ref 1";
        VolumeByPriceStudyRef1.SetStudyID(0);

        VolumeByPriceStudyRef2.Name = "Volume by Price Study Ref 2";
        VolumeByPriceStudyRef2.SetStudyID(0);

        return;
    }

    int zzID = ZigZagStudyRef.GetStudyID();
    int vbpID1 = VolumeByPriceStudyRef1.GetStudyID();
    int vbpID2 = VolumeByPriceStudyRef2.GetStudyID();

    SCFloatArray ZigZagPeakType;
    SCFloatArray ZigZagPeakIndex;
    sc.GetStudyExtraArrayFromChartUsingID(sc.ChartNumber, zzID, 0, 0, ZigZagPeakType);
    sc.GetStudyExtraArrayFromChartUsingID(sc.ChartNumber, zzID, 0, 1, ZigZagPeakIndex);

    int Index = sc.Index;

    int& lastHighIndex = sc.GetPersistentInt(1);
    int& lastLowIndex = sc.GetPersistentInt(2);
    SCDateTime& lastHighTime = sc.GetPersistentSCDateTime(1);
    SCDateTime& lastLowTime = sc.GetPersistentSCDateTime(2);
    int& lastPeakIndex = sc.GetPersistentInt(3);
    float& lastPeakType = sc.GetPersistentFloat(1);
    int& moveDirection = sc.GetPersistentInt(4);
    // New persistent variable to store the prior swing's start time
    SCDateTime& priorSwingTime = sc.GetPersistentSCDateTime(3);
    int CurrentPeakIndex = 0;
    int BackRefIndex = 0;

   
            if (ZigZagPeakType[Index] == 1.0f || ZigZagPeakType[Index] == -1.0f)
            {
                CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex[Index]);
                BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType[Index] == 1.0f && ZigZagPeakType[BackRefIndex] == -1.0f && lastPeakType != 1.0f)
                    {
                        // High after low: VBP1 uses lastLowTime, VBP2 uses prior high time
                        priorSwingTime = lastHighTime; // Store the high time before updating
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex = BackRefIndex;
                        lastLowTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastLowTime, priorSwingTime);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && ZigZagPeakType[BackRefIndex] == 1.0f && lastPeakType != -1.0f)
                    {
                        // Low after high: VBP1 uses lastHighTime, VBP2 uses prior low time
                        priorSwingTime = lastLowTime; // Store the low time before updating
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex = BackRefIndex;
                        lastHighTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastHighTime, priorSwingTime);
                    }
                }
                else
                {
                    if (ZigZagPeakType[Index] == 1.0f && lastPeakType == 0.0f)
                    {
                        // First high: VBP1 uses sc.BaseDateTimeIn[0], VBP2 uses same (no prior swing)
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0], sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && lastPeakType == 0.0f)
                    {
                        // First low: VBP1 uses sc.BaseDateTimeIn[0], VBP2 uses same (no prior swing)
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0], sc.BaseDateTimeIn[0]);
                    }
                }
            }
  
}