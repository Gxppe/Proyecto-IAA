#include "instance.h"
#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

static vector<string> tokenize(ifstream &file) {
  static const regex separators(":=|;");
  vector<string> tokens;
  string line;
  while (getline(file, line)) {
    bool is_comment = line.empty() || line[0] == '#';
    if (is_comment) continue;

    istringstream words(regex_replace(line, separators, " $& "));
    for (string word; words >> word;) tokens.push_back(word);
  }
  return tokens;
}

Instance read_instance(const string &path) {
  ifstream file(path);
  if (!file) throw runtime_error("No se pudo abrir: " + path);
  vector<string> tokens = tokenize(file);

  Instance I;
  size_t pos = 0;

  auto peek = [&]() -> const string & {
    if (pos >= tokens.size()) throw runtime_error("Fin de archivo inesperado");
    return tokens[pos];
  };
  auto next = [&]() -> const string & {
    const string &token = peek();
    pos++;
    return token;
  };
  auto expect = [&](const string &expected) {
    if (next() != expected)
      throw runtime_error("Se esperaba '" + expected + "' en token " + to_string(pos));
  };
  auto read_until = [&](const string &end) {
    vector<string> read;
    for (string token = next(); token != end; token = next()) read.push_back(token);
    return read;
  };
  auto ensure_size = [&](int id) {
    if (id < 0) throw runtime_error("ID negativo: " + to_string(id));
    if ((size_t)id < I.dist.size()) return;
    I.nodes.resize(id + 1);
    I.dist.resize(id + 1);
    for (vector<double> &row : I.dist) row.resize(id + 1, -1);
  };

  while (pos < tokens.size()) {
    string section = next();

    if (section == "set") {
      string name = next();
      expect(":=");
      vector<int> ids;
      for (const string &id : read_until(";")) ids.push_back(stoi(id));

      if (name == "O") I.O = ids;
      else if (name == "N") I.N = ids;
      else if (name == "M") I.M = ids;
      else if (name == "K") I.K = ids;
      else throw runtime_error("set desconocido: " + name);

      bool ids_are_nodes = name != "K";
      if (ids_are_nodes)
        for (int id : ids) ensure_size(id);

    } else if (section == "param") {
      string name = next();

      if (name == "d[*,*]") {
        for (string block_start = next(); block_start != ";"; block_start = next()) {
          if (block_start != ":") throw runtime_error("Se esperaba ':' en d[*,*]");

          vector<int> columns;
          for (const string &column : read_until(":=")) columns.push_back(stoi(column));

          while (peek() != ":" && peek() != ";") {
            int row = stoi(next());
            for (int column : columns) {
              ensure_size(max(row, column));
              I.dist[row][column] = stod(next());
            }
          }
        }

      } else if (name == "f") {
        expect(":=");
        vector<string> pairs = read_until(";");
        if (pairs.size() % 2 != 0) throw runtime_error("param f incompleto");
        for (size_t k = 0; k < pairs.size(); k += 2) I.f[stoi(pairs[k])] = stod(pairs[k + 1]);

      } else {
        expect(":=");
        string value = next();
        expect(";");
        if (name == "D") I.D = stoi(value);
        else if (name == "Q") I.Q = stoi(value);
        else if (name == "v") I.v = stod(value);
        else throw runtime_error("param desconocido: " + name);
      }

    } else if (section == "param:") {
      const size_t columns_per_row = 5;
      read_until(":=");
      vector<string> values = read_until(";");
      if (values.size() % columns_per_row != 0) throw runtime_error("Tabla de nodos incompleta");

      for (size_t k = 0; k < values.size(); k += columns_per_row) {
        int id = stoi(values[k]);
        ensure_size(id);
        Node &node = I.nodes[id];
        node.x = stod(values[k + 1]);
        node.y = stod(values[k + 2]);
        node.w = stoi(values[k + 3]);
        node.q = stoi(values[k + 4]);
      }

    } else {
      throw runtime_error("Token inesperado: " + section);
    }
  }
  return I;
}
