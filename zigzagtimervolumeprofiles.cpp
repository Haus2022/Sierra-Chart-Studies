#include "sierrachart.h"

SCDLLName("ZigZagVBPTimer DLL")

void UpdateVBPStudies(SCStudyInterfaceRef sc, int vbpID1, int vbpID2, int numberOfProfiles, SCDateTime dateTime)
{
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 36, dateTime.GetDateAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID1, 38, dateTime.GetTimeAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputInt(sc.ChartNumber, vbpID1, 32, 5);

    if (numberOfProfiles >= 1) // Update VBP2 for Two or Three profiles
    {
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 36, dateTime.GetDateAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID2, 38, dateTime.GetTimeAsSCDateTime().GetAsDouble());
        sc.SetChartStudyInputInt(sc.ChartNumber, vbpID2, 32, 5);
    }
}

void UpdateVBP3(SCStudyInterfaceRef sc, int vbpID3, SCDateTime dateTime)
{
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID3, 36, dateTime.GetDateAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputFloat(sc.ChartNumber, vbpID3, 38, dateTime.GetTimeAsSCDateTime().GetAsDouble());
    sc.SetChartStudyInputInt(sc.ChartNumber, vbpID3, 32, 5);
}

SCSFExport scsf_ZigZagVBPTimer(SCStudyInterfaceRef sc)
{
    SCSubgraphRef Subgraph_ZigZag1 = sc.Subgraph[0];
    SCSubgraphRef Subgraph_ZigZag3 = sc.Subgraph[1];

    int ip = 0;
    SCInputRef Input_NumberofProfiles = sc.Input[ip++];
    SCInputRef Input_ReversalType1 = sc.Input[ip++];
    SCInputRef Input_ReversalAmount1 = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef1 = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef2 = sc.Input[ip++];
    SCInputRef Input_ReversalAmount3 = sc.Input[ip++];
    SCInputRef VolumeByPriceStudyRef3 = sc.Input[ip++];

    // Section 1 - Set the configuration variables and defaults
    if (sc.SetDefaults)
    {
        sc.GraphName = "Zig Zag to 3 VBP Timer -";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        Input_NumberofProfiles.Name = "Number of Volume Profiles";
        Input_NumberofProfiles.SetCustomInputStrings("One;Two;Three");
        Input_NumberofProfiles.SetCustomInputIndex(0);

        Input_ReversalType1.Name = "VBP 1 & 2 Reversal Type";
        Input_ReversalType1.SetCustomInputStrings("Percent;Amount");
        Input_ReversalType1.SetCustomInputIndex(0);

        Input_ReversalAmount1.Name = "Reversal x.xx % or x. Amount";
        Input_ReversalAmount1.SetFloat(0.0f);

        VolumeByPriceStudyRef1.Name = "Volume by Price Study Ref 1";
        VolumeByPriceStudyRef1.SetStudyID(0);
        VolumeByPriceStudyRef2.Name = "Volume by Price Study Ref 2";
        VolumeByPriceStudyRef2.SetStudyID(0);

        Input_ReversalAmount3.Name = "Reversal x. Amount (VBP3)";
        Input_ReversalAmount3.SetFloat(0.0f);

        VolumeByPriceStudyRef3.Name = "Volume by Price Study Ref 3";
        VolumeByPriceStudyRef3.SetStudyID(0);

        return;
    }

    int vbpID1 = VolumeByPriceStudyRef1.GetStudyID();
    int vbpID2 = VolumeByPriceStudyRef2.GetStudyID();
    int vbpID3 = VolumeByPriceStudyRef3.GetStudyID();

    SCFloatArrayRef High = sc.High;
    SCFloatArrayRef Low = sc.Low;

    // Persistent variables for ZigZag1 (VBP1 and VBP2)
    int& lastHighIndex1 = sc.GetPersistentInt(1);
    int& lastLowIndex1 = sc.GetPersistentInt(2);
    SCDateTime& lastHighTime1 = sc.GetPersistentSCDateTime(1);
    SCDateTime& lastLowTime1 = sc.GetPersistentSCDateTime(2);
    int& lastPeakIndex1 = sc.GetPersistentInt(3);
    float& lastPeakType1 = sc.GetPersistentFloat(1);
    int& moveDirection1 = sc.GetPersistentInt(4);

    // Persistent variables for ZigZag3 (VBP3)
    int& lastHighIndex3 = sc.GetPersistentInt(5); // Use unique keys
    int& lastLowIndex3 = sc.GetPersistentInt(6);
    SCDateTime& lastHighTime3 = sc.GetPersistentSCDateTime(3);
    SCDateTime& lastLowTime3 = sc.GetPersistentSCDateTime(4);
    int& lastPeakIndex3 = sc.GetPersistentInt(7);
    float& lastPeakType3 = sc.GetPersistentFloat(2);
    int& moveDirection3 = sc.GetPersistentInt(8);

    int Index = sc.Index;

    // Calculate ZigZags and update VBPs in a single switch
    switch (Input_NumberofProfiles.GetInt())
    {
        case 0: // One profile
        case 1: // Two profiles
        {
            // Calculate ZigZag for VBP1 and VBP2
            if (Input_ReversalType1.GetInt() == 0) // Percent
                sc.ZigZag(High, Low, Subgraph_ZigZag1, Input_ReversalAmount1.GetFloat() * 0.01f);
            else // Amount
                sc.ZigZag(High, Low, Subgraph_ZigZag1, 0.0f, Input_ReversalAmount1.GetFloat());

            SCFloatArray ZigZagPeakType1 = Subgraph_ZigZag1.Arrays[0];
            SCFloatArray ZigZagPeakIndex1 = Subgraph_ZigZag1.Arrays[1];

            if (ZigZagPeakType1[Index] == 1.0f || ZigZagPeakType1[Index] == -1.0f)
            {
                int CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex1[Index]);
                int BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex1[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType1[Index] == 1.0f && ZigZagPeakType1[BackRefIndex] == -1.0f && lastPeakType1 != 1.0f)
                    {
                        lastHighIndex1 = CurrentPeakIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex1 = BackRefIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetInt(), lastLowTime1);
                    }
                    else if (ZigZagPeakType1[Index] == -1.0f && ZigZagPeakType1[BackRefIndex] == 1.0f && lastPeakType1 != -1.0f)
                    {
                        lastLowIndex1 = CurrentPeakIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex1 = BackRefIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetInt(), lastHighTime1);
                    }
                }
                else
                {
                    if (ZigZagPeakType1[Index] == 1.0f && lastPeakType1 == 0.0f)
                    {
                        lastHighIndex1 = CurrentPeakIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetInt(), sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType1[Index] == -1.0f && lastPeakType1 == 0.0f)
                    {
                        lastLowIndex1 = CurrentPeakIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, Input_NumberofProfiles.GetInt(), sc.BaseDateTimeIn[0]);
                    }
                }
            }
            break;
        }
        case 2: // Three profiles
        {
            // Calculate ZigZag for VBP1 and VBP2
            if (Input_ReversalType1.GetInt() == 0) // Percent
                sc.ZigZag(High, Low, Subgraph_ZigZag1, Input_ReversalAmount1.GetFloat() * 0.01f);
            else // Amount
                sc.ZigZag(High, Low, Subgraph_ZigZag1, 0.0f, Input_ReversalAmount1.GetFloat());

            // Calculate ZigZag for VBP3
            sc.ZigZag(High, Low, Subgraph_ZigZag3, 0.0f, Input_ReversalAmount3.GetFloat());

            SCFloatArray ZigZagPeakType1 = Subgraph_ZigZag1.Arrays[0];
            SCFloatArray ZigZagPeakIndex1 = Subgraph_ZigZag1.Arrays[1];
            SCFloatArray ZigZagPeakType3 = Subgraph_ZigZag3.Arrays[0];
            SCFloatArray ZigZagPeakIndex3 = Subgraph_ZigZag3.Arrays[1];

            // Update VBP1 and VBP2 based on ZigZag1
            if (ZigZagPeakType1[Index] == 1.0f || ZigZagPeakType1[Index] == -1.0f)
            {
                int CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex1[Index]);
                int BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex1[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType1[Index] == 1.0f && ZigZagPeakType1[BackRefIndex] == -1.0f && lastPeakType1 != 1.0f)
                    {
                        lastHighIndex1 = CurrentPeakIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex1 = BackRefIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, 1, lastLowTime1); // Always update both VBP1 and VBP2
                    }
                    else if (ZigZagPeakType1[Index] == -1.0f && ZigZagPeakType1[BackRefIndex] == 1.0f && lastPeakType1 != -1.0f)
                    {
                        lastLowIndex1 = CurrentPeakIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex1 = BackRefIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, 1, lastHighTime1);
                    }
                }
                else
                {
                    if (ZigZagPeakType1[Index] == 1.0f && lastPeakType1 == 0.0f)
                    {
                        lastHighIndex1 = CurrentPeakIndex;
                        lastHighTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = 1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, 1, sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType1[Index] == -1.0f && lastPeakType1 == 0.0f)
                    {
                        lastLowIndex1 = CurrentPeakIndex;
                        lastLowTime1 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex1 = CurrentPeakIndex;
                        lastPeakType1 = -1.0f;

                        UpdateVBPStudies(sc, vbpID1, vbpID2, 1, sc.BaseDateTimeIn[0]);
                    }
                }
            }

            // Update VBP3 based on ZigZag3
            if (ZigZagPeakType3[Index] == 1.0f || ZigZagPeakType3[Index] == -1.0f)
            {
                int CurrentPeakIndex = static_cast<int>(ZigZagPeakIndex3[Index]);
                int BackRefIndex = (Index > 0) ? static_cast<int>(ZigZagPeakIndex3[Index - 1]) : -1;

                if (BackRefIndex >= 0)
                {
                    if (ZigZagPeakType3[Index] == 1.0f && ZigZagPeakType3[BackRefIndex] == -1.0f && lastPeakType3 != 1.0f)
                    {
                        lastHighIndex3 = CurrentPeakIndex;
                        lastHighTime3 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastLowIndex3 = BackRefIndex;
                        lastLowTime3 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex3 = CurrentPeakIndex;
                        lastPeakType3 = 1.0f;

                        UpdateVBP3(sc, vbpID3, lastLowTime3);
                    }
                    else if (ZigZagPeakType3[Index] == -1.0f && ZigZagPeakType3[BackRefIndex] == 1.0f && lastPeakType3 != -1.0f)
                    {
                        lastLowIndex3 = CurrentPeakIndex;
                        lastLowTime3 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastHighIndex3 = BackRefIndex;
                        lastHighTime3 = sc.BaseDateTimeIn[BackRefIndex];
                        lastPeakIndex3 = CurrentPeakIndex;
                        lastPeakType3 = -1.0f;

                        UpdateVBP3(sc, vbpID3, lastHighTime3);
                    }
                }
                else
                {
                    if (ZigZagPeakType3[Index] == 1.0f && lastPeakType3 == 0.0f)
                    {
                        lastHighIndex3 = CurrentPeakIndex;
                        lastHighTime3 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex3 = CurrentPeakIndex;
                        lastPeakType3 = 1.0f;

                        UpdateVBP3(sc, vbpID3, sc.BaseDateTimeIn[0]);
                    }
                    else if (ZigZagPeakType3[Index] == -1.0f && lastPeakType3 == 0.0f)
                    {
                        lastLowIndex3 = CurrentPeakIndex;
                        lastLowTime3 = sc.BaseDateTimeIn[CurrentPeakIndex];
                        lastPeakIndex3 = CurrentPeakIndex;
                        lastPeakType3 = -1.0f;

                        UpdateVBP3(sc, vbpID3, sc.BaseDateTimeIn[0]);
                    }
                }
            }
            break;
        }
    }
}















