#include "sierrachart.h"
#include <vector>
#include <algorithm>
#include <cmath>

// Undefine Sierra Chart macros to avoid conflicts
#undef max
#undef min

SCDLLName("GetPercentileLevelsStudy")

// Helper function to calculate percentiles
void CalculatePercentiles(SCStudyInterfaceRef sc, SCFloatArrayRef data, int lookback, 
                         const std::vector<float>& percentile_levels, 
                         std::vector<float>& results, bool enableDebug)
{
    std::vector<float> values;
    values.reserve(lookback);
    
    // Collect non-zero values within lookback period
    float min_val = std::numeric_limits<float>::max();
    float max_val = std::numeric_limits<float>::lowest();
    for (int i = std::max(0, sc.ArraySize - lookback); i < sc.ArraySize; i++) {
        if (data[i] != 0 || std::isfinite(data[i])) { // Include zeros if valid for oscillator
            values.push_back(data[i]);
            min_val = std::min(min_val, data[i]);
            max_val = std::max(max_val, data[i]);
        }
    }

    results.resize(percentile_levels.size(), 0.0f);
    
    if (values.size() >= 10) { // Minimum data threshold
        std::sort(values.begin(), values.end());
        
        // Log data range for debugging
        if (enableDebug) {
            SCString range_msg;
            range_msg.Format("Data points: %zu, Min: %.2f, Max: %.2f", values.size(), min_val, max_val);
            sc.AddMessageToLog(range_msg, 0);
        }

        size_t n = values.size();
        for (size_t i = 0; i < percentile_levels.size(); i++) {
            float p = percentile_levels[i] / 100.0f;
            // Use p * n for standard percentile calculation
            size_t rank = static_cast<size_t>(std::round(p * n));
            rank = std::max<size_t>(0, std::min(n - 1, rank));
            results[i] = values[rank];

            // Log each percentile for debugging
            if (enableDebug) {
                SCString msg;
                msg.Format("Percentile %.2f%%: Rank %zu, Value %.2f", percentile_levels[i], rank, results[i]);
                sc.AddMessageToLog(msg, 0);
            }
        }
    } else {
        if (enableDebug) {
            SCString msg;
            msg.Format("Not enough valid data for percentiles (%zu points, need at least 10)", values.size());
            sc.AddMessageToLog(msg, 1);
        }
    }
}

