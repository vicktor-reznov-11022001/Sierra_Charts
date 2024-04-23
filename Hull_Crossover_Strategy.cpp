#include "sierrachart.h"

SCDLLName("Hull Crossover Strategy")

/*
This code implements a trading strategy based on Hull Moving Average (HMA) crossovers. Here's a breakdown of its components and functionality:

Initialization Section:
Default settings for the strategy are defined, such as HMA periods, stop and target values, and graphical settings for visualization.
Trading behavior parameters are set, including position management and order handling settings.
Main Logic:
Hull moving averages (HMAs) are calculated based on the specified input data and periods using sc.HullMovingAverage.
A new order object (s_SCNewOrder) is created with parameters like quantity, order type, time in force, and offsets for target and stop orders.
The current position data is obtained using sc.GetTradePosition.
The strategy checks if the current bar has closed and if there's a crossover between the faster and slower HMAs.
If a crossover from bottom to top occurs (CROSS_FROM_BOTTOM), indicating a bullish signal:
If there's a short position (PositionData.PositionQuantity < 0), all orders are canceled, and the position is flattened to close it.
A buy entry order is generated using sc.BuyEntry with the parameters from NewOrder.
If a crossover from top to bottom occurs (CROSS_FROM_TOP), indicating a bearish signal:
If there's a long position (PositionData.PositionQuantity > 0), all orders are canceled, and the position is flattened.
A sell entry order is generated using sc.SellEntry with the parameters from NewOrder.
Additional Notes:
Only one trade per bar is allowed (sc.AllowOnlyOneTradePerBar = true).
Trade statistics and data are maintained (sc.MaintainTradeStatisticsAndTradesData = true).
This code prepares order parameters and conditions but doesn't execute actual trades. Integration with a brokerage or trading platform would be necessary for live trading.
Overall, this code implements a trading strategy based on HMA crossovers, allowing for both long and short trades with specified stop and target levels.
*/

SCSFExport scsf_Hull_Crossover_Trading(SCStudyInterfaceRef sc)
{
    SCSubgraphRef Hull_Fast = sc.Subgraph[0];
    SCSubgraphRef Hull_Slow = sc.Subgraph[1];
	
	SCInputRef Hull_Fast_Period = sc.Input[0];
	SCInputRef Hull_Slow_Period = sc.Input[1];
	SCInputRef Hull_Fast_Data = sc.Input[2];
	SCInputRef Hull_Slow_Data = sc.Input[3];
	
	SCInputRef Target_Ticks = sc.Input[4];
	SCInputRef Stop_Ticks = sc.Input[5];

    // Section 1 - Set the configuration variables and defaults
    if (sc.SetDefaults)
    {
        sc.GraphName = "Hull Crossover Strategy";

        sc.AutoLoop = 1;
        sc.GraphRegion = 0;

        Hull_Fast_Period.Name = "Faster Hull Period";
        Hull_Fast_Period.SetInt(9);

        Hull_Slow_Period.Name = "Slower Hull Period";
        Hull_Slow_Period.SetInt(9);
		
		Stop_Ticks.Name = "Stop Value in terms of Ticks";
        Stop_Ticks.SetInt(80);
		
		Target_Ticks.Name = "Target Value in terms of Ticks";
        Target_Ticks.SetInt(80);

        Hull_Fast.Name = "Faster Hull";
        Hull_Fast.DrawStyle = DRAWSTYLE_LINE;
        Hull_Fast.PrimaryColor = RGB(128, 255, 128);

        Hull_Slow.Name = "Slower Hull";
        Hull_Slow.DrawStyle = DRAWSTYLE_LINE;
        Hull_Slow.PrimaryColor = RGB(255, 0, 0);

        Hull_Fast_Data.Name = "Faster Hull Input Data";
        Hull_Fast_Data.SetInputDataIndex(SC_LAST);

        Hull_Slow_Data.Name = "Slower Hull Input Data";
        Hull_Slow_Data.SetInputDataIndex(SC_LAST);

        // Any of the following variables can also be set outside and below the sc.SetDefaults code block

        sc.AllowMultipleEntriesInSameDirection = false;
        sc.MaximumPositionAllowed = 1;
        sc.SupportReversals = false;

        // This is false by default. Orders will go to the simulation system always.
        sc.SendOrdersToTradeService = false;

        sc.AllowOppositeEntryWithOpposingPositionOrOrders = false;
        sc.SupportAttachedOrdersForTrading = false;

        sc.CancelAllOrdersOnEntriesAndReversals = true;
        sc.AllowEntryWithWorkingOrders = false;
        sc.CancelAllWorkingOrdersOnExit = true;

        // Only 1 trade for each Order Action type is allowed per bar.
        sc.AllowOnlyOneTradePerBar = true;

        // This needs to be set to true when a trading study uses trading functions.
        sc.MaintainTradeStatisticsAndTradesData = true;

        return;
    }

    sc.HullMovingAverage(sc.BaseDataIn[Hull_Fast_Data.GetInputDataIndex()], Hull_Fast, Hull_Fast_Data.GetInt());
    sc.HullMovingAverage(sc.BaseDataIn[Hull_Slow_Data.GetInputDataIndex()], Hull_Slow, Hull_Slow_Period.GetInt());

    // Create an s_SCNewOrder object.
    s_SCNewOrder NewOrder;
    NewOrder.OrderQuantity = 1;
    NewOrder.OrderType = SCT_ORDERTYPE_MARKET;
    NewOrder.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
    NewOrder.AttachedOrderTarget1Type = SCT_ORDERTYPE_LIMIT;
    NewOrder.AttachedOrderStop1Type = SCT_ORDERTYPE_TRAILING_STOP;
    NewOrder.Target1Offset = Target_Ticks.GetInt() * sc.TickSize;
    NewOrder.Stop1Offset = Stop_Ticks.GetInt() * sc.TickSize;

    // Check for already Open Position
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);

    int Result = 0;

    if ((sc.GetBarHasClosedStatus() == BHCS_BAR_HAS_CLOSED))
    {
        if (sc.CrossOver(Hull_Fast, Hull_Slow) == CROSS_FROM_BOTTOM)
        {
            if(PositionData.PositionQuantity < 0)
            {
                sc.CancelAllOrders();
                sc.FlattenPosition();
            }

            Result = static_cast<int>(sc.BuyEntry(NewOrder));
        }

        if (sc.CrossOver(Hull_Fast, Hull_Slow) == CROSS_FROM_TOP)
        {
            if(PositionData.PositionQuantity > 0)
            {
                sc.CancelAllOrders();
                sc.FlattenPosition();
            }
            
            Result = static_cast<int>(sc.SellEntry(NewOrder));
        }
    }
}
//================================================================================================//
