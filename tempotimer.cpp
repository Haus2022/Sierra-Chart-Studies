
#include "sierrachart.h"

SCDLLName("Tempo Timer DLL")

SCSFExport scsf_TempoTimer(SCStudyInterfaceRef sc)
{
    SCString msg;
    // Inputs
    SCInputRef Input_Boundaries = sc.Input[0];
    SCInputRef EnableDebug = sc.Input[1];
    SCInputRef TextColor = sc.Input[2];
    SCInputRef TextSize = sc.Input[3];

    // Set Defaults
    if (sc.SetDefaults)
    {
        sc.GraphName = "Measure of Tempo 10";
        sc.StudyDescription = "Measures time for price to move up or down by user-defined range using system time";
        sc.AutoLoop = 0; // Live-time indicator

        Input_Boundaries.Name = "Range boundaries";
        Input_Boundaries.SetFloat(5.0f); //  default for testing

        EnableDebug.Name = "Enable Debug Messages";
        EnableDebug.SetYesNo(0); // Enable for debugging

        TextColor.Name = "Text Color";
        TextColor.SetColor(RGB(255, 255, 255)); // white for visibility

        TextSize.Name = "Text Size";
        TextSize.SetInt(10); // Increased for visibility

        // Initialize persistent variables
        sc.GetPersistentDouble(0) = 0.0; // priorDouble
        sc.GetPersistentFloat(0) = 0.0f; // plast
        sc.GetPersistentFloat(1) = 0.0f; // Top
        sc.GetPersistentFloat(2) = 0.0f; // Bottom
        sc.GetPersistentFloat(3) = 0.0f; // lastTimeDiffSeconds
        sc.GetPersistentInt(1) = 0; // UpperHits
        sc.GetPersistentInt(2) = 0; // LowerHits
        sc.GetPersistentInt(3) = 0; // TextLineNumber

        return;
    }

    // Get persistent variables
    double& priorDouble = sc.GetPersistentDouble(0);
    float& plast = sc.GetPersistentFloat(0);
    float& Top = sc.GetPersistentFloat(1);
    float& Bottom = sc.GetPersistentFloat(2);
    float& lastTimeDiffSeconds = sc.GetPersistentFloat(3);
    int& UpperHits = sc.GetPersistentInt(1);
    int& LowerHits = sc.GetPersistentInt(2);
    int& TextLineNumber = sc.GetPersistentInt(3);

    // Get offset
    float Offset = Input_Boundaries.GetFloat();

    // Get current price and time
    float LastPrice = sc.LastTradePrice;
    double currentDouble = sc.CurrentSystemDateTime.GetAsDouble();

    // Initialize on first run
    if (priorDouble == 0)
    {
        priorDouble = currentDouble;
        plast = sc.RoundToTickSize(LastPrice, Offset);

        if (Offset == 5.0f)
        {
            float nextPlast = plast + Offset;
            float prevPlast = plast - Offset;
            if (fmod(plast + Offset, 10.0f) != 0.0f)
            {
                if (fabs(nextPlast - LastPrice) < fabs(plast - LastPrice) && fmod(nextPlast + Offset, 10.0f) == 0.0f)
                    plast = nextPlast;
                else if (fabs(prevPlast - LastPrice) < fabs(plast - LastPrice) && fmod(prevPlast + Offset, 10.0f) == 0.0f)
                    plast = prevPlast;
            }
        }

        Top = plast + Offset;
        Bottom = plast - Offset;
        lastTimeDiffSeconds = 0.0f;

        if (EnableDebug.GetYesNo())
        {
            msg.Format("Initial setup: plast: %.2f, top: %.2f, bottom: %.2f, upper hits: %d, lower hits: %d", 
                       plast, Top, Bottom, UpperHits, LowerHits);
            sc.AddMessageToLog(msg, 0);
        }
    }

    // Clean up drawing when study is removed
    if (sc.LastCallToFunction)
    {
        if (TextLineNumber != 0)
        {
            sc.DeleteUserDrawnACSDrawing(sc.ChartNumber, TextLineNumber);
            if (EnableDebug.GetYesNo())
            {
                msg.Format("Deleted user-drawn text with LineNumber: %d", TextLineNumber);
                sc.AddMessageToLog(msg, 0);
            }
        }
        return;
    }

    // Combined text drawing
    s_UseTool combinedText;
    combinedText.Clear();
    combinedText.ChartNumber = sc.ChartNumber;
    combinedText.DrawingType = DRAWING_TEXT;
    combinedText.Region = 0;
    combinedText.Text.Format("Upper: %d | Tempo: %.2f s | Lower: %d", UpperHits, lastTimeDiffSeconds, LowerHits);
    combinedText.Color = TextColor.GetColor();
    combinedText.FontSize = TextSize.GetInt();
    combinedText.AddAsUserDrawnDrawing = 1; // Movable by user
    combinedText.AddMethod = UTAM_ADD_OR_ADJUST; // Update content without resetting position

    // Set position only for new drawings
    if (TextLineNumber == 0)
    {
        int BarIndex = max(0, sc.ArraySize + 10); // Fallback to 0 if chart is small
        combinedText.BeginIndex = BarIndex;
        combinedText.BeginValue = LastPrice; // Use current price
        if (EnableDebug.GetYesNo())
        {
            msg.Format("Creating new text drawing at BarIndex: %d, Value: %.2f, Text: %s, sc.ArraySize: %d", 
                       BarIndex, combinedText.BeginValue, combinedText.Text.GetChars(), sc.ArraySize);
            sc.AddMessageToLog(msg, 0);
        }
    }
    else
    {
        combinedText.LineNumber = TextLineNumber; // Use existing LineNumber
        if (EnableDebug.GetYesNo())
        {
            msg.Format("Updating existing text drawing with LineNumber: %d, Text: %s", 
                       TextLineNumber, combinedText.Text.GetChars());
            sc.AddMessageToLog(msg, 0);
        }
    }

    // Apply the drawing
    if (sc.UseTool(combinedText))
    {
        TextLineNumber = combinedText.LineNumber;
        if (EnableDebug.GetYesNo())
        {
            msg.Format("sc.UseTool successful, assigned LineNumber: %d", TextLineNumber);
            sc.AddMessageToLog(msg, 0);
        }
    }
    else if (EnableDebug.GetYesNo())
    {
        msg.Format("sc.UseTool failed for text drawing, ChartNumber: %d, Region: %d", 
                   combinedText.ChartNumber, combinedText.Region);
        sc.AddMessageToLog(msg, 1);
    }

    // Handle boundary hits
    auto handleBoundaryHit = [&](bool isUpperHit, float boundaryPrice, int& hitCounter) {
        if ((isUpperHit && LastPrice >= boundaryPrice) || 
            (!isUpperHit && LastPrice <= boundaryPrice))
        {
            double timeDiffDays = currentDouble - priorDouble;
            lastTimeDiffSeconds = static_cast<float>(timeDiffDays * 86400.0);
            hitCounter++;

            if (EnableDebug.GetYesNo())
            {
                msg.Format("%s hit detected: Time difference: %.2f seconds, %s hits: %d, LastPrice: %.2f, Boundary: %.2f", 
                           isUpperHit ? "Upper" : "Lower", lastTimeDiffSeconds, isUpperHit ? "upper" : "lower", hitCounter, LastPrice, boundaryPrice);
                sc.AddMessageToLog(msg, 0);
            }

            priorDouble = currentDouble;
            plast = sc.RoundToTickSize(boundaryPrice, Offset);
            if (Offset == 5.0f && fmod(plast + Offset, 10.0f) != 0.0f)
            {
                float nextPlast = plast + Offset;
                float prevPlast = plast - Offset;
                if (fmod(nextPlast + Offset, 10.0f) == 0.0f)
                    plast = nextPlast;
                else if (fmod(prevPlast + Offset, 10.0f) == 0.0f)
                    plast = prevPlast;
            }

            Top = plast + Offset;
            Bottom = plast - Offset;

            if (EnableDebug.GetYesNo())
            {
                msg.Format("After %s hit: plast: %.2f, top: %.2f, bottom: %.2f, upper hits: %d, lower hits: %d", 
                           isUpperHit ? "upper" : "lower", plast, Top, Bottom, UpperHits, LowerHits);
                sc.AddMessageToLog(msg, 0);
            }
        }
    };

    // Debug price and boundaries
    if (EnableDebug.GetYesNo())
    {
        msg.Format("Checking: LastPrice: %.2f, Top: %.2f, Bottom: %.2f, SystemTime: %.6f", 
                   LastPrice, Top, Bottom, currentDouble);
        sc.AddMessageToLog(msg, 0);
    }

    // Check boundary hits
    handleBoundaryHit(true, Top, UpperHits);  // Upper boundary
    handleBoundaryHit(false, Bottom, LowerHits); // Lower boundary
}

