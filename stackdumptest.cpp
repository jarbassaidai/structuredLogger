#include "stackdumptest.h"

stackdumptest::stackdumptest(t_spSL elog, long ncalls)
{
    //ctor
    myelog = elog;
    levelN(ncalls);
}

stackdumptest::~stackdumptest()
{
    //dtor
}

void stackdumptest::levelN(long l) {

    level = l ;
    callmyself();

}

void stackdumptest::callmyself()
{
    while (level > 0 ) {
        level --;
        callmyself();
    }

    myelog->stack_trace("testing stack_trace dump ","callmyself N times + 1 ");
}
