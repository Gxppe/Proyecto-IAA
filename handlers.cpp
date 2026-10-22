#include "handlers.h"
#include "constraints.h"
#include <algorithm>
#include <iostream>

static bool contains(const vector<int> &v, int x) {
  return find(v.begin(), v.end(), x) != v.end();
}

static string label(const Instance &I, int id) {
  if (contains(I.O, id)) return "D" + to_string(id);
  if (contains(I.M, id)) return "A" + to_string(id);
  return to_string(id);
}

Solution empty_solution(const Instance &I) {
  int max_depot_id = -1;
  for (int depot : I.O) max_depot_id = max(max_depot_id, depot);

  Solution sol(max_depot_id + 1);
  for (int depot : I.O) sol[depot] = {depot};
  return sol;
}

void print_solution(const Instance &I, const Solution &sol) {
  for (const vector<int> &route : sol) {
    if (route.empty()) continue;

    for (size_t i = 0; i < route.size(); i++) {
      if (i > 0) cout << " -> ";
      cout << label(I, route[i]);
    }
    cout << endl;
  }
}

bool is_feasible(const Instance &I, const Solution &sol) {
  if (!bridge_left_once_2(I, sol)) return false;
  if (!bridge_entered_once_3(I, sol)) return false;
  if (!no_bridge_to_foreign_depot_4(I, sol)) return false;
  if (!no_bridge_from_foreign_depot_5(I, sol)) return false;
  if (!accommodation_flow_balance_6(I, sol)) return false;
  if (!bridge_flow_balance_7(I, sol)) return false;
  if (!leave_accommodation_next_day_8(I, sol)) return false;
  if (!accommodation_visit_from_bridge_9(I, sol)) return false;
  if (!new_day_requires_accommodation_10(I, sol)) return false;
  if (!valid_accommodation_entry_11(I, sol)) return false;
  if (!valid_accommodation_exit_12(I, sol)) return false;
  if (!return_to_depot_once_13(I, sol)) return false;
  if (!leave_depot_once_14(I, sol)) return false;
  if (!leave_depot_first_day_15(I, sol)) return false;
  if (!shift_time_limit_16(I, sol)) return false;
  if (!accumulated_time_coherence_17(I, sol)) return false;
  if (!arc_domain_18(I, sol)) return false;
  if (!accommodation_visit_domain_19(I, sol)) return false;
  if (!accumulated_time_non_negative_20(I, sol)) return false;
  return true;
}
