#pragma once
#include "handlers.h"
#include "instance.h"

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

Solution hill_climbing(const Instance &I, const Solution &sol, Move move);

Solution reconstructor(const Instance &I, const Solution &broken, double alpha);
