#include "constraints.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <tuple>

struct Arc {
  int from, to, day, team;
};

static bool contains(const vector<int> &v, int x) {
  return find(v.begin(), v.end(), x) != v.end();
}

static bool is_bridge(const Instance &I, int id) { return contains(I.N, id); }
static bool is_accommodation(const Instance &I, int id) { return contains(I.M, id); }
static bool is_depot(const Instance &I, int id) { return contains(I.O, id); }

static bool alpha(const Instance &I, int node, int team) {
  return !(is_depot(I, node) && node != team);
}

static int travel_time(const Instance &I, int i, int j) {
  return (int)lround(I.dist.at(i).at(j));
}

static int last_day(const Instance &I) { return (int)I.K.size(); }

static vector<Arc> build_arcs(const Instance &I, const Solution &sol) {
  vector<Arc> arcs;
  for (int team : I.O) {
    if ((size_t)team >= sol.size()) continue;
    const vector<int> &route = sol[team];
    int day = 1;
    for (size_t p = 0; p + 1 < route.size(); p++) {
      int from = route[p], to = route[p + 1];
      arcs.push_back({from, to, day, team});

      bool day_ends_at_to = !is_bridge(I, to);
      if (day_ends_at_to) day++;
    }
  }
  return arcs;
}

template <class Pred> static int count_arcs(const vector<Arc> &arcs, Pred pred) {
  return (int)count_if(arcs.begin(), arcs.end(), pred);
}

static int y(const vector<Arc> &arcs, int m, int day, int team) {
  return count_arcs(arcs, [&](const Arc &a) { return a.team == team && a.day == day && a.to == m; });
}

typedef map<tuple<int, int, int>, double> AccumulatedTimes;

static double time_at(const AccumulatedTimes &C, int team, int day, int node) {
  auto it = C.find(make_tuple(team, day, node));
  bool is_day_start = it == C.end();
  return is_day_start ? 0 : it->second;
}

static AccumulatedTimes accumulated_times(const Instance &I, const vector<Arc> &arcs) {
  AccumulatedTimes C;
  for (const Arc &a : arcs) {
    bool to_has_time = is_bridge(I, a.to) || is_accommodation(I, a.to);
    bool arc_has_distance = I.dist.at(a.from).at(a.to) >= 0;
    if (!to_has_time || !arc_has_distance) continue;

    auto first_arrival = make_tuple(a.team, a.day, a.to);
    if (C.count(first_arrival)) continue;

    int inspection = is_bridge(I, a.to) ? I.nodes[a.to].w : 0;
    C[first_arrival] = time_at(C, a.team, a.day, a.from) + travel_time(I, a.from, a.to) + inspection;
  }
  return C;
}

bool bridge_left_once_2(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int i : I.N)
    if (count_arcs(arcs, [&](const Arc &a) { return a.from == i; }) != 1) return false;
  return true;
}

bool bridge_entered_once_3(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int j : I.N)
    if (count_arcs(arcs, [&](const Arc &a) { return a.to == j; }) != 1) return false;
  return true;
}

bool no_bridge_to_foreign_depot_4(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  return count_arcs(arcs, [&](const Arc &a) {
           return is_bridge(I, a.from) && is_depot(I, a.to) && a.to != a.team;
         }) == 0;
}

bool no_bridge_from_foreign_depot_5(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  return count_arcs(arcs, [&](const Arc &a) {
           return is_depot(I, a.from) && a.from != a.team && is_bridge(I, a.to);
         }) == 0;
}

bool accommodation_flow_balance_6(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  auto bridge_or_depot = [&](int id, int team) {
    return (is_bridge(I, id) || is_depot(I, id)) && alpha(I, id, team);
  };
  for (int team : I.O)
    for (int m : I.M)
      for (int k = 1; k < last_day(I); k++) {
        int arrivals = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k && a.to == m && bridge_or_depot(a.from, team);
        });
        int departures = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k + 1 && a.from == m && bridge_or_depot(a.to, team);
        });
        if (arrivals != departures) return false;
      }
  return true;
}

bool bridge_flow_balance_7(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O)
    for (int n : I.N)
      for (int k = 1; k <= last_day(I); k++) {
        int in = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k && a.to == n && alpha(I, a.from, team);
        });
        int out = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k && a.from == n && alpha(I, a.to, team);
        });
        if (in != out) return false;
      }
  return true;
}

bool leave_accommodation_next_day_8(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O)
    for (int m : I.M)
      for (int k = 1; k < last_day(I); k++) {
        int to_bridge_next_day = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k + 1 && a.from == m && is_bridge(I, a.to);
        });
        if (to_bridge_next_day != y(arcs, m, k, team)) return false;
      }
  return true;
}

