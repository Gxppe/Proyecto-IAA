#include "optimize_utils.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

static const double MIN_IMPROVEMENT = 1e-6;

struct VnsResult {
  Solution best;
  double initial_cost;
  double best_cost;
};

static VnsResult vns(const Instance &I, double alpha, double epsilon, int max_iterations, mt19937 &rng) {
  Solution current = reconstructor(I, empty_solution(I), 1.0);
  double current_cost = objective(I, current);
  double initial_cost = current_cost;
  MoveSelector selector;

  for (int iteration = 0; iteration < max_iterations; iteration++) {
    Solution shaken = reconstructor(I, destructor(I, current, alpha, rng), alpha);
    double shaken_cost = objective(I, shaken);

    Move move = select_move(selector, epsilon, rng);
    Solution improved = hill_climbing(I, shaken, move);
    double improved_cost = objective(I, improved);

    double relative_improvement = (shaken_cost - improved_cost) / shaken_cost;
    update_move(selector, move, relative_improvement);

    if (improved_cost < current_cost - MIN_IMPROVEMENT) {
      current = improved;
      current_cost = improved_cost;
    }
  }
  return {current, initial_cost, current_cost};
}

static double parse_number(const string &text, const string &name) {
  size_t parsed = 0;
  double value = 0;
  try {
    value = stod(text, &parsed);
  } catch (const exception &) {
    parsed = 0;
  }
  if (parsed == 0 || parsed != text.size()) throw invalid_argument(name + " no es un numero valido: " + text);
  return value;
}

static void require_between(double value, double low, double high, const string &name) {
  if (value >= low && value <= high) return;

  ostringstream message;
  message << setprecision(15) << name << " debe estar entre " << low << " y " << high << ", se recibio " << value;
  throw invalid_argument(message.str());
}

int main(int argc, char **argv) {
  if (argc != 6) {
    cerr << "Uso: " << argv[0] << " <ruta_instancia.dat> <alpha> <epsilon> <max_iteraciones> <seed>" << endl;
    return 1;
  }

  try {
    string path = argv[1];
    double alpha = parse_number(argv[2], "alpha");
    double epsilon = parse_number(argv[3], "epsilon");
    double max_iterations = parse_number(argv[4], "max_iteraciones");
    double seed = parse_number(argv[5], "seed");

    require_between(alpha, 0, 1, "alpha");
    require_between(epsilon, 0, 1, "epsilon");
    require_between(max_iterations, 0, 1e9, "max_iteraciones");
    require_between(seed, 0, 4294967295.0, "seed");
    if (max_iterations != floor(max_iterations) || seed != floor(seed))
      throw invalid_argument("max_iteraciones y seed deben ser enteros");

    Instance I = read_instance(path);
    mt19937 rng((unsigned)seed);

    auto start = chrono::steady_clock::now();
    VnsResult result = vns(I, alpha, epsilon, (int)max_iterations, rng);
    double seconds = chrono::duration<double>(chrono::steady_clock::now() - start).count();

    cout << setprecision(15);
    cout << "Instancia: " << path << endl;
    cout << "Costo inicial: " << result.initial_cost << endl;
    cout << "Mejor costo: " << result.best_cost << endl;
    cout << "Tiempo: " << seconds << " s" << endl;
    print_solution(I, result.best);
  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }
  return 0;
}
