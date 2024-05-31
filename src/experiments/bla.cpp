
struct S {
  int fn(int) const {return 1;}
  int fn(int){return 2;}
};

int main(){
  S s;
  const S ss;
  s.fn(1);
  ss.fn(2);
}
