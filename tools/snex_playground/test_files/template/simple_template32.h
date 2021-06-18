/*
BEGIN_TEST_DATA
  f: main
  ret: int
  args: int
  input: 12
  output: 19
  error: ""
  filename: "template/simple_template32"
END_TEST_DATA
*/

template <int C> struct X
{
    template <int D> struct Inner
    {
	    int get() { return C + D; }
    };
    
    Inner<12> i;
};



int main(int input)
{
	X<19> obj;

	return obj.i.get();
}
