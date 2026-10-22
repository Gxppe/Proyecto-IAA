#pragma once
#include "handlers.h"
#include "instance.h"
#include <array>
#include <random>

double travel_cost(const Instance &I, int from, int to);
double route_cost(const Instance &I, const vector<int> &route, int team);
double objective(const Instance &I, const Solution &sol);

Solution two_opt(const Instance &I, const Solution &sol, int team, int day, int first, int last);

Solution swap_blocks(const Instance &I, const Solution &sol,
                     int team_a, int day_a, int start_a,
                     int team_b, int day_b, int start_b, int length);

Solution change_day(const Instance &I, const Solution &sol,
                    int from_team, int from_day, int position,
                    int to_team, int to_day, int insert_at);

Solution change_new_day(const Instance &I, const Solution &sol,
                        int from_team, int from_day, int position,
                        int to_team, int accommodation);

Solution change_accommodation(const Instance &I, const Solution &sol, int team, int night, int accommodation);

enum class Move { TWO_OPT, SWAP, CHANGE_DAY, CHANGE_NEW_DAY, CHANGE_ACCOMMODATION };

const int MOVE_COUNT = 5;
const array<Move, MOVE_COUNT> ALL_MOVES = {Move::TWO_OPT, Move::SWAP, Move::CHANGE_DAY, Move::CHANGE_NEW_DAY,
                                           Move::CHANGE_ACCOMMODATION};

struct MoveSelector {
  array<double, MOVE_COUNT> q{};
  array<int, MOVE_COUNT> times_chosen{};
};

Move select_move(const MoveSelector &selector, double epsilon, mt19937 &rng);
void update_move(MoveSelector &selector, Move move, double reward);

Solution hill_climbing(const Instance &I, const Solution &sol, Move move);

Solution destructor(const Instance &I, const Solution &sol, double alpha, mt19937 &rng);

Solution reconstructor(const Instance &I, const Solution &broken, double alpha);
