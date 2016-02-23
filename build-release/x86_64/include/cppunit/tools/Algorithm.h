#ifndef CPPUNIT_TOOLS_ALGORITHM_H_INCLUDED
#define CPPUNIT_TOOLS_ALGORITHM_H_INCLUDED

#include <cppunit/Portability.h>

CPPUNIT_NS_BEGIN

template<class SequenceType, class ValueType>
void
removeFromSequence( SequenceType &sequence, 
                    const ValueType &valueToRemove )
{
   for ( unsigned int index =0; index < sequence.size(); ++index )
   {
      if ( sequence[ index ] == valueToRemove )
         sequence.erase( sequence.begin() + index );
   }
}

CPPUNIT_NS_END


#endif // CPPUNIT_TOOLS_ALGORITHM_H_INCLUDED
