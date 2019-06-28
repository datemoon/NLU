#include <vector>
#include <stdio.h>

using namespace std;

class Type
{
public:
	Type()
	{
		printf("Constructor %p\n",this);
	}
	Type(const Type & A):_a(A._a)
	{
		printf("_a : %d @ copy %p to %p\n",_a,&A,this);
	}
	~Type()
	{
		printf("destructor %p\n",this);
	}
	int _a;
};

template <class T>
class Test
{
public:
	void AddT(T &a)
	{
		_a.push_back(a);
	}
private:
	vector<T> _a;
};

int main()
{
	Type t ;
	vector<Type> a;
	for(int i=0;i<10;++i)
	{
		t._a = i;
		a.push_back(t);
		printf("***********************\n");
	}
	return 0;
}
