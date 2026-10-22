#include "optimize_utils.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>

static const double COST_PER_DISTANCE = 800;

static bool contains(const vector<int> &v, int x) {
  return find(v.begin(), v.end(), x) != v.end();
}

static bool is_bridge(const Instance &I, int id) { return contains(I.N, id); }

double travel_cost(const Instance &I, int from, int to) {
  double distance_one_decimal = round(I.dist.at(from).at(to) * 10) / 10;
  return COST_PER_DISTANCE * distance_one_decimal;
}

double route_cost(const Instance &I, const vector<int> &route, int team) {
  double cost = 0;
  for (size_t p = 0; p + 1 < route.size(); p++) {
    int from = route[p], to = route[p + 1];
    cost += travel_cost(I, from, to);

    if (contains(I.M, to)) cost += I.f.at(to);

    bool dispatches_team = from == team && (is_bridge(I, to) || contains(I.M, to));
    if (dispatches_team) cost += I.v;
  }
  return cost;
}

double objective(const Instance &I, const Solution &sol) {
  double cost = 0;
  for (int team : I.O)
    if ((size_t)team < sol.size()) cost += route_cost(I, sol[team], team);
  return cost;
}

struct RouteDays {
  int depot;
  vector<vector<int>> bridges;
  vector<int> lodgings;
};

static RouteDays split_days(const Instance &I, const vector<int> &route) {
  if (route.size() < 2) throw invalid_argument("La ruta debe partir y terminar en su deposito");

  RouteDays days{route.front(), {{}}, {}};
  for (size_t p = 1; p + 1 < route.size(); p++) {
    int node = route[p];
    if (is_bridge(I, node)) {
      days.bridges.back().push_back(node);
    } else {
      days.lodgings.push_back(node);
      days.bridges.push_back({});
    }
  }
  return days;
}

static vector<int> join_days(const RouteDays &days) {
  vector<int> route = {days.depot};
  for (size_t d = 0; d < days.bridges.size(); d++) {
    route.insert(route.end(), days.bridges[d].begin(), days.bridges[d].end());
    if (d < days.lodgings.size()) route.push_back(days.lodgings[d]);
  }
  route.push_back(days.depot);
  return route;
}

static vector<int> &bridges_of_day(RouteDays &days, int day) {
  return days.bridges.at(day - 1);
}

static void remove_empty_days(RouteDays &days) {
  for (int d = (int)days.bridges.size() - 1; d >= 0 && days.bridges.size() > 1; d--) {
    if (!days.bridges[d].empty()) continue;

    bool is_last_day = d == (int)days.bridges.size() - 1;
    int lodging_to_remove = is_last_day ? d - 1 : d;
    days.bridges.erase(days.bridges.begin() + d);
    days.lodgings.erase(days.lodgings.begin() + lodging_to_remove);
  }
}

typedef map<int, RouteDays> EditedRoutes;

static RouteDays &days_of(EditedRoutes &edited, const Instance &I, const Solution &sol, int team) {
  auto it = edited.find(team);
  if (it == edited.end()) it = edited.emplace(team, split_days(I, sol.at(team))).first;
  return it->second;
}

static Solution with_edited_routes(const Solution &sol, const EditedRoutes &edited) {
  Solution result = sol;
  for (const auto &entry : edited) result[entry.first] = join_days(entry.second);
  return result;
}

static int take_bridge(RouteDays &days, int day, int position) {
  vector<int> &bridges = bridges_of_day(days, day);
  int bridge = bridges.at(position);
  bridges.erase(bridges.begin() + position);
  return bridge;
}

Solution two_opt(const Instance &I, const Solution &sol, int team, int day, int first, int last) {
  EditedRoutes edited;
  vector<int> &bridges = bridges_of_day(days_of(edited, I, sol, team), day);
  if (first < 0 || last >= (int)bridges.size() || first >= last)
    throw out_of_range("two_opt: posiciones invalidas");

  reverse(bridges.begin() + first, bridges.begin() + last + 1);
  return with_edited_routes(sol, edited);
}

