#include "sierrachart.h"

SCDLLName("VolumeProfileTimer DLL")

/* This is a general study to make the Sierra ZigZag indicator update the Sierra VolumebyPrice indicator */

void UpdateVBPStudies(SCStudyInterfaceRef sc, int vbpID1, int vbpID2, int numberOfProfiles, SCDateTime dateTime)
{
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 36, dateTime.GetDateAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 38, dateTime.GetTimeAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputInt(sc.ChartNumber, vbpID1, 32, 5);

    if (numberOfProfiles == 1)
    {
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 36, dateTime.GetDateAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 38, dateTime.GetTimeAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputInt(sc.ChartNumber, vbpID2, 32, 5);
    }
}

SCSFExport scsf_VolumeProfileTimer(SCStudyInterfaceRef sc)
{
    int ip = 0;

    SCInputRef ZigZagStudyRef = sc.Input[ip++];
    SCInputRef Input_UpdateBehaviour = sc.Input[ip++];
    SCInputRef Input_NumberofProfiles = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef1 = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef2 = sc.Input[ip++];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Volume Profile Timer";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        ZigZagStudyRef.Name = "Zig Zag Study Ref";
        ZigZagStudyRef.SetStudyID(0);

        Input_UpdateBehaviour.Name = "Update Behaviour";
        Input_UpdateBehaviour.SetCustomInputStrings("Wait;Increment");
        Input_UpdateBehaviour.SetCustomInputIndex(0);

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
    int CurrentPeakIndex = 0;
    int BackRefIndex = 0;

    switch (Input_UpdateBehaviour.GetIndex())
    {
        case 0:  // Wait for threshold
            if (ZigZagPeakType[Index] == 1.0f || ZigZagPeakType[Index] == -1.0f)
            {
                CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex[Index]);
                BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType[Index] == 1.0f && ZigZagPeakType[BackRefIndex] == -1.0f && lastPeakType != 1.0f)
                    {
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex = BackRefIndex;
                        lastLowTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastLowTime);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && ZigZagPeakType[BackRefIndex] == 1.0f && lastPeakType != -1.0f)
                    {
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex = BackRefIndex;
                        lastHighTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastHighTime);
                    }
                }
                else
                {
                    if (ZigZagPeakType[Index] == 1.0f && lastPeakType == 0.0f)
                    {
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && lastPeakType == 0.0f)
                    {
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0]);
                    }
                }
            }
            break;

        case 1:  // Incrementally updates as direction continues
            if (ZigZagPeakType[Index] == 1.0f || ZigZagPeakType[Index] == -1.0f)
            {
                CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex[Index]);
                BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType[Index] == 1.0f && ZigZagPeakType[BackRefIndex] == -1.0f)
                    {
                        moveDirection = 1;
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex = BackRefIndex;
                        lastLowTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastHighTime);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && ZigZagPeakType[BackRefIndex] == 1.0f)
                    {
                        moveDirection = -1;
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex = BackRefIndex;
                        lastHighTime = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastLowTime);
                    }
                    else if (moveDirection == 1 && ZigZagPeakType[Index] == 1.0f)
                    {
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastHighTime);
                    }
                    else if (moveDirection == -1 && ZigZagPeakType[Index] == -1.0f)
                    {
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), lastLowTime);
                    }
                }
                else
                {
                    if (ZigZagPeakType[Index] == 1.0f && lastPeakType == 0.0f)
                    {
                        moveDirection = 1;
                        lastHighIndex = CurrentPeakIndex;
                        lastHighTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType[Index] == -1.0f && lastPeakType == 0.0f)
                    {
                        moveDirection = -1;
                        lastLowIndex = CurrentPeakIndex;
                        lastLowTime = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex = CurrentPeakIndex;
                        lastPeakType = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetIndex(), sc.BaseDateTimeIn[0]);
                    }
                }
            }
            break;
    }
}