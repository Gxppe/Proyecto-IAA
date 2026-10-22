#pragma once
#include "handlers.h"
#include "instance.h"

bool bridge_left_once_2(const Instance &I, const Solution &sol);
bool bridge_entered_once_3(const Instance &I, const Solution &sol);
bool no_bridge_to_foreign_depot_4(const Instance &I, const Solution &sol);
bool no_bridge_from_foreign_depot_5(const Instance &I, const Solution &sol);
bool accommodation_flow_balance_6(const Instance &I, const Solution &sol);
bool bridge_flow_balance_7(const Instance &I, const Solution &sol);
bool leave_accommodation_next_day_8(const Instance &I, const Solution &sol);
bool accommodation_visit_from_bridge_9(const Instance &I, const Solution &sol);
bool new_day_requires_accommodation_10(const Instance &I, const Solution &sol);
bool valid_accommodation_entry_11(const Instance &I, const Solution &sol);
bool valid_accommodation_exit_12(const Instance &I, const Solution &sol);
bool return_to_depot_once_13(const Instance &I, const Solution &sol);
bool leave_depot_once_14(const Instance &I, const Solution &sol);
bool leave_depot_first_day_15(const Instance &I, const Solution &sol);
bool shift_time_limit_16(const Instance &I, const Solution &sol);
bool accumulated_time_coherence_17(const Instance &I, const Solution &sol);
bool arc_domain_18(const Instance &I, const Solution &sol);
bool accommodation_visit_domain_19(const Instance &I, const Solution &sol);
bool accumulated_time_non_negative_20(const Instance &I, const Solution &sol);