Solution swap_blocks(const Instance &I, const Solution &sol,
                     int team_a, int day_a, int start_a,
                     int team_b, int day_b, int start_b, int length) {
  if (team_a == team_b && day_a == day_b) throw invalid_argument("swap_blocks: mismo equipo y dia");
  if (length < 1) throw invalid_argument("swap_blocks: largo invalido");

  EditedRoutes edited;
  vector<int> &bridges_a = bridges_of_day(days_of(edited, I, sol, team_a), day_a);
  vector<int> &bridges_b = bridges_of_day(days_of(edited, I, sol, team_b), day_b);
  bool block_a_fits = start_a >= 0 && start_a + length <= (int)bridges_a.size();
  bool block_b_fits = start_b >= 0 && start_b + length <= (int)bridges_b.size();
  if (!block_a_fits || !block_b_fits) throw out_of_range("swap_blocks: bloque fuera del dia");

  swap_ranges(bridges_a.begin() + start_a, bridges_a.begin() + start_a + length, bridges_b.begin() + start_b);
  return with_edited_routes(sol, edited);
}

Solution change_day(const Instance &I, const Solution &sol,
                    int from_team, int from_day, int position,
                    int to_team, int to_day, int insert_at) {
  EditedRoutes edited;
  RouteDays &origin = days_of(edited, I, sol, from_team);
  RouteDays &destination = days_of(edited, I, sol, to_team);

  int bridge = take_bridge(origin, from_day, position);

  vector<int> &to_bridges = bridges_of_day(destination, to_day);
  if (insert_at < 0 || insert_at > (int)to_bridges.size()) throw out_of_range("change_day: posicion de insercion invalida");
  to_bridges.insert(to_bridges.begin() + insert_at, bridge);

  remove_empty_days(origin);
  return with_edited_routes(sol, edited);
}

Solution change_new_day(const Instance &I, const Solution &sol,
                        int from_team, int from_day, int position,
                        int to_team, int accommodation) {
  if (!contains(I.M, accommodation)) throw invalid_argument("change_new_day: no es un alojamiento");

  EditedRoutes edited;
  RouteDays &origin = days_of(edited, I, sol, from_team);
  int bridge = take_bridge(origin, from_day, position);
  remove_empty_days(origin);

  RouteDays &destination = days_of(edited, I, sol, to_team);
  destination.lodgings.push_back(accommodation);
  destination.bridges.push_back({bridge});
  return with_edited_routes(sol, edited);
}

Solution change_accommodation(const Instance &I, const Solution &sol, int team, int night, int accommodation) {
  if (!contains(I.M, accommodation)) throw invalid_argument("change_accommodation: no es un alojamiento");

  EditedRoutes edited;
  days_of(edited, I, sol, team).lodgings.at(night - 1) = accommodation;
  return with_edited_routes(sol, edited);
}

static const int MAX_SWAP_LENGTH = 5;
static const double MIN_IMPROVEMENT = 1e-6;

typedef function<bool(const Solution &)> StopAtNeighbor;

struct DaySlot {
  int team, day, bridges;
};

static vector<DaySlot> day_slots(const Instance &I, const Solution &sol) {
  vector<DaySlot> slots;
  for (int team : I.O) {
    RouteDays days = split_days(I, sol.at(team));
    for (size_t d = 0; d < days.bridges.size(); d++)
      slots.push_back({team, (int)d + 1, (int)days.bridges[d].size()});
  }
  return slots;
}

static int select_accommodation(const Instance &I, int from) {
  int best = I.M.at(0);
  for (int m : I.M)
    if (I.f.at(m) + travel_cost(I, from, m) < I.f.at(best) + travel_cost(I, from, best)) best = m;
  return best;
}

