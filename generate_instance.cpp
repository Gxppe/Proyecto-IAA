#include "instance.h"
#include <iomanip>
#include <iostream>

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Uso: " << argv[0] << " <ruta_instancia.dat>" << endl;
    return 1;
  }
  string path = argv[1];
  Instance I;
  try {
    I = read_instance(path);
  } catch (const exception &e) {
    cerr << "Error: " << e.what() << endl;
    return 1;
  }
  cout << "Instancia: " << path << endl;
  cout << setprecision(15);

  cout << "Depositos: " << I.O.size() << ", Puentes: " << I.N.size()
       << ", Alojamientos: " << I.M.size() << ", Dias: " << I.K.size() << endl;
  cout << "D=" << I.D << " Q=" << I.Q << " v=" << I.v << endl;

  cout << "puente, costo de inspeccion" << endl;
  for (int b : I.N) cout << b << ", " << I.nodes[b].w << endl;

  cout << "alojamiento, costo de alojamiento" << endl;
  for (int a : I.M) cout << a << ", " << I.f[a] << endl;

  vector<int> ids = I.O;
  ids.insert(ids.end(), I.N.begin(), I.N.end());
  ids.insert(ids.end(), I.M.begin(), I.M.end());
  cout << "dist";
  for (int j : ids) cout << "\t" << j;
  cout << endl;
  for (int a : ids) {
    cout << a;
    for (int b : ids) cout << "\t" << I.dist[a][b];
    cout << endl;
  }
  return 0;
}