SCSFExport scsf_GeneralizedPercentileStudy(SCStudyInterfaceRef sc)
{
    int sg = 0; 
    SCSubgraphRef Subgraph_Percentile1 = sc.Subgraph[sg++]; // 99th percentile
    SCSubgraphRef Subgraph_Percentile2 = sc.Subgraph[sg++]; // 75th percentile
    SCSubgraphRef Subgraph_Percentile3 = sc.Subgraph[sg++]; // 50th percentile
    SCSubgraphRef Subgraph_Percentile4 = sc.Subgraph[sg++]; // 25th percentile
    SCSubgraphRef Subgraph_Percentile5 = sc.Subgraph[sg++]; // 1st percentile

    int ip = 0; 
    SCInputRef Input_Data = sc.Input[ip++];    // Base data or study subgraph
    SCInputRef Input_LookbackPeriod = sc.Input[ip++];   // Lookback period
    SCInputRef Input_PercentileLevel1 = sc.Input[ip++]; // Percentile 1 (e.g., 99%)
    SCInputRef Input_PercentileLevel2 = sc.Input[ip++]; // Percentile 2 (e.g., 75%)
    SCInputRef Input_PercentileLevel3 = sc.Input[ip++]; // Percentile 3 (e.g., 50%)
    SCInputRef Input_PercentileLevel4 = sc.Input[ip++]; // Percentile 4 (e.g., 25%)
    SCInputRef Input_PercentileLevel5 = sc.Input[ip++]; // Percentile 5 (e.g., 1%)
    SCInputRef Input_EnableDebug = sc.Input[ip++];      // Enable debug logging

    // Declare persistent variables as references
    float& Percentile1 = sc.GetPersistentFloat(1); // 99th percentile value
    float& Percentile2 = sc.GetPersistentFloat(2); // 75th percentile value
    float& Percentile3 = sc.GetPersistentFloat(3); // 50th percentile value
    float& Percentile4 = sc.GetPersistentFloat(4); // 25th percentile value
    float& Percentile5 = sc.GetPersistentFloat(5); // 1st percentile value
    int& LastLookback = sc.GetPersistentInt(1);    // Last lookback period
    int& LastStudyID = sc.GetPersistentInt(2);     // Last study ID
    int& LastSubgraph = sc.GetPersistentInt(3);    // Last subgraph index
    float& LastP1 = sc.GetPersistentFloat(6);      // Last Percentile Level 1 (99%)
    float& LastP2 = sc.GetPersistentFloat(7);      // Last Percentile Level 2 (75%)
    float& LastP3 = sc.GetPersistentFloat(8);      // Last Percentile Level 3 (50%)
    float& LastP4 = sc.GetPersistentFloat(9);      // Last Percentile Level 4 (25%)
    float& LastP5 = sc.GetPersistentFloat(10);     // Last Percentile Level 5 (1%)

    // Set defaults
    if (sc.SetDefaults)
    {
        sc.GraphName = "Get Percentile Levels Study";
        sc.AutoLoop = 1; // Use AutoLoop for simplicity
        sc.ValueFormat = VALUEFORMAT_INHERITED; // Inherit format from data source
        sc.GraphRegion = 0; // Main chart region
        sc.MaintainAdditionalChartDataArrays = 1; // Required for study data access

        // Subgraph settings
        Subgraph_Percentile1.Name = "Percentile 99%";
        Subgraph_Percentile1.DrawStyle = DRAWSTYLE_LINE;
        Subgraph_Percentile1.PrimaryColor = RGB(0, 255, 255); // Cyan
        Subgraph_Percentile1.LineWidth = 2;
        Subgraph_Percentile1.DrawZeros = false;

        Subgraph_Percentile2.Name = "Percentile 75%";
        Subgraph_Percentile2.DrawStyle = DRAWSTYLE_LINE;
        Subgraph_Percentile2.PrimaryColor = RGB(0, 200, 200); // Light Cyan
        Subgraph_Percentile2.LineWidth = 2;
        Subgraph_Percentile2.DrawZeros = false;

        Subgraph_Percentile3.Name = "Percentile 50%";
        Subgraph_Percentile3.DrawStyle = DRAWSTYLE_LINE;
        Subgraph_Percentile3.PrimaryColor = RGB(255, 255, 0); // Yellow
        Subgraph_Percentile3.LineWidth = 2;
        Subgraph_Percentile3.DrawZeros = false;

        Subgraph_Percentile4.Name = "Percentile 25%";
        Subgraph_Percentile4.DrawStyle = DRAWSTYLE_LINE;
        Subgraph_Percentile4.PrimaryColor = RGB(200, 200, 0); // Light Yellow
        Subgraph_Percentile4.LineWidth = 2;
        Subgraph_Percentile4.DrawZeros = false;

        Subgraph_Percentile5.Name = "Percentile 1%";
        Subgraph_Percentile5.DrawStyle = DRAWSTYLE_LINE;
        Subgraph_Percentile5.PrimaryColor = RGB(255, 0, 0); // Red
        Subgraph_Percentile5.LineWidth = 2;
        Subgraph_Percentile5.DrawZeros = false;

        // Input settings
        Input_Data.Name = "Input Data";
        Input_Data.SetStudySubgraphValues(0, 0); // Default to main chart, subgraph 0

        Input_LookbackPeriod.Name = "Lookback Period (Bars)";
        Input_LookbackPeriod.SetInt(100);
        Input_LookbackPeriod.SetIntLimits(1, 10000);

        Input_PercentileLevel1.Name = "Percentile Level 1 (%)";
        Input_PercentileLevel1.SetFloat(99.0f);
        Input_PercentileLevel1.SetFloatLimits(0.0f, 100.0f);

        Input_PercentileLevel2.Name = "Percentile Level 2 (%)";
        Input_PercentileLevel2.SetFloat(75.0f);
        Input_PercentileLevel2.SetFloatLimits(0.0f, 100.0f);

        Input_PercentileLevel3.Name = "Percentile Level 3 (%)";
        Input_PercentileLevel3.SetFloat(50.0f);
        Input_PercentileLevel3.SetFloatLimits(0.0f, 100.0f);

        Input_PercentileLevel4.Name = "Percentile Level 4 (%)";
        Input_PercentileLevel4.SetFloat(25.0f);
        Input_PercentileLevel4.SetFloatLimits(0.0f, 100.0f);

        Input_PercentileLevel5.Name = "Percentile Level 5 (%)";
        Input_PercentileLevel5.SetFloat(1.0f);
        Input_PercentileLevel5.SetFloatLimits(0.0f, 100.0f);

        Input_EnableDebug.Name = "Enable Debug Logging";
        Input_EnableDebug.SetYesNo(false); // Default to off

        // Initialize persistent variables to 0
        Percentile1 = 0.0f;
        Percentile2 = 0.0f;
        Percentile3 = 0.0f;
        Percentile4 = 0.0f;
        Percentile5 = 0.0f;
        LastLookback = 0;
        LastStudyID = 0;
        LastSubgraph = 0;
        LastP1 = 0.0f;
        LastP2 = 0.0f;
        LastP3 = 0.0f;
        LastP4 = 0.0f;
        LastP5 = 0.0f;

        return;
    }

    // Get input data
    SCFloatArray StudyArray;
    sc.GetStudyArrayUsingID(Input_Data.GetStudyID(), Input_Data.GetSubgraphIndex(), StudyArray);
    if (StudyArray.GetArraySize() == 0) {
        if (Input_EnableDebug.GetYesNo()) {
            sc.AddMessageToLog("No data available for selected input", 1);
        }
        return;
    }

    // Check if recalculation is needed
    bool recalculate = false;
    int current_lookback = Input_LookbackPeriod.GetInt();
    int current_study_id = Input_Data.GetStudyID();
    int current_subgraph = Input_Data.GetSubgraphIndex();
    float current_p1 = Input_PercentileLevel1.GetFloat();
    float current_p2 = Input_PercentileLevel2.GetFloat();
    float current_p3 = Input_PercentileLevel3.GetFloat();
    float current_p4 = Input_PercentileLevel4.GetFloat();
    float current_p5 = Input_PercentileLevel5.GetFloat();

    if (sc.IsFullRecalculation || 
        current_lookback != LastLookback ||
        current_study_id != LastStudyID ||
        current_subgraph != LastSubgraph ||
        current_p1 != LastP1 ||
        current_p2 != LastP2 ||
        current_p3 != LastP3 ||
        current_p4 != LastP4 ||
        current_p5 != LastP5) {
        recalculate = true;
        LastLookback = current_lookback;
        LastStudyID = current_study_id;
        LastSubgraph = current_subgraph;
        LastP1 = current_p1;
        LastP2 = current_p2;
        LastP3 = current_p3;
        LastP4 = current_p4;
        LastP5 = current_p5;
    }

    // Validate inputs
    std::vector<float> percentile_levels = {
        current_p1, current_p2, current_p3, current_p4, current_p5
    };
    for (float p : percentile_levels) {
        if (p < 0.0f || p > 100.0f) {
            if (Input_EnableDebug.GetYesNo()) {
                sc.AddMessageToLog("Invalid percentile level detected (must be 0-100)", 1);
            }
            return;
        }
    }
    if (current_lookback > sc.ArraySize) {
        if (Input_EnableDebug.GetYesNo()) {
            sc.AddMessageToLog("Lookback period exceeds available bars", 1);
        }
        return;
    }

    // Calculate percentiles if needed
    if (recalculate && sc.Index == 0) {
        std::vector<float> results;
        CalculatePercentiles(sc, StudyArray, current_lookback, percentile_levels, results, Input_EnableDebug.GetYesNo());
        
        if (results.size() == 5) {
            Percentile1 = results[0]; // 99th
            Percentile2 = results[1]; // 75th
            Percentile3 = results[2]; // 50th
            Percentile4 = results[3]; // 25th
            Percentile5 = results[4]; // 1st
        }
    }

    // Log persistent values for debugging
    if (Input_EnableDebug.GetYesNo()) {
        SCString persist_msg;
        persist_msg.Format("Persistent Values: 99%%=%.2f, 75%%=%.2f, 50%%=%.2f, 25%%=%.2f, 1%%=%.2f",
                           Percentile1, Percentile2, Percentile3, Percentile4, Percentile5);
        sc.AddMessageToLog(persist_msg, 0);
    }

    // Plot percentiles
    Subgraph_Percentile1[sc.Index] = Percentile1;
    Subgraph_Percentile2[sc.Index] = Percentile2;
    Subgraph_Percentile3[sc.Index] = Percentile3;
    Subgraph_Percentile4[sc.Index] = Percentile4;
    Subgraph_Percentile5[sc.Index] = Percentile5;
}