static Solution without_bridge(const Instance &I, const Solution &sol, int team, int day, int position) {
  EditedRoutes edited;
  RouteDays &days = days_of(edited, I, sol, team);
  take_bridge(days, day, position);
  remove_empty_days(days);
  return with_edited_routes(sol, edited);
}

static bool two_opt_neighbors(const Instance &I, const Solution &sol, const StopAtNeighbor &stop) {
  for (const DaySlot &slot : day_slots(I, sol))
    for (int first = 0; first < slot.bridges; first++)
      for (int last = first + 1; last < slot.bridges; last++)
        if (stop(two_opt(I, sol, slot.team, slot.day, first, last))) return true;
  return false;
}

static bool swap_neighbors(const Instance &I, const Solution &sol, const StopAtNeighbor &stop) {
  vector<DaySlot> slots = day_slots(I, sol);
  for (int length = 1; length <= MAX_SWAP_LENGTH; length++)
    for (size_t a = 0; a < slots.size(); a++)
      for (size_t b = a + 1; b < slots.size(); b++)
        for (int start_a = 0; start_a + length <= slots[a].bridges; start_a++)
          for (int start_b = 0; start_b + length <= slots[b].bridges; start_b++) {
            Solution neighbor = swap_blocks(I, sol, slots[a].team, slots[a].day, start_a,
                                            slots[b].team, slots[b].day, start_b, length);
            if (stop(neighbor)) return true;
          }
  return false;
}

static bool change_day_neighbors(const Instance &I, const Solution &sol, const StopAtNeighbor &stop) {
  vector<DaySlot> slots = day_slots(I, sol);
  for (const DaySlot &from : slots)
    for (int position = 0; position < from.bridges; position++)
      for (const DaySlot &to : slots) {
        bool same_day = from.team == to.team && from.day == to.day;
        if (same_day) continue;

        for (int insert_at = 0; insert_at <= to.bridges; insert_at++) {
          Solution neighbor = change_day(I, sol, from.team, from.day, position, to.team, to.day, insert_at);
          if (stop(neighbor)) return true;
        }
      }
  return false;
}

static bool change_new_day_neighbors(const Instance &I, const Solution &sol, const StopAtNeighbor &stop) {
  for (const DaySlot &from : day_slots(I, sol))
    for (int position = 0; position < from.bridges; position++) {
      Solution removed = without_bridge(I, sol, from.team, from.day, position);
      for (int to_team : I.O) {
        const vector<int> &route = removed.at(to_team);
        int last_before_depot = route[route.size() - 2];
        int accommodation = select_accommodation(I, last_before_depot);

        Solution neighbor = change_new_day(I, sol, from.team, from.day, position, to_team, accommodation);
        if (stop(neighbor)) return true;
      }
    }
  return false;
}

static bool change_accommodation_neighbors(const Instance &I, const Solution &sol, const StopAtNeighbor &stop) {
  for (int team : I.O) {
    vector<int> lodgings = split_days(I, sol.at(team)).lodgings;
    for (size_t night = 1; night <= lodgings.size(); night++)
      for (int accommodation : I.M) {
        if (accommodation == lodgings[night - 1]) continue;
        if (stop(change_accommodation(I, sol, team, (int)night, accommodation))) return true;
      }
  }
  return false;
}

static bool visit_neighbors(const Instance &I, const Solution &sol, Move move, const StopAtNeighbor &stop) {
  switch (move) {
  case Move::TWO_OPT: return two_opt_neighbors(I, sol, stop);
  case Move::SWAP: return swap_neighbors(I, sol, stop);
  case Move::CHANGE_DAY: return change_day_neighbors(I, sol, stop);
  case Move::CHANGE_NEW_DAY: return change_new_day_neighbors(I, sol, stop);
  case Move::CHANGE_ACCOMMODATION: return change_accommodation_neighbors(I, sol, stop);
  }
  throw invalid_argument("hill_climbing: movimiento desconocido");
}

