#include <map>

// testing if std::map and std::pmr::map take same memory
// yes

int main(){
  std::map<int, int> M;
  for (int i = 0; i < 5000000; ++i)
    M[i] = i;
  return M.size();
}