bool accommodation_visit_from_bridge_9(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O)
    for (int m : I.M)
      for (int k = 1; k <= last_day(I); k++) {
        int from_bridge = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k && a.to == m && is_bridge(I, a.from);
        });
        if (from_bridge != y(arcs, m, k, team)) return false;
      }
  return true;
}

bool new_day_requires_accommodation_10(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O)
    for (int k = 1; k < last_day(I); k++) {
      int lodgings = 0;
      for (int m : I.M) lodgings += y(arcs, m, k, team);

      vector<int> targets = I.N;
      targets.insert(targets.end(), I.M.begin(), I.M.end());
      for (int j : targets) {
        int from_bridge = count_arcs(arcs, [&](const Arc &a) {
          return a.team == team && a.day == k + 1 && a.to == j && is_bridge(I, a.from);
        });
        if (from_bridge > lodgings) return false;
      }
    }
  return true;
}

bool valid_accommodation_entry_11(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  return count_arcs(arcs, [&](const Arc &a) {
           return is_accommodation(I, a.to) && (is_accommodation(I, a.from) || is_depot(I, a.from));
         }) == 0;
}

bool valid_accommodation_exit_12(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  return count_arcs(arcs, [&](const Arc &a) {
           return is_accommodation(I, a.from) && (is_accommodation(I, a.to) || is_depot(I, a.to));
         }) == 0;
}

bool return_to_depot_once_13(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O) {
    int returns = count_arcs(arcs, [&](const Arc &a) {
      return a.team == team && a.to == team && is_bridge(I, a.from);
    });
    if (returns != 1) return false;
  }
  return true;
}

bool leave_depot_once_14(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O) {
    int departures = count_arcs(arcs, [&](const Arc &a) {
      return a.team == team && a.from == team && is_bridge(I, a.to);
    });
    if (departures != 1) return false;
  }
  return true;
}

bool leave_depot_first_day_15(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O) {
    int first_day_departures = count_arcs(arcs, [&](const Arc &a) {
      return a.team == team && a.day == 1 && a.from == team && is_bridge(I, a.to);
    });
    if (first_day_departures != 1) return false;
  }
  return true;
}

bool shift_time_limit_16(const Instance &I, const Solution &sol) {
  map<pair<int, int>, int> day_time;
  for (const Arc &a : build_arcs(I, sol)) {
    if (!is_bridge(I, a.to) || !alpha(I, a.from, a.team)) continue;
    day_time[{a.team, a.day}] += travel_time(I, a.from, a.to) + I.nodes[a.to].w;
  }
  for (const auto &entry : day_time)
    if (entry.second > I.Q) return false;
  return true;
}

bool accumulated_time_coherence_17(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  AccumulatedTimes C = accumulated_times(I, arcs);
  for (const Arc &a : arcs) {
    bool from_ok = is_bridge(I, a.from) || is_depot(I, a.from);
    bool to_ok = is_bridge(I, a.to) || is_accommodation(I, a.to);
    if (!from_ok || !to_ok || !alpha(I, a.from, a.team)) continue;

    double Ci = time_at(C, a.team, a.day, a.from);
    double Cj = time_at(C, a.team, a.day, a.to);
    if (Cj < Ci + travel_time(I, a.from, a.to)) return false;
  }
  return true;
}

bool arc_domain_18(const Instance &I, const Solution &sol) {
  set<tuple<int, int, int, int>> used;
  for (const Arc &a : build_arcs(I, sol)) {
    bool from_valid = is_bridge(I, a.from) || is_accommodation(I, a.from) || is_depot(I, a.from);
    bool to_valid = is_bridge(I, a.to) || is_accommodation(I, a.to) || is_depot(I, a.to);
    if (!from_valid || !to_valid || a.from == a.to) return false;
    if (is_accommodation(I, a.from) && is_accommodation(I, a.to)) return false;
    if (is_depot(I, a.from) && is_depot(I, a.to)) return false;
    if (!contains(I.K, a.day)) return false;
    if (!used.insert(make_tuple(a.team, a.day, a.from, a.to)).second) return false;
  }
  return true;
}

bool accommodation_visit_domain_19(const Instance &I, const Solution &sol) {
  vector<Arc> arcs = build_arcs(I, sol);
  for (int team : I.O)
    for (int m : I.M)
      for (int k = 1; k <= last_day(I); k++)
        if (y(arcs, m, k, team) > 1) return false;
  return true;
}

bool accumulated_time_non_negative_20(const Instance &I, const Solution &sol) {
  for (const auto &entry : accumulated_times(I, build_arcs(I, sol)))
    if (entry.second < 0) return false;
  return true;
}