Solution hill_climbing(const Instance &I, const Solution &sol, Move move) {
  Solution current = sol;
  double current_cost = objective(I, current);

  while (true) {
    Solution improved;
    double improved_cost = 0;

    bool found = visit_neighbors(I, current, move, [&](const Solution &neighbor) {
      double cost = objective(I, neighbor);
      bool improves = cost < current_cost - MIN_IMPROVEMENT;
      if (!improves || !is_feasible(I, neighbor)) return false;

      improved = neighbor;
      improved_cost = cost;
      return true;
    });

    if (!found) return current;
    current = improved;
    current_cost = improved_cost;
  }
}

Solution destructor(const Instance &I, const Solution &sol, double alpha, mt19937 &rng) {
  if (alpha < 0 || alpha > 1) {
    ostringstream message;
    message << "destructor: alpha debe estar entre 0 y 1, se recibio alpha = " << alpha;
    throw invalid_argument(message.str());
  }

  vector<int> bridges = I.N;
  shuffle(bridges.begin(), bridges.end(), rng);
  size_t amount_to_remove = (size_t)floor(alpha * bridges.size() + 1e-9);
  vector<int> removed(bridges.begin(), bridges.begin() + amount_to_remove);

  Solution broken = sol;
  for (vector<int> &route : broken) {
    auto is_removed = [&](int node) { return contains(removed, node); };
    route.erase(remove_if(route.begin(), route.end(), is_removed), route.end());
  }
  return broken;
}

struct OpenRoute {
  int team;
  RouteDays days;
  vector<int> day_times;
  bool closed;
};

static int visit_time(const Instance &I, int from, int bridge) {
  return (int)lround(I.dist.at(from).at(bridge)) + I.nodes.at(bridge).w;
}

static int day_start(const RouteDays &days, size_t d) {
  return d == 0 ? days.depot : days.lodgings[d - 1];
}

static int day_end(const RouteDays &days, size_t d) {
  return days.bridges[d].empty() ? day_start(days, d) : days.bridges[d].back();
}

static size_t last_day_index(const OpenRoute &route) { return route.days.bridges.size() - 1; }

static bool has_bridges(const OpenRoute &route) {
  return any_of(route.days.bridges.begin(), route.days.bridges.end(),
                [](const vector<int> &bridges) { return !bridges.empty(); });
}

static OpenRoute open_route(const Instance &I, const vector<int> &route, int team) {
  RouteDays days = route.size() >= 2 ? split_days(I, route) : RouteDays{team, {{}}, {}};
  remove_empty_days(days);

  OpenRoute open{team, days, {}, false};
  for (size_t d = 0; d < days.bridges.size(); d++) {
    int time = 0, previous = day_start(days, d);
    for (int bridge : days.bridges[d]) {
      time += visit_time(I, previous, bridge);
      previous = bridge;
    }
    open.day_times.push_back(time);
  }
  return open;
}

static bool add_cheapest_bridge_to_day(const Instance &I, OpenRoute &route, size_t d, vector<int> &unvisited) {
  int current = day_end(route.days, d);
  int best = -1;
  for (int bridge : unvisited) {
    bool fits_in_shift = route.day_times[d] + visit_time(I, current, bridge) <= I.Q;
    if (!fits_in_shift) continue;
    if (best == -1 || travel_cost(I, current, bridge) < travel_cost(I, current, best)) best = bridge;
  }
  if (best == -1) return false;

  route.day_times[d] += visit_time(I, current, best);
  route.days.bridges[d].push_back(best);
  unvisited.erase(find(unvisited.begin(), unvisited.end(), best));
  return true;
}

static bool fill_earlier_day(const Instance &I, OpenRoute &route, vector<int> &unvisited) {
  for (size_t d = 0; d < last_day_index(route); d++)
    if (add_cheapest_bridge_to_day(I, route, d, unvisited)) return true;
  return false;
}

static void greedy_step(const Instance &I, OpenRoute &route, vector<int> &unvisited) {
  size_t last = last_day_index(route);
  if (add_cheapest_bridge_to_day(I, route, last, unvisited)) return;

  bool has_days_left = route.days.bridges.size() < I.K.size();
  bool can_sleep = has_days_left && !route.days.bridges[last].empty();
  if (!can_sleep) {
    route.closed = true;
    return;
  }

  route.days.lodgings.push_back(select_accommodation(I, day_end(route.days, last)));
  route.days.bridges.push_back({});
  route.day_times.push_back(0);
}

static vector<int> missing_bridges(const Instance &I, const Solution &sol) {
  vector<int> missing;
  for (int bridge : I.N) {
    bool visited = any_of(sol.begin(), sol.end(), [&](const vector<int> &route) { return contains(route, bridge); });
    if (!visited) missing.push_back(bridge);
  }
  return missing;
}

static Solution greedy_completion(const Instance &I, const Solution &broken, size_t first_team) {
  vector<int> unvisited = missing_bridges(I, broken);

  vector<OpenRoute> routes;
  for (size_t k = 0; k < I.O.size(); k++) {
    int team = I.O[(first_team + k) % I.O.size()];
    routes.push_back(open_route(I, broken.at(team), team));
  }

  for (OpenRoute &route : routes)
    while (!has_bridges(route) && !route.closed && !unvisited.empty()) greedy_step(I, route, unvisited);

  vector<bool> earlier_days_full(routes.size(), false);
  for (size_t turn = 0; !unvisited.empty(); turn++) {
    if (all_of(earlier_days_full.begin(), earlier_days_full.end(), [](bool full) { return full; })) break;

    size_t r = turn % routes.size();
    if (!earlier_days_full[r]) earlier_days_full[r] = !fill_earlier_day(I, routes[r], unvisited);
  }

  for (size_t turn = 0; !unvisited.empty(); turn++) {
    bool all_closed = all_of(routes.begin(), routes.end(), [](const OpenRoute &route) { return route.closed; });
    if (all_closed) break;

    OpenRoute &route = routes[turn % routes.size()];
    if (!route.closed) greedy_step(I, route, unvisited);
  }

  Solution built = empty_solution(I);
  for (OpenRoute &route : routes) {
    bool ends_sleeping = route.days.bridges.back().empty() && !route.days.lodgings.empty();
    if (ends_sleeping) {
      route.days.bridges.pop_back();
      route.days.lodgings.pop_back();
    }
    built[route.team] = join_days(route.days);
  }
  return built;
}

Solution reconstructor(const Instance &I, const Solution &broken, double alpha) {
  for (size_t first_team = 0; first_team < I.O.size(); first_team++) {
    Solution built = greedy_completion(I, broken, first_team);
    if (is_feasible(I, built)) return built;
  }

  ostringstream message;
  message << "reconstructor: no se pudo reconstruir una solucion factible con alpha = " << alpha;
  throw runtime_error(message.str());
}

Move select_move(const MoveSelector &selector, double epsilon, mt19937 &rng) {
  if (epsilon < 0 || epsilon > 1) {
    ostringstream message;
    message << "select_move: epsilon debe estar entre 0 y 1, se recibio epsilon = " << epsilon;
    throw invalid_argument(message.str());
  }

  bool explore = uniform_real_distribution<double>(0, 1)(rng) < epsilon;
  if (explore) return ALL_MOVES[uniform_int_distribution<int>(0, MOVE_COUNT - 1)(rng)];

  int best = 0;
  for (int m = 1; m < MOVE_COUNT; m++)
    if (selector.q[m] > selector.q[best]) best = m;
  return ALL_MOVES[best];
}

void update_move(MoveSelector &selector, Move move, double reward) {
  int m = static_cast<int>(move);
  selector.times_chosen[m]++;
  selector.q[m] += (reward - selector.q[m]) / selector.times_chosen[m];
}